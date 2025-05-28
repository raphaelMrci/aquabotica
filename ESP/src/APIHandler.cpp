#include "APIHandler.hpp"
#include "config.h"

#include <WiFi.h>

#include <ArduinoJson.h>

APIHandler::err_wifi_t APIHandler::init(const WiFiConfig &config)
{
    // Serial.println("Connecting to WiFi...");

    if (config.ssid.empty() || config.password.empty()) {
        Serial.println(
            "ERROR: SSID or Password is missing. Cannot connect to WiFi.");
        return WIFI_ERR;
    }

    // Check for static IP configuration
    if (!config.ip.empty() && !config.gateway.empty() && !config.mask.empty()) {
        IPAddress localIP, gateway, subnet;

        if (localIP.fromString(config.ip.c_str()) &&
            gateway.fromString(config.gateway.c_str()) &&
            subnet.fromString(config.mask.c_str())) {

            if (!WiFi.config(localIP, gateway, subnet)) {
                Serial.println("ERROR: Failed to configure static IP.");
                return WIFI_ERR;
            }
            // Serial.println("Static IP configuration applied.");
        } else {
            Serial.println("ERROR: Invalid IP, Gateway, or Mask format.");
            return WIFI_ERR;
        }
    }

    // Connect to WiFi
    // Serial.println("Connecting to WiFi...");
    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    int retryCount = 20; // Retry up to 20 times

    while (WiFi.status() != WL_CONNECTED && retryCount > 0) {
        delay(500);
        Serial.print(".");
        retryCount--;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nERROR: Failed to connect to WiFi.");
        return WIFI_ERR;
    }

    // Serial.println("\nConnected to WiFi.");
    return WIFI_OK;
}
