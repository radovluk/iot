// Standard Libraries
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// ESP-IDF Core Headers
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_mac.h"
#include "esp_pm.h"
#include "esp32/rtc.h"
#include "esp_wake_stub.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

// Project-Specific Headers
#include "sdkconfig.h"
#include "wifi.h"
#include "mqtt.h"
#include "sntp.h"
#include "main.h"
#include "gauge.h"
#include "rtc_wake_stub.h"

// RTC slow memory config variables
RTC_DATA_ATTR uint32_t MAX_PIR_EVENTS = CONFIG_MAX_PIR_EVENTS;
RTC_DATA_ATTR uint32_t BATTERY_INFO_INTERVAL_SEC = CONFIG_BATTERY_INFO_INTERVAL_SEC;
RTC_DATA_ATTR uint32_t AUTOMATIC_WAKEUP_INTERVAL_SEC = CONFIG_WAKEUP_INTERVAL_SEC;
RTC_DATA_ATTR int PIR_PIN = CONFIG_PIR_PIN;
RTC_DATA_ATTR int MAGNETIC_SWITCH_PIN = CONFIG_MAGNETIC_SWITCH_PIN;
RTC_DATA_ATTR uint32_t SENSOR_INACTIVE_DELAY_MS = CONFIG_SENSOR_INACTIVE_DELAY_MS;
RTC_DATA_ATTR PIR_Event_t pir_events[CONFIG_MAX_PIR_EVENTS];

// Keeps track of the last time battery information was sent
RTC_DATA_ATTR uint64_t last_battery_info_time = 0;

// Counter of the already stored PIR events, stored in RTC memory
RTC_DATA_ATTR int pir_event_count = 0;

// Define this_device variable
RTC_DATA_ATTR device_info_t this_device;

// Extern declarations for time synchronization variables during wake up stub
RTC_DATA_ATTR uint64_t rtc_time_at_last_sync = 0;
RTC_DATA_ATTR uint64_t actual_time_at_last_sync = 0;

// List of ESPs
const device_info_t ESPs[] = {ESP_DEVICE_1, ESP_DEVICE_2, ESP_DEVICE_3};

// Sleep_enter_time stored in RTC memory
static RTC_DATA_ATTR struct timeval sleep_enter_time;

// Main application
void app_main(void)
{
    ESP_LOGI("progress", "Booting the main app");
    
    // Configure frequency and the light sleep
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

    // Initialize NVS, erase and reinitialize if storage is full or version mismatch occurs
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

    // Synchronize RTC time and actual time
    actual_time_at_last_sync = get_current_time_in_ms();
    rtc_time_at_last_sync = get_time_since_boot_in_ms();
    ESP_LOGI("progress", "updating RTC time at last sync: %llu ms", rtc_time_at_last_sync);
    ESP_LOGI("progress", "updating actual time at last sync: %llu ms", actual_time_at_last_sync);
    // Check if it is time to send the battery status and if so, do so.
    updateBatteryStatus();

    // Check if there are any PIR events in pir events array and send them to MQTT
    handlePIReventsArray();

    // Configure RTC GPIOs for PIR and Magnetic Switch
    ESP_LOGI("progress", "Configuring RTC GPIOs");
    configure_rtc_gpio();

    // Determine the wakeup reason
    ESP_LOGI("progress", "Determining the wakeup reason.");
    handle_wakeup_reason();

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

    ESP_LOGI("progress", "Enabling timer wakeup, %ds\n", AUTOMATIC_WAKEUP_INTERVAL_SEC);
    esp_sleep_enable_timer_wakeup(AUTOMATIC_WAKEUP_INTERVAL_SEC * 1000000);

#if CONFIG_IDF_TARGET_ESP32
    // Isolate GPIO12 pin from external circuits. This is needed for modules
    // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
    // to minimize current consumption.
    rtc_gpio_isolate(GPIO_NUM_12);
#endif

    ESP_LOGI("progress", "Enabling timer wakeup in wake up stub every %ds\n", AUTOMATIC_WAKEUP_INTERVAL_SEC);
    esp_set_deep_sleep_wake_stub(&wake_stub);

    gettimeofday(&sleep_enter_time, NULL);
    printf("progress", "Entering deep sleep\n");
    esp_deep_sleep_start();
}

/**
 * @brief Configures power management settings.
 *
 * This function sets the CPU frequency scaling and enables or disables light sleep
 * based on the configuration defined in `main.h`.
 *
 * - It ensures the system operates efficiently by dynamically adjusting the CPU frequency.
 * - Light sleep is optionally enabled to reduce power consumption.
 */
void configPM(){
    esp_pm_config_esp32_t pm_config = {
    .max_freq_mhz = CONFIG_MAX_FREQ,
    .min_freq_mhz = CONFIG_MIN_FREQ, //DFS, enable in menucofig in Power Management
    .light_sleep_enable = CONFIG_LIGHT_SLEEP_ENABLE,
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
  }

/**
 * @brief Prints the current CPU frequency.
 *
 * Retrieves and logs the current CPU frequency in MHz. Useful for debugging and verifying
 * that the power management configuration is applied correctly.
 */
void print_cpu_frequency() {
    // Get the current CPU frequency in Hz
    uint32_t cpu_freq_hz = esp_clk_cpu_freq();
    // Convert it to MHz for readability
    uint32_t cpu_freq_mhz = cpu_freq_hz / 1000000;
    // Print the current CPU frequency
    ESP_LOGI("progress", "Current CPU frequency: %u MHz", cpu_freq_mhz);
}

/**
 * @brief Identifies the current device based on its MAC address.
 *
 * Matches the MAC address of the device with the pre-configured device list in `main.h`.
 * Sets the device's ID, MQTT topic, security key, and battery information availability.
 *
 * @param mac_address Pointer to the MAC address array of the device.
 */
void identify_device(const uint8_t* mac_address) {
    for (int i = 0; i < sizeof(ESPs) / sizeof(ESPs[0]); ++i) {
        if (memcmp(mac_address, ESPs[i].mac_address, sizeof(ESPs[i].mac_address)) == 0) {
            // Copy the device info into this_device
            memcpy(&this_device, &ESPs[i], sizeof(device_info_t));
            // Logging
            ESP_LOGI("*", "*********** Device identified as %s", this_device.device_name);
            return;
        }
    }
    ESP_LOGI("*", "Device not recognized.");
}

/**
 * @brief Initializes logging levels for various components based on configuration.
 *
 * Sets the logging levels for different modules (e.g., MQTT, application, sensors, progress, gauge)
 * according to the settings defined in `main.h`. This allows for flexible control
 * over the verbosity of logs for each component.
 */
void initialize_logging(void)
{
    // Set logging levels based on configuration
    esp_log_level_set("*", APP_LOG_LEVEL);
    esp_log_level_set("mqtt", MQTT_LOG_LEVEL);
    esp_log_level_set("sensor", SENSOR_LOG_LEVEL);
    esp_log_level_set("battery", BATTERY_LOG_LEVEL);
    esp_log_level_set("progress", PROGRESS_LOG_LEVEL);
    esp_log_level_set("gauge", GAUGE_LOG_LEVEL);
}

/**
 * @brief Configures the RTC GPIOs for sensors.
 *
 * Initializes the GPIO pins connected to the PIR sensor and the magnetic switch sensor.
 * Configures the pins as inputs with appropriate pull-up or pull-down resistors.
 */
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

/**
 * @brief Handles the wakeup reason after deep sleep.
 *
 * Determines the cause of the system's wakeup (e.g., timer, PIR sensor, or magnetic switch).
 * Logs the wakeup cause and performs specific actions based on the trigger.
 */
void handle_wakeup_reason(){
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;
    printf("Time spent in deep sleep: %dms. RTC time: %llus\n", sleep_time_ms, esp_rtc_get_time_us()/1000000);

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
        // Wakeup caused by external GPIO (EXT1)
        uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
        if (wakeup_pin_mask & (1ULL << PIR_PIN)) {
            ESP_LOGI("*", "Wakeup caused by PIR sensor");
        }
        if (wakeup_pin_mask & (1ULL << MAGNETIC_SWITCH_PIN)) {
            ESP_LOGI("*", "Wakeup caused by Magnetic Switch");
            sendMagneticSwitchEventToMQTT();
        }
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        // Wakeup caused by timer
        ESP_LOGI("*", "Wakeup caused by automatic timer.");
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
        // Device just started (not a wakeup)
        ESP_LOGI("*", "Device started (not a wakeup)");
    } else {
        // Other wakeup reasons
        ESP_LOGI("*", "Wakeup from unknown source (reason: %d)", wakeup_reason);
    }
}

/**
 * @brief Updates the battery status and sends it to the MQTT broker.
 *
 * Checks if the interval for sending battery information has elapsed. If so, retrieves
 * the battery voltage and state of charge (SOC) and publishes the data to the configured
 * MQTT topic.
 */
void updateBatteryStatus() {
    struct timeval now;
    gettimeofday(&now, NULL);

    // Convert `struct timeval` to milliseconds
    uint64_t now_ms = (uint64_t)now.tv_sec * 1000 + (uint64_t)now.tv_usec / 1000;
    uint64_t last_update_ms = last_battery_info_time;

    if ((now_ms - last_update_ms) >= BATTERY_INFO_INTERVAL_SEC * 1000) {
        ESP_LOGI("battery", "Time to send battery information");
        if (this_device.battery_info_available) {
            getRSOC();
            ESP_LOGI("battery", "Sending battery status to MQTT");
            sendBatteryStatusToMQTT();
            last_battery_info_time = now_ms; // Update the timestamp
        }
    } else {
        uint64_t time_left_ms = BATTERY_INFO_INTERVAL_SEC * 1000 - (now_ms - last_update_ms);
        uint64_t time_left_sec = time_left_ms / 1000; // Convert milliseconds to seconds
        ESP_LOGI("battery", "Not time to send battery status yet. Time left: %llu seconds.", time_left_sec);
    }
}

/**
 * @brief Handles the array of PIR events stored in RTC memory.
 *
 * If PIR events are stored in memory, this function flushes them to the MQTT broker
 * as a batch. Resets the event count after successful transmission.
 */
void handlePIReventsArray() {
    if (pir_event_count > 0) {
        ESP_LOGI("PIR", "Found %d stored PIR events. Flushing to MQTT.", pir_event_count);
        sendPIReventsToMQTT();    
        // Reset the PIR event count in sendPIREventsFunction
    } else {
        ESP_LOGI("PIR", "No stored PIR events to send.");
    }
}

/**
 * @brief Retrieves the current time in milliseconds since the Epoch.
 *
 * Uses the system clock to calculate the precise current time, including seconds
 * and milliseconds. This is used for timestamping events and synchronizing logs.
 *
 * @return Current time in milliseconds since the Epoch.
 */
uint64_t get_current_time_in_ms() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return (uint64_t)(now.tv_sec) * 1000 + (now.tv_usec) / 1000; // Convert to milliseconds
}

/**
 * @brief Calculates the time since the system booted in milliseconds.
 *
 * Retrieves the time elapsed since the system booted, based on the high-resolution
 * timer. Useful for measuring durations or synchronizing tasks.
 *
 * @return Time since boot in milliseconds.
 */
uint64_t get_time_since_boot_in_ms() {
    return esp_timer_get_time() / 1000; // Convert microseconds to milliseconds
}