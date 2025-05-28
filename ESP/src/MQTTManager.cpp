#include "MQTTManager.hpp"
#include "CommandHandler.hpp"
#include "fish_config.h"
#include <ArduinoJson.h>

void MQTTManager::begin(Client &client, const char *mqttServer)
{
    mqttClient.setClient(client);
    mqttClient.setServer(mqttServer, 1883);
    mqttClient.setBufferSize(512);
    mqttClient.setCallback(
        [this](char *topic, byte *payload, unsigned int length) {
            this->callback(topic, payload, length);
        });
    reconnect();
}

void MQTTManager::loop()
{
    if (!mqttClient.connected()) {
        reconnect();
    }
    mqttClient.loop();
}

void MQTTManager::publishSensorData(float temperature, float ph, float light,
                                    float turbidity, float waterLevel,
                                    bool lightOn, int fishType)
{
    JsonDocument doc;

    JsonObject tempObj = doc["temp"].to<JsonObject>();
    tempObj["value"] = temperature;
    tempObj["min"] = fish_config[fishType].min_temp;
    tempObj["max"] = fish_config[fishType].max_temp;
    tempObj["isOutOfRange"] = temperature < fish_config[fishType].min_temp ||
                              temperature > fish_config[fishType].max_temp;

    JsonObject phObj = doc["ph"].to<JsonObject>();
    phObj["value"] = ph;
    phObj["min"] = fish_config[fishType].min_ph;
    phObj["max"] = fish_config[fishType].max_ph;
    phObj["isOutOfRange"] =
        ph < fish_config[fishType].min_ph || ph > fish_config[fishType].max_ph;

    JsonObject turbObj = doc["turb"].to<JsonObject>();
    turbObj["value"] = turbidity;
    turbObj["min"] = fish_config[fishType].min_turb;
    turbObj["max"] = fish_config[fishType].max_turb;
    turbObj["isOutOfRange"] = turbidity < fish_config[fishType].min_turb ||
                              turbidity > fish_config[fishType].max_turb;

    JsonObject waterObj = doc["waterLevel"].to<JsonObject>();
    waterObj["value"] = waterLevel;
    waterObj["min"] = fish_config[fishType].min_water_lvl;
    waterObj["max"] = fish_config[fishType].min_water_lvl;
    waterObj["isOutOfRange"] =
        waterLevel < fish_config[fishType].min_water_lvl ||
        waterLevel > fish_config[fishType].min_water_lvl;

    JsonObject lightObj = doc["light"].to<JsonObject>();
    lightObj["value"] = light;
    lightObj["isTurnedOn"] = lightOn;
    lightObj["isOutOfRange"] = false;

    JsonObject foodObj = doc["food"].to<JsonObject>();
    foodObj["lastTime"] = millis();
    foodObj["nextTime"] = millis() + 86400000;

    doc["config"] = fish_config[fishType].name;

    char JSONmessageBuffer[512];
    serializeJson(doc, JSONmessageBuffer);

    bool result = mqttClient.publish("aquabotica/sensors", JSONmessageBuffer);
}

void MQTTManager::callback(char *topic, byte *payload, unsigned int length)
{
    CommandHandler *cmd = CommandHandler::getInstance();

    if (!cmd) {
        Serial.println("CommandHandler is null! Aborting command dispatch.");
        return;
    }

    if (String(topic) == "aquabotica/light") {
        // Handle light control
        cmd->sendCommand("LIGHT");
    } else if (String(topic) == "aquabotica/food") {
        // Handle food control
        cmd->sendCommand("DISPENSE_FOOD");
    } else if (String(topic) == "aquabotica/autoconfig") {
        handleCapture();
    } else if (String(topic) == "aquabotica/delete_config") {
        Serial.println("Received delete_config command");
        resetConfig();
    } else {
        Serial.print("Unknown topic: ");
        Serial.println(topic);
        return; // Ignore unknown topics
    }
}

void MQTTManager::reconnect()
{
    while (!mqttClient.connected()) {
        // Serial.println("reconnect(): Not connected, attempting to
        // connect...");

        // Serial.print("Attempting MQTT connection...");
        bool success = mqttClient.connect("aquabotica-device");

        // Serial.print("connect() returned: ");
        // Serial.println(success ? "true" : "false");

        if (success) {
            // Serial.println("connected");
            mqttClient.subscribe("aquabotica/light");
            mqttClient.subscribe("aquabotica/food");
            mqttClient.subscribe("aquabotica/autoconfig");
            mqttClient.subscribe("aquabotica/delete_config");
            // Serial.println(
            //     "Subscribed to topics: aquabotica/light and
            //     aquabotica/food");
        } else {
            // Serial.print("failed, rc=");
            // Serial.print(mqttClient.state());
            // Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
