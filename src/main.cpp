#include "SDReader.hpp"
#include "WiFiConfig.hpp"

#include "APIHandler.hpp"
#include "CommandHandler.hpp"

#include "config.h"
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"

#include <Aquabotica_plastic_fish_inferencing.h>

// Camera configuration
#define CAMERA_MODEL_WROVER_KIT // Has PSRAM

#include "camera_pins.h"

/* Constant defines -------------------------------------------------------- */
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3

// Instantiate CommandHandler for communication with ESP32
CommandHandler commandHandler(Serial);

SDReader sdReader;
APIHandler apiHandler;

status_t status = STATUS_BOOT;

camera_config_t cameraConfig;

static bool debug_nn = false; // Set this to true to see e.g. features generated
                              // from the raw signal
static bool is_initialised = false;

uint8_t *snapshot_buf; // points to the output of the capture

// ------- Prototypes ------------------------------------------------------- //
uint8_t *allocateSnapshotBuffer();
bool captureImage(uint8_t *snapshot_buf);
bool processImage(uint8_t *snapshot_buf, int &detected);
void handleBoundingBox(const ei_impulse_result_bounding_box_t &bb);
// void logError(const String &message, int code = 0);
void handleCapture(const String &command);

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size =
        FRAMESIZE_QVGA, // QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, // 0-63 lower number means higher quality
    .fb_count =
        1, // if more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

bool ei_camera_init(void)
{

    if (is_initialised)
        return true;

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);      // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, 0); // lower the saturation
    }

#if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#elif defined(CAMERA_MODEL_ESP_EYE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    s->set_awb_gain(s, 1);
#endif

    is_initialised = true;
    return true;
}

void handleHello(const String &command)
{
    commandHandler.sendCommand("READY");

    if (status == STATUS_BOOT) {
        status = STATUS_SYNCED;
    }
}

void handleReady(const String &command)
{
    if (status == STATUS_BOOT) {
        status = STATUS_SYNCED;
        commandHandler.sendCommand("READY");
    }
}

void handleInit(const String &command)
{
    if (status != STATUS_SYNCED) {
        return;
    }

    status = STATUS_INIT;

    SDReader::err_sd_t err = sdReader.init();

    switch (err) {

    case SDReader::err_sd_t::SD_ERR_NO_SDC:
        commandHandler.sendCommand("NO_SDC");
        status = STATUS_NO_SDC;
        return;
    case SDReader::err_sd_t::SD_ERR_CONFIG_FILE_NOT_CREATED:
        commandHandler.sendCommand("CONFIG_FILE_NOT_CREATED");
        status = STATUS_CONFIG_FILE_NOT_CREATED;
        return;
    }

    // Read WiFi configuration
    WiFiConfig wifiConfig;

    SDReader::err_read_config_t readConfErr = sdReader.readConfig(wifiConfig);

    switch (readConfErr) {
    case SDReader::err_read_config_t::RC_BAD_WIFI_CONFIG:
        commandHandler.sendCommand("BAD_WIFI_CONF");
        status = STATUS_BAD_WIFI_CONF;
        return;
    }

    APIHandler::err_wifi_t api_err = apiHandler.init(wifiConfig);

    if (api_err == APIHandler::err_wifi_t::WIFI_ERR) {
        commandHandler.sendCommand("NO_WIFI_CONN");
        status = STATUS_NO_WIFI_CONN;
        return;
    }

    bool camInit = ei_camera_init();
    if (!camInit) {
        esp_camera_deinit();
        // Serial.printf("ERROR: Camera init failed with error 0x%x\n", camErr);
        commandHandler.sendCommand("CAM_INIT_FAIL");
        status = STATUS_CAM_INIT_FAIL;
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);      // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, 0); // lower the saturation
    }

    is_initialised = true;

    // APIHandler::api_response_code_t response = apiHandler.pingAPI();

    // Serial.println("API response: " + String(response));

    // if (response != 200) {
    //     commandHandler.sendCommand("NO_INTERNET");
    //     status = STATUS_NO_INTERNET;
    //     return;
    // }

    commandHandler.sendCommand("INIT_SUCCESS");
    status = STATUS_READY;
}

void statusHandler(const String &command)
{
    commandHandler.sendCommand("STATUS", String(status));
}

bool ei_camera_capture(uint32_t img_width, uint32_t img_height,
                       uint8_t *out_buf)
{
    bool do_resize = false;

    if (!is_initialised) {
        ei_printf("ERR: Camera is not initialized\r\n");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
        ei_printf("Camera capture failed\n");
        return false;
    }

    bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);

    esp_camera_fb_return(fb);

    if (!converted) {
        ei_printf("Conversion failed\n");
        return false;
    }

    if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS) ||
        (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {
        do_resize = true;
    }

    if (do_resize) {
        ei::image::processing::crop_and_interpolate_rgb888(
            out_buf, EI_CAMERA_RAW_FRAME_BUFFER_COLS,
            EI_CAMERA_RAW_FRAME_BUFFER_ROWS, out_buf, img_width, img_height);
    }

    return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
    // we already have a RGB888 buffer, so recalculate offset into pixel index
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        // Swap BGR to RGB here
        // due to https://github.com/espressif/esp32-camera/issues/379
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) +
                              (snapshot_buf[pixel_ix + 1] << 8) +
                              snapshot_buf[pixel_ix];

        // go to the next pixel
        out_ptr_ix++;
        pixel_ix += 3;
        pixels_left--;
    }
    // and done!
    return 0;
}

static const int captureTryCount = 5;

void handleCapture(const String &command)
{
    const int maxRetries = 5;   // Maximum number of capture retries
    int retryCount = 0;         // Current retry attempt
    bool labelDetected = false; // Flag to indicate if a label is detected

    while (retryCount < maxRetries && !labelDetected) {
        retryCount++;

        // Allocate memory for the snapshot buffer
        snapshot_buf = (uint8_t *)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS *
                                         EI_CAMERA_RAW_FRAME_BUFFER_ROWS *
                                         EI_CAMERA_FRAME_BYTE_SIZE);

        if (snapshot_buf == nullptr) {
            // ei_printf("ERR: Failed to allocate snapshot buffer!\n");
            commandHandler.sendCommand("CAPTURE_FAIL");
            return; // Exit if memory allocation fails
        }

        // Set up signal data
        ei::signal_t signal;
        signal.total_length =
            EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
        signal.get_data = &ei_camera_get_data;

        // Capture image
        if (!ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH,
                               (size_t)EI_CLASSIFIER_INPUT_HEIGHT,
                               snapshot_buf)) {
            commandHandler.sendCommand("CAPTURE_FAIL");
            free(snapshot_buf);
            continue; // Retry capture
        }

        // Run the classifier
        ei_impulse_result_t result = {0};
        EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);

        if (err != EI_IMPULSE_OK) {
            commandHandler.sendCommand("AI_FAIL");
            free(snapshot_buf);
            continue; // Retry if classification fails
        }

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
        for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
            ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
            if (bb.value == 0) {
                continue; // Skip bounding boxes with zero value
            }

            // Stop retries as a label is detected
            labelDetected = true;

            // Process the detected label (e.g., fetch additional data)
            float calories;
            if (apiHandler.fetchData(bb.label, calories)) {
                String args = String(bb.label) + " " + String(calories);
                commandHandler.sendCommand("FISH_INFO", args);
            } else {
                commandHandler.sendCommand("CAPTURE_FAIL");
            }
            break; // Stop processing further bounding boxes
        }
#else
        // Handle predictions (classification mode)
        ei_printf("Predictions:\r\n");
        for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
            ei_printf("%.5f\r\n", result.classification[i].value);
        }
#endif

#if EI_CLASSIFIER_HAS_ANOMALY
        // Print anomaly results if applicable
        ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

#if EI_CLASSIFIER_HAS_VISUAL_ANOMALY
        // Print visual anomalies if applicable
        ei_printf("Visual anomalies:\r\n");
        for (uint32_t i = 0; i < result.visual_ad_count; i++) {
            ei_impulse_result_bounding_box_t bb =
                result.visual_ad_grid_cells[i];
            if (bb.value == 0) {
                continue;
            }
            ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                      bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
        }
#endif

        free(snapshot_buf); // Free memory after each attempt
    }

    if (!labelDetected) {
        commandHandler.sendCommand("FISH_NOT_RECOG");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    while (!Serial) {
        ;
    }

    // Register command handlers
    commandHandler.registerRoute("HELLO", handleHello);
    commandHandler.registerRoute("INIT", handleInit);
    commandHandler.registerRoute("READY", handleReady);
    commandHandler.registerRoute("STATUS", statusHandler);
    commandHandler.registerRoute("CAPTURE", handleCapture);

    commandHandler.sendCommand("HELLO");
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

    if (status != STATUS_READY) {
        return; // Skip processing if the system isn't ready
    }
}
