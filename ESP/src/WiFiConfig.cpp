#include "WiFiConfig.hpp"

WiFiConfig::WiFiConfig()
{
}

WiFiConfig::WiFiConfig(std::string ssid, std::string password,
                       std::string gateway, std::string ip, std::string mask)
{
    this->ssid = ssid;
    this->password = password;
    this->gateway = gateway;
    this->ip = ip;
    this->mask = mask;
}
