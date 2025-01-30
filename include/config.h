#pragma once

#include <Arduino.h>

typedef enum status_e {
    STATUS_BOOT,
    STATUS_SYNCED,
    STATUS_INIT,
    STATUS_READY,

    STATUS_CAM_INIT_FAIL,
    STATUS_NO_WIFI_CONN,
    STATUS_NO_SDC,
    STATUS_CONFIG_FILE_NOT_CREATED,
    STATUS_BAD_WIFI_CONF,
    STATUS_NO_INTERNET,

    STATUS_ERROR = -1
} status_t;

const String API_URL("http://192.168.209.117:8000");

typedef struct config_s {
    unsigned int temp;
    unsigned int ph;
    unsigned int turb;
    unsigned int water_lvl;
} config_t;
