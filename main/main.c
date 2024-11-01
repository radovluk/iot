#include "main.h"

#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gauge.h"
#include "mqtt.h"
#include "nvs_flash.h"
#include "sntp.h"
#include "wifi.h"
#include "esp_sleep.h"

#include "esp_mac.h"
#include "driver/rtc_io.h"
#include "device_config.h"

// GPIO Definitions
#define PIR_PIN 27
#define MAGNETIC_SWITCH_PIN 33


const uint8_t LIVINGROOM_MAC_ADDRESS[6] = {0xEC, 0x62, 0x60, 0xBC, 0xE8, 0x50};
const uint8_t KITCHEN_MAC_ADDRESS[6]    = {0xEC, 0x62, 0x60, 0xBC, 0xE8, 0x18};
const uint8_t BATHROOM_MAC_ADDRESS[6]   = {0x94, 0x3C, 0xC6, 0xD1, 0x42, 0x2C};

// Define the device-specific parameters
const char* DEVICE_ID   = NULL;
const char* DEVICE_TOPIC = NULL;
const char* DEVICE_KEY  = NULL;

int count = 0;
bool battery_info_available = true;

void IRAM_ATTR handlePIRevent(void *arg) {
    count++;
    ets_printf("Got PIR event %d\n", count);
}


void app_main() {
    ESP_LOGI("progress", "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI("progress", "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt", ESP_LOG_INFO);
    esp_log_level_set("progress", ESP_LOG_INFO);
    esp_log_level_set("gauge", ESP_LOG_INFO);

    // Read the MAC address
    uint8_t current_mac[6];
    esp_read_mac(current_mac, ESP_MAC_WIFI_STA);
    ESP_LOGI("progress", "MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
             current_mac[0], current_mac[1], current_mac[2],
             current_mac[3], current_mac[4], current_mac[5]);

    // Check if it matches any of the predefined MAC addresses
    if (memcmp(current_mac, LIVINGROOM_MAC_ADDRESS, sizeof(LIVINGROOM_MAC_ADDRESS)) == 0) {
        ESP_LOGI("MAC Address", "Device identified as LIVING ROOM.");
        DEVICE_ID    = "4";
        DEVICE_TOPIC = "1/4/data";
        DEVICE_KEY   = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzODAxNTYsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS80In0.pz7e__yvBeb-xrVAXNlY_6-GPg0PBvrMOzxsG9re_ohsAMgKnVddBFwaaRB15P07J-D4_s_1KpHDRNw0trIfXdnPTFaV0ibKzk2C-j6EGXlBRFi7POP0p_QMobHk5DtI54j9fpbxtAvl7uwQCWJlBY4w0rmynlJrEN5TRvu2veMtvN8HPOoYpw4k1L_jif_w0Jli-MM-aDhhuRFUO07hwqV1qoxArm0xcd4EW0u0OWM0Uvs9vW51Vr_BDb7-TgvywJQO9R8DCjXk3BBPG8BYavinuA4fTTC5oKzJRyRI3_zwv7DHaXMT3eD-tRMKxqvdBsaxpTG0UyCIQ9HrefKTVaE8JD6so1fbGdsMQ3qvjKtSamQYPFWMFhUGj7qmEwzjIqXBXdGEO1j7YTh3jG1fDaXXIvVSffj2_Hl1hCEwuiaPxh7DRQIhZVNV0Gv2IXq1_s7hB6byjXnUdQyJtUZS8xfdCPEP3YHDPe14fRUBZJDJrYsg2XPRGkLrpcPlIDSgDgv9nS_vO19lwX3QN-LmtJ7P2mYgVnG0ELljRAKtZvYhcKfoSyE6R1Amw5XlAiV5OcftdpayJLlqmMStQjakQLuxQVI6KALcCNkHvbOAM5zcAbSCgRNSdZppxlGzePu15ngtBZjL5xSuIEW6Y5NTM6E1v24kUhWUvvi9p42c3r0";
    } else if (memcmp(current_mac, KITCHEN_MAC_ADDRESS, sizeof(KITCHEN_MAC_ADDRESS)) == 0) {
        ESP_LOGI("MAC Address", "Device identified as KITCHEN.");
        DEVICE_ID    = "5";
        DEVICE_TOPIC = "1/5/data";
        DEVICE_KEY   = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzODE1MzMsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS81In0.HVn4uZAQRIOvE6ZR16YnU9LhEqBmatBh7w2enEuiLxJt51ievhqYdRFW8DLjGW49zXdXP_u-h1LxRyyfkwrTFXY0fH8__3J4D1dLd3WRlfXyztkmCH846GRwMBoi-DLrXg4OW3BoVt2mGYhauOo4OwixXYNUCT1MfPZEpTb8DpcCbVfYHwEdd1y2WrF5BLguKyIuxUG6qwl2Llcti1porh5D78Onwt-OTiAAMyHlXHYg-DTEJR3qYSI2IuBtvU6jwO5G68BHIxL7Ug8GLbTdE0xtuwUvPKeupwyJPFBVFHTSD1s7p7F7PZhzJ5W6NQq4NibUnDHJNY7GrOXG9anApJpveC05xdKnraueZ41uJrg4leQlDRWTkFyeRPLBsX6Z_5TTA2kj4krEDQR4lmR9zilI1gJQV1h06NYwtO4Gx-48uXzhoo8JdckaxAM6m1mXTPOgGU-U2do3Cs3RhlsceUVTOld5rd0so4C4Ui3_X5TDA8uz8noPxyhxSYPKk4aZ8f80MjNsy2SicCxUv4jVhEH3iIzYi3m_0rFzynIf_oQbbVtkGKGyRjp3EEU-g9v-SvHdf4oYyA6kQ6YFe4IC6rABbH1rUp5jvUpJcsmgPqskbPTfnwZPCxWCRk5Jdc2piV5vHwxhguERu7Sg-fVZ4RIl2Er17FRHzvBZokUYWx8";
    } else if (memcmp(current_mac, BATHROOM_MAC_ADDRESS, sizeof(BATHROOM_MAC_ADDRESS)) == 0) {
        ESP_LOGI("MAC Address", "Device identified as BATHROOM.");
        battery_info_available = false;
        DEVICE_ID    = "3";
        DEVICE_TOPIC = "1/3/data";
        DEVICE_KEY   = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzNzg3MzIsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS8zIn0.a0bD30ukZEU0lcRSPyn6wsXSgL2paInHwrJF4TJl12m9w8JivwD3_cqpsQn6QS_F3AkxHhBl6E2FjJyZEVimZKINMe0tMbmaGy6JejS4RVclgs1lw1t13Ml4BDZEZU9WRm4gSuWPEOeY_cMbbizg1PIx3juKi-_pRjEGMDpcalnQAw0wDbDUmImMNE8ifKV3_JgOsbzAhK_yW_Zn1EB4C8Vvroj7LAeOsBsAja_I1ejh2E0KeAU6aIS1-ZMni6Y3v5JL41RZiEGWhgqhU8XTpgLPfbUbRhAmP08QBRoHd6a7XOMW-uuHiuwTswYgm9wWxHKVWWVfrR--_LR7-26z06H5yUd_wQvegvcXjW30zuwwCKVwN6802jlnDXCXH8-j6WbMf6lVRCLl8MJNQjiAa8IHeW0DxCTDG_xvJ5P8dC5H7qkLa0EnOh65PlN8l2Rbja63MWg5q4hDNNc31AjeFjreJw50QI4Bgg3plpNqHfe9jcxducpMVd8Gn-_FC_isrcAHt1QWRajKtGlZeyOjZgzw6FKFM4UrdwWZ7NKhO1pAmkzCSG79-EbZT-viU9lkTz8WmcayejPt9pvmfPmGwe98EC1TatCB-SYHvmv5MbtnTjjGhaIECmRYiPEDushueAF5gVfJ1gsle8qsYOtOeEEPSz3CCe56HZfP_WUVBqs";
    } else {
        ESP_LOGI("MAC Address", "Device location not recognized.");
        // Set default values
        DEVICE_ID    = "unknown";
        DEVICE_TOPIC = "unknown/topic";
        DEVICE_KEY   = "default_key";
    }

    // Log the selected parameters for debugging
    ESP_LOGI("Device Info", "DEVICE_ID: %s", DEVICE_ID);
    ESP_LOGI("Device Info", "DEVICE_TOPIC: %s", DEVICE_TOPIC);
    ESP_LOGI("Device Info", "DEVICE_KEY: %s", DEVICE_KEY);

    if (battery_info_available){
        getRSOC();
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("progress", "Starting Wifi");
    start_wifi();

    ESP_LOGI("progress", "Starting Clock");
    start_clock();

    ESP_LOGI("progress", "Starting MQTT");
    start_mqtt();

    if (battery_info_available){
    ESP_LOGI("progress", "Sending battery status to MQTT");
        sendBatteryStatusToMQTT();
    }

    // Configure RTC GPIOs for PIR and Magnetic Switch
    // PIR Sensor Configuration (Assuming it uses pull-down and is active high)
    ESP_ERROR_CHECK(rtc_gpio_init(PIR_PIN));
    ESP_ERROR_CHECK(rtc_gpio_set_direction(PIR_PIN, RTC_GPIO_MODE_INPUT_ONLY));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(PIR_PIN));
    ESP_ERROR_CHECK(rtc_gpio_pullup_dis(PIR_PIN));

    // Magnetic Switch Sensor Configuration
    ESP_ERROR_CHECK(rtc_gpio_init(MAGNETIC_SWITCH_PIN));
    ESP_ERROR_CHECK(rtc_gpio_set_direction(MAGNETIC_SWITCH_PIN, RTC_GPIO_MODE_INPUT_ONLY));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(MAGNETIC_SWITCH_PIN));
    ESP_ERROR_CHECK(rtc_gpio_pullup_dis(MAGNETIC_SWITCH_PIN));


    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
        if (wakeup_pin_mask & (1ULL << PIR_PIN)) {
            // PIR sensor caused wakeup
            ESP_LOGI("progress", "Wakeup caused by PIR sensor");
            sendPIReventToMQTT();
        }
        if (wakeup_pin_mask & (1ULL << MAGNETIC_SWITCH_PIN)) {
            // Magnetic switch caused wakeup
            ESP_LOGI("progress", "Wakeup caused by Magnetic Switch");
            sendMagneticSwitchEventToMQTT();
        }
    } else {
        ESP_LOGI("progress", "Not a wakeup from deep sleep");
    }

    ESP_LOGI("progress", "Waiting for sensors to become inactive");
    while (gpio_get_level(PIR_PIN) == 1 || gpio_get_level(MAGNETIC_SWITCH_PIN) == 1) {
        // Wait until both sensors are inactive (logic low)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Create a bitmask for the GPIO pins
    uint64_t wakeup_pins_mask = (1ULL << PIR_PIN) | (1ULL << MAGNETIC_SWITCH_PIN);

    // Enable EXT1 wakeup on the selected pins with any high logic level
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(wakeup_pins_mask, ESP_EXT1_WAKEUP_ANY_HIGH));

    ESP_LOGI("progress", "Calling function finish_wifi()");
    finish_wifi();

    ESP_LOGI("progress", "Going to sleep");
    esp_deep_sleep_start();
}
