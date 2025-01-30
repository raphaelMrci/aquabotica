#include "APIHandler.hpp"
#include "config.h"

#include <WiFi.h>

#include <ArduinoJson.h>

APIHandler::err_wifi_t APIHandler::init(const WiFiConfig &config)
{
    Serial.println("Connecting to WiFi...");

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
            Serial.println("Static IP configuration applied.");
        } else {
            Serial.println("ERROR: Invalid IP, Gateway, or Mask format.");
            return WIFI_ERR;
        }
    }

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
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

    Serial.println("\nConnected to WiFi.");
    return WIFI_OK;
}

APIHandler::api_response_code_t APIHandler::pingAPI()
{
    String url = API_URL + "/";

    _http.begin(url); // Start HTTP connection

    int httpResponseCode = _http.GET(); // Make a GET request

    _http.end(); // End the HTTP connection

    return httpResponseCode;
}

APIHandler::api_response_code_t APIHandler::fetchData(const String &name,
                                                      float &result)
{
    String url = API_URL + "/search/" + name;
    _http.begin(url); // Start HTTP connection

    int httpResponseCode = _http.GET(); // Make a GET request

    if (httpResponseCode > 0) {
        String response = _http.getString(); // Get the response

        // Parse response (extract calories value)
        result = _parseCalories(response);
    } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(httpResponseCode);
    }

    _http.end(); // End the HTTP connection

    return httpResponseCode;
}

float APIHandler::_parseCalories(String &data)
{
    const size_t capacity = JSON_OBJECT_SIZE(35) + 300;
    DynamicJsonDocument doc(capacity);

    // Parse the JSON response
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return 0; // Return 0 if parsing fails
    }

    // Extract the "calories" value from the JSON response
    float calories = doc["calories"];
    return calories / 100;
}
