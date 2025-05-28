#include <HTTPClient.h>

#include "CommandHandler.hpp"
#include "WiFiConfig.hpp"

class APIHandler
{
  public:
    APIHandler() = default;

    typedef enum { WIFI_OK, WIFI_ERR } err_wifi_t;

    typedef int api_response_code_t;

    err_wifi_t init(const WiFiConfig &config);

  private:
    HTTPClient _http; // Single instance of HTTPClient
};
