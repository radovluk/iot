// Standard Libraries
#include <stdio.h>
#include <string.h>

// ESP-IDF Core Headers
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_mac.h"
#include "esp_pm.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

// Project-Specific Headers
#include "wifi.h"
#include "mqtt.h"
#include "sntp.h"
#include "main.h"
#include "gauge.h"

// Information about the current device
const char* DEVICE_ID = "unknown";
const char* DEVICE_TOPIC = "unknown/topic";
const char* DEVICE_KEY = "default_key";
bool battery_info_available = true;

// List of ESPs
const device_info_t ESPs[] = {
    {
        .device_name = "Living Room",
        .mac_address = {0xEC, 0x62, 0x60, 0xBC, 0xE8, 0x50},
        .device_id = "4",
        .device_topic = "1/4/data",
        .device_key = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzODAxNTYsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS80In0.pz7e__yvBeb-xrVAXNlY_6-GPg0PBvrMOzxsG9re_ohsAMgKnVddBFwaaRB15P07J-D4_s_1KpHDRNw0trIfXdnPTFaV0ibKzk2C-j6EGXlBRFi7POP0p_QMobHk5DtI54j9fpbxtAvl7uwQCWJlBY4w0rmynlJrEN5TRvu2veMtvN8HPOoYpw4k1L_jif_w0Jli-MM-aDhhuRFUO07hwqV1qoxArm0xcd4EW0u0OWM0Uvs9vW51Vr_BDb7-TgvywJQO9R8DCjXk3BBPG8BYavinuA4fTTC5oKzJRyRI3_zwv7DHaXMT3eD-tRMKxqvdBsaxpTG0UyCIQ9HrefKTVaE8JD6so1fbGdsMQ3qvjKtSamQYPFWMFhUGj7qmEwzjIqXBXdGEO1j7YTh3jG1fDaXXIvVSffj2_Hl1hCEwuiaPxh7DRQIhZVNV0Gv2IXq1_s7hB6byjXnUdQyJtUZS8xfdCPEP3YHDPe14fRUBZJDJrYsg2XPRGkLrpcPlIDSgDgv9nS_vO19lwX3QN-LmtJ7P2mYgVnG0ELljRAKtZvYhcKfoSyE6R1Amw5XlAiV5OcftdpayJLlqmMStQjakQLuxQVI6KALcCNkHvbOAM5zcAbSCgRNSdZppxlGzePu15ngtBZjL5xSuIEW6Y5NTM6E1v24kUhWUvvi9p42c3r0",
        .battery_info_available = true
    },
    {
        .device_name = "Kitchen",
        .mac_address = {0xEC, 0x62, 0x60, 0xBC, 0xE8, 0x18},
        .device_id = "5",
        .device_topic = "1/5/data",
        .device_key = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzODE1MzMsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS81In0.HVn4uZAQRIOvE6ZR16YnU9LhEqBmatBh7w2enEuiLxJt51ievhqYdRFW8DLjGW49zXdXP_u-h1LxRyyfkwrTFXY0fH8__3J4D1dLd3WRlfXyztkmCH846GRwMBoi-DLrXg4OW3BoVt2mGYhauOo4OwixXYNUCT1MfPZEpTb8DpcCbVfYHwEdd1y2WrF5BLguKyIuxUG6qwl2Llcti1porh5D78Onwt-OTiAAMyHlXHYg-DTEJR3qYSI2IuBtvU6jwO5G68BHIxL7Ug8GLbTdE0xtuwUvPKeupwyJPFBVFHTSD1s7p7F7PZhzJ5W6NQq4NibUnDHJNY7GrOXG9anApJpveC05xdKnraueZ41uJrg4leQlDRWTkFyeRPLBsX6Z_5TTA2kj4krEDQR4lmR9zilI1gJQV1h06NYwtO4Gx-48uXzhoo8JdckaxAM6m1mXTPOgGU-U2do3Cs3RhlsceUVTOld5rd0so4C4Ui3_X5TDA8uz8noPxyhxSYPKk4aZ8f80MjNsy2SicCxUv4jVhEH3iIzYi3m_0rFzynIf_oQbbVtkGKGyRjp3EEU-g9v-SvHdf4oYyA6kQ6YFe4IC6rABbH1rUp5jvUpJcsmgPqskbPTfnwZPCxWCRk5Jdc2piV5vHwxhguERu7Sg-fVZ4RIl2Er17FRHzvBZokUYWx8",
        .battery_info_available = true
    },
    {
        .device_name = "Bathroom",
        .mac_address = {0x94, 0x3C, 0xC6, 0xD1, 0x42, 0x2C},
        .device_id = "3",
        .device_topic = "1/3/data",
        .device_key = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzNzg3MzIsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS8zIn0.a0bD30ukZEU0lcRSPyn6wsXSgL2paInHwrJF4TJl12m9w8JivwD3_cqpsQn6QS_F3AkxHhBl6E2FjJyZEVimZKINMe0tMbmaGy6JejS4RVclgs1lw1t13Ml4BDZEZU9WRm4gSuWPEOeY_cMbbizg1PIx3juKi-_pRjEGMDpcalnQAw0wDbDUmImMNE8ifKV3_JgOsbzAhK_yW_Zn1EB4C8Vvroj7LAeOsBsAja_I1ejh2E0KeAU6aIS1-ZMni6Y3v5JL41RZiEGWhgqhU8XTpgLPfbUbRhAmP08QBRoHd6a7XOMW-uuHiuwTswYgm9wWxHKVWWVfrR--_LR7-26z06H5yUd_wQvegvcXjW30zuwwCKVwN6802jlnDXCXH8-j6WbMf6lVRCLl8MJNQjiAa8IHeW0DxCTDG_xvJ5P8dC5H7qkLa0EnOh65PlN8l2Rbja63MWg5q4hDNNc31AjeFjreJw50QI4Bgg3plpNqHfe9jcxducpMVd8Gn-_FC_isrcAHt1QWRajKtGlZeyOjZgzw6FKFM4UrdwWZ7NKhO1pAmkzCSG79-EbZT-viU9lkTz8WmcayejPt9pvmfPmGwe98EC1TatCB-SYHvmv5MbtnTjjGhaIECmRYiPEDushueAF5gVfJ1gsle8qsYOtOeEEPSz3CCe56HZfP_WUVBqs",
        .battery_info_available = false
    }
};

void app_main() {
    configPM();
    print_cpu_frequency();

    ESP_LOGI("progress", "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI("progress", "[APP] IDF version: %s", esp_get_idf_version());

    // Initalize the logging information
    initialize_logging();

    // Read the MAC address and identify the device
    uint8_t mac_address[6];
    esp_read_mac(mac_address, ESP_MAC_WIFI_STA);
    identify_device(mac_address);

    // If the battery info is available, retrive the battery status
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
    ESP_LOGI("progress", "Configuring RTC GPIOs");
    configure_rtc_gpio();

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
        if (wakeup_pin_mask & (1ULL << PIR_PIN)) {
            // PIR sensor caused wakeup
            ESP_LOGI("*", "******* Wakeup caused by PIR sensor *******");
            sendPIReventToMQTT();
        }
        if (wakeup_pin_mask & (1ULL << MAGNETIC_SWITCH_PIN)) {
            // Magnetic switch caused wakeup
            ESP_LOGI("*", "******* Wakeup caused by Magnetic Switch *******");
            sendMagneticSwitchEventToMQTT();
        }
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        ESP_LOGI("*", "Wakeup from light sleep");
    } else {
        ESP_LOGI("*", "Not a wakeup from deep sleep or light sleep");
    }

    ESP_LOGI("progress", "Waiting for sensors to become inactive");
    while (gpio_get_level(PIR_PIN) == 1 || gpio_get_level(MAGNETIC_SWITCH_PIN) == 1) {
        // Wait until both sensors are inactive (logic low)
        vTaskDelay(pdMS_TO_TICKS(SENSOR_INACTIVE_DELAY_MS));
    }

    // Create a bitmask for the GPIO pins
    uint64_t wakeup_pins_mask = (1ULL << PIR_PIN) | (1ULL << MAGNETIC_SWITCH_PIN);

    // Enable EXT1 wakeup on the selected pins with any high logic level
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(wakeup_pins_mask, ESP_EXT1_WAKEUP_ANY_HIGH));

    ESP_LOGI("progress", "Disconnecting from WIFI.");
    finish_wifi();

    ESP_LOGI("progress", "Going to sleep");
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    esp_deep_sleep_start();
}

void configPM(){
    esp_pm_config_esp32_t pm_config = {
    .max_freq_mhz = MAX_FREQ,
    .min_freq_mhz = MIN_FREQ, //DFS, enable in menucofig in Power Management
    .light_sleep_enable = LIGHT_SLEEP_ENABLE,
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
  }

void print_cpu_frequency() {
    // Get the current CPU frequency in Hz
    uint32_t cpu_freq_hz = esp_clk_cpu_freq();
    // Convert it to MHz for readability
    uint32_t cpu_freq_mhz = cpu_freq_hz / 1000000;
    // Print the current CPU frequency
    ESP_LOGI("CPU_FREQ", "Current CPU frequency: %u MHz", cpu_freq_mhz);
}

// Function to identify the device based on the MAC ADDRESS
void identify_device(const uint8_t* mac_address) {
    for (int i = 0; i < sizeof(ESPs) / sizeof(ESPs[0]); ++i) {
        if (memcmp(mac_address, ESPs[i].mac_address, sizeof(ESPs[i].mac_address)) == 0) {
            ESP_LOGI("*", "********** Device identified as %s. **********", ESPs[i].device_name);
            DEVICE_ID = ESPs[i].device_id;
            DEVICE_TOPIC = ESPs[i].device_topic;
            DEVICE_KEY = ESPs[i].device_key;
            battery_info_available = ESPs[i].battery_info_available;
            ESP_LOGI("*", "DEVICE_ID: %s", DEVICE_ID);
            ESP_LOGI("*", "DEVICE_TOPIC: %s", DEVICE_TOPIC);
            ESP_LOGI("*", "DEVICE_KEY: %s", DEVICE_KEY);
            ESP_LOGI("*", "Battery info available: %s", battery_info_available ? "true" : "false");
            return;
        }
    }
    ESP_LOGI("*", "Device not recognized.");
}

void initialize_logging() {
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt", ESP_LOG_INFO);
    esp_log_level_set("progress", ESP_LOG_INFO);
    esp_log_level_set("gauge", ESP_LOG_INFO);
}

// Configure RTC GPIOs for PIR and Magnetic Switch
void configure_rtc_gpio() {
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
}
