#pragma once

#include <SD.h>
#include <string>

#include "WiFiConfig.hpp"

class SDReader
{
  public:
    typedef enum {
        SD_OK = 0,
        SD_ERR_NO_SDC,
        SD_ERR_CONFIG_FILE_NOT_CREATED
    } err_sd_t;

    typedef enum {
        RC_OK = 0,
        RC_BAD_WIFI_CONFIG,
    } err_read_config_t;

    SDReader() = default;

    err_sd_t init();

    err_read_config_t readConfig(WiFiConfig &config);

    String readFile(String path);

  private:
    String _defaultConfig = "SSID=\n"
                            "Password=\n"
                            "IP=\n"
                            "Gateway=\n"
                            "MASK=\n";
};
