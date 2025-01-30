#pragma once

#include <string>

class WiFiConfig
{
  public:
    WiFiConfig();
    WiFiConfig(std::string ssid, std::string password, std::string gateway,
               std::string ip, std::string mask);

    std::string ssid;
    std::string password;
    std::string gateway;
    std::string ip;
    std::string mask;
};
