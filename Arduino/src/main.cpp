#include "CommandHandler.hpp"
#include "config.h"
#include "pins.h"
#include "sensors.h"

#include <Arduino.h>
#include <Servo.h>
#include <ctype.h>

Servo foodServo;

// Instantiate CommandHandler for communication with ESP32
CommandHandler commandHandler(Serial);

status_t status = STATUS_BOOT;

void handleHello(const String &command)
{
    commandHandler.sendCommand("READY");
    commandHandler.sendCommand("INIT");

    if (status == STATUS_BOOT) {
        status = STATUS_SYNCED;
    }
}

void handleReady(const String &command)
{
    if (status == STATUS_BOOT) {
        status = STATUS_SYNCED;
        commandHandler.sendCommand("READY");
        commandHandler.sendCommand("INIT");
    }
}

void handleNoSDCard(const String &command)
{
    status = STATUS_NO_SDC;
    // lcd.clear();
    // lcd.setCursor(1, 0);
    // lcd.print("No SD card!");
}

void handleBadWiFiConfig(const String &command)
{
    status = STATUS_BAD_WIFI_CONF;
    // lcd.clear();
    // lcd.setCursor(1, 0);
    // lcd.print("Bad WiFi config!");
}

void handleNoWiFiConnection(const String &command)
{
    status = STATUS_NO_WIFI_CONN;
    // lcd.clear();
    // lcd.setCursor(1, 0);
    // lcd.print("Unable to connect");
    // lcd.setCursor(1, 1);
    // lcd.print("to WiFi...");
}

void handleCamInitFailed(const String &command)
{
    status = STATUS_CAM_INIT_FAIL;
    // lcd.clear();
    // lcd.setCursor(1, 0);
    // lcd.print("Camera init");
    // lcd.setCursor(1, 1);
    // lcd.print("failed!");
}

void handleNoInternet(const String &command)
{
    status = STATUS_NO_INTERNET;
    // lcd.clear();
    // lcd.setCursor(1, 0);
    // lcd.print("No internet");
    // lcd.setCursor(1, 1);
    // lcd.print("connection...");
}

void handleInitSuccess(const String &command)
{
    if (status == STATUS_SYNCED) {
        status = STATUS_READY;
        // lcd.clear();
        // lcd.setCursor(1, 0);
        // lcd.print("Scale It!");
    }
}

void handleConfigFileNotCreated(const String &command)
{
    status = STATUS_ERROR;
    // lcd.clear();
    // lcd.setCursor(1, 0);
    // lcd.print("Config file");
    // lcd.setCursor(1, 1);
    // lcd.print("not created...");
}

void handleNoApiUrl(const String &command)
{
    // lcd.clear();
    // lcd.setCursor(1, 0);
    // lcd.print("No API URL...");
    // delay(2000); // Display for 2 seconds
}

void statusHandler(const String &command)
{
    if (command.length() == 0) {
        return;
    }

    if (!isdigit(command[0])) {
        // Serial.println("Invalid status received: " + command);
        return;
    }

    status_t newStatus = static_cast<status_t>(command.toInt());

    switch (newStatus) {
    case STATUS_BOOT:
        handleHello("");
        break;
    case STATUS_SYNCED:
        handleReady("");
        break;
    case STATUS_INIT:
        break;
    case STATUS_READY:
        handleInitSuccess("");
        break;
    case STATUS_CAM_INIT_FAIL:
        handleCamInitFailed("");
        break;
    case STATUS_NO_WIFI_CONN:
        handleNoWiFiConnection("");
        break;
    case STATUS_NO_SDC:
        handleNoSDCard("");
        break;
    case STATUS_CONFIG_FILE_NOT_CREATED:
        handleConfigFileNotCreated("");
        break;
    case STATUS_BAD_WIFI_CONF:
        handleBadWiFiConfig("");
        break;
    case STATUS_NO_INTERNET:
        handleNoInternet("");
        break;
    case STATUS_NO_API_URL:
        handleNoApiUrl("");
        break;
    }
}

void handleLightSwitch(const String &command)
{
    int brightness = getLEDsBrightness();

    if (brightness == 0) {
        setLEDsIntensity(255); // Turn on the light
    } else {
        setLEDsIntensity(0); // Turn off the light
    }
}

void handleDispenseFood(const String &command)
{
    foodServo.write(90); // dispense position
    delay(1000);
    foodServo.write(0); // return to idle
}

void init_commands()
{
    // Register command handlers
    commandHandler.registerRoute("HELLO", handleHello);
    commandHandler.registerRoute("NO_SDC", handleNoSDCard);
    commandHandler.registerRoute("BAD_WIFI_CONF", handleBadWiFiConfig);
    commandHandler.registerRoute("NO_WIFI_CONN", handleNoWiFiConnection);
    commandHandler.registerRoute("CAM_INIT_FAIL", handleCamInitFailed);
    commandHandler.registerRoute("NO_INTERNET", handleNoInternet);
    commandHandler.registerRoute("INIT_SUCCESS", handleInitSuccess);
    commandHandler.registerRoute("CONFIG_FILE_NOT_CREATED",
                                 handleConfigFileNotCreated);
    commandHandler.registerRoute("READY", handleReady);
    commandHandler.registerRoute("STATUS", statusHandler);
    commandHandler.registerRoute("API_URL_MISSING", handleNoApiUrl);
    commandHandler.registerRoute("LIGHT", handleLightSwitch);
    commandHandler.registerRoute("DISPENSE_FOOD", handleDispenseFood);

    // Send HELLO
    commandHandler.sendCommand("HELLO");
}

void init_servo()
{
    pinMode(SERVO_PIN, OUTPUT);

    foodServo.attach(SERVO_PIN);

    foodServo.write(0);
}

void setup()
{
    Serial.begin(115200);

    delay(1000);

    while (!Serial) {
        ; // Wait for serial port to connect
    }

    delay(1000);

    init_LEDs();
    init_light();
    init_servo();
    init_temp();

    init_commands();
}

void sendSensorsValues(float temperature, float ph, float light,
                       float turbidity, int waterLevel, bool lightOn)
{
    commandHandler.sendCommand(
        "SENSORS", String(temperature) + " " + String(ph) + " " +
                       String(light) + " " + String(turbidity) + " " +
                       String(waterLevel) + " " + (lightOn ? "1" : "0"));
}

void loop()
{
    commandHandler.handleIncomingCommand(); // Handle serial commands

    if (status == STATUS_BOOT) {
        static unsigned long lastHello = 0;
        if (millis() - lastHello > 1000) { // Send HELLO every 1 second
            lastHello = millis();
            commandHandler.sendCommand("HELLO");
        }
        return;
    }

    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 5000) { // Send STATUS every 5 second
        lastStatus = millis();
        commandHandler.sendCommand("STATUS");
    }

    if (status != STATUS_READY) {
        return; // Skip processing if the system isn't ready
    }

    // Fetch each sensor value
    float temp = getTemperature();          // Get water temperature
    float ph = getPhValue();                // Get water pH
    float light = getLightValue();          // Get light intensity from LDR
    float turbidity = getTurbidity();       // Get water turbidity
    int waterLevel = getWaterLevel();       // Get water level
    bool lightOn = getLEDsBrightness() > 0; // Check if light is on

    // Send all sensor data
    sendSensorsValues(temp, ph, light, turbidity, waterLevel, lightOn);

    led_loop(); // Update LEDs

    // Wait for 1 second before next reading
    delay(1000);
}
