#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

// WiFi credentials
#define ssid "Wokwi-GUEST"
#define password ""

// MQTT Broker details
const char *mqtt_broker =
    "test.mosquitto.org"; // Remplacez par votre broker privé si nécessaire
const char *topic = "smart-aquarium/sensors";
const char *topic_commands = "smart-aquarium/commands";
const int mqtt_port = 1883;

// UUID of the device
const char *UUID = "66a09098-8f50-4dc2-a59a-3235ee26a05f";

// JWT Token (to be received after association via backend)
char jwt_token[256] = ""; // Vide jusqu'à la réception d'un token valide

WiFiClient espClient;
PubSubClient client(espClient);

void init_wifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println("\nConnected to WiFi");
    Serial.println(WiFi.localIP());
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message received [");
    Serial.print(topic);
    Serial.print("]: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Process commands from backend if necessary
    if (String(topic) == topic_commands) {
        DynamicJsonDocument doc(256);
        deserializeJson(doc, payload, length);
        const char *command = doc["command"];
        Serial.print("Received command: ");
        Serial.println(command);
        // Process specific commands here
    }
}

void reconnect_mqtt()
{
    while (!client.connected()) {
        Serial.print("Connecting to MQTT broker...");
        if (client.connect(UUID)) {
            Serial.println("Connected");
            client.subscribe(topic_commands); // Subscribe to commands topic
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" trying again in 5 seconds");
            delay(5000);
        }
    }
}

void send_sensor_data()
{
    if (strlen(jwt_token) == 0) {
        Serial.println("No JWT token available. Cannot send data.");
        return;
    }

    // Simulate sensor data
    float temperature = 24.5; // Example value
    float pH = 7.2;           // Example value

    // Create JSON payload
    StaticJsonDocument<256> doc;
    doc["uuid"] = UUID;
    doc["temperature"] = temperature;
    doc["pH"] = pH;
    doc["token"] = jwt_token; // Include the JWT token

    char payload[256];
    serializeJson(doc, payload);

    client.publish(topic, payload);
    Serial.println("Data sent: ");
    Serial.println(payload);
}

void setup()
{
    Serial.begin(115200);
    init_wifi();

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(mqtt_callback);

    reconnect_mqtt();

    // Simulate obtaining JWT token after association (in real case, get from
    // backend)
    strcpy(
        jwt_token,
        "sample-jwt-token-from-backend"); // Replace with real token retrieval
}

void loop()
{
    if (!client.connected()) {
        reconnect_mqtt();
    }
    client.loop();

    send_sensor_data();
    delay(5000); // Send data every 5 seconds
}
