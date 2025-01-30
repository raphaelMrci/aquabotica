#include "SDReader.hpp"
#include "SD_MMC.h"

// Initialize SD card and create the default config file if necessary
SDReader::err_sd_t SDReader::init()
{
    if (!SD_MMC.begin("", true)) {
        return SD_ERR_NO_SDC; // Return error if SD card initialization fails
    }

    // Check if config.txt exists
    if (!SD_MMC.exists("/config.txt")) {
        // Attempt to create the config.txt file
        File file = SD_MMC.open("/config.txt", FILE_WRITE);
        if (!file) {
            return SD_ERR_CONFIG_FILE_NOT_CREATED; // Return error if file
                                                   // creation fails
        }

        file.print(_defaultConfig.c_str()); // Write the default config
        file.close(); // Ensure the file is properly closed
    }

    return SD_OK;
}

// Read and parse the configuration file into a WiFiConfig object
SDReader::err_read_config_t SDReader::readConfig(WiFiConfig &config)
{
    String configFileContent = readFile("/config.txt");
    if (configFileContent.isEmpty()) {
        return RC_BAD_WIFI_CONFIG; // Return error if the file is empty or
                                   // cannot be read
    }

    // Split the file content into lines
    String lines[5];
    int index = 0;

    while (configFileContent.length() > 0 && index < 5) {
        int endOfLine = configFileContent.indexOf('\n');
        if (endOfLine == -1) { // Handle the last line
            lines[index++] = configFileContent;
            break;
        } else {
            lines[index++] = configFileContent.substring(0, endOfLine);
            configFileContent = configFileContent.substring(endOfLine + 1);
        }
    }

    // Parse the lines
    for (int i = 0; i < index; i++) {
        lines[i].trim(); // Remove any whitespace or newline characters

        if (lines[i].startsWith("SSID=")) {
            config.ssid = lines[i].substring(5).c_str();
        } else if (lines[i].startsWith("Password=")) {
            config.password = lines[i].substring(9).c_str();
        } else if (lines[i].startsWith("Gateway=")) {
            config.gateway = lines[i].substring(8).c_str();
        } else if (lines[i].startsWith("IP=")) {
            config.ip = lines[i].substring(3).c_str();
        } else if (lines[i].startsWith("MASK=")) {
            config.mask = lines[i].substring(5).c_str();
        }
    }

    Serial.println("WiFi configuration read successfully.");
    Serial.println(("SSID: " + config.ssid).c_str());
    Serial.println(("Password: " + config.password).c_str());

    return RC_OK; // Successfully parsed the config
}

// Read the contents of a file from the SD card
String SDReader::readFile(String path)
{
    File file = SD_MMC.open(path, FILE_READ);
    if (!file) {
        return ""; // Return an empty string if the file cannot be opened
    }

    String fileContent = file.readString(); // Read the entire file content
    file.close();                           // Ensure the file is closed
    return fileContent;
}
