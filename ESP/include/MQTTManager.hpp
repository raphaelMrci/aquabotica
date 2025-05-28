#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>

class MQTTManager
{
  public:
    MQTTManager() = default;

    void begin(Client &client, const char *mqttServer);
    void loop();
    void publishSensorData(float temperature, float ph, float light,
                           float turbidity, float waterLevel, bool lignOn,
                           int fishType);

  private:
    WiFiClient espClient;
    PubSubClient mqttClient;

    String userId;

    void reconnect();
    void callback(char *topic, byte *payload, unsigned int length);
};
