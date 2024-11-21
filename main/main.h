#pragma once

// --------------------------------- Configuration ---------------------------------

// #define EXAMPLE_ESP_WIFI_SSID      "CAPS-Seminar-Room"
// #define EXAMPLE_ESP_WIFI_PASS      "caps-schulz-seminar-room-wifi"
// #define SNTP_SERVER_NAME           "ntp1.in.tum.de"
// #define MQTT_BROKER                "131.159.85.125" 

#define EXAMPLE_ESP_WIFI_SSID      "vbMobile24G"
#define EXAMPLE_ESP_WIFI_PASS      "vbMobile"
#define SNTP_SERVER_NAME            "pool.ntp.org"
#define MQTT_BROKER                "192.168.81.143"

// Device Configuration
#define CONFIG_MAX_PIR_EVENTS              10              // < Maximum number of PIR events stored in RTC memory.
#define CONFIG_BATTERY_INFO_INTERVAL_SEC   180             // < Interval (in seconds) to send battery information to MQTT.
#define CONFIG_WAKEUP_INTERVAL_SEC         20              // < Interval (in seconds) for automatic wakeup.
#define CONFIG_SENSOR_INACTIVE_DELAY_MS    3000            // < Delay (in milliseconds) for sensors to become inactive after triggering.

// GPIO Configuration
#define CONFIG_PIR_PIN 27                  // GPIO for PIR sensor
#define CONFIG_MAGNETIC_SWITCH_PIN 33      // GPIO for Magnetic switch sensor

// CPU Frequency Settings
#define CONFIG_MAX_FREQ 240                // Maximum CPU frequency in MHz
#define CONFIG_MIN_FREQ 80                 // Minimum CPU frequency in MHz

// List of ESPs (name, mac adress, topic, key, battery info available)
#define ESP_DEVICE_1 {"Living Room", {0xEC, 0x62, 0x60, 0xBC, 0xE8, 0x50}, 4, "1/4/data", "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzODAxNTYsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS80In0.pz7e__yvBeb-xrVAXNlY_6-GPg0PBvrMOzxsG9re_ohsAMgKnVddBFwaaRB15P07J-D4_s_1KpHDRNw0trIfXdnPTFaV0ibKzk2C-j6EGXlBRFi7POP0p_QMobHk5DtI54j9fpbxtAvl7uwQCWJlBY4w0rmynlJrEN5TRvu2veMtvN8HPOoYpw4k1L_jif_w0Jli-MM-aDhhuRFUO07hwqV1qoxArm0xcd4EW0u0OWM0Uvs9vW51Vr_BDb7-TgvywJQO9R8DCjXk3BBPG8BYavinuA4fTTC5oKzJRyRI3_zwv7DHaXMT3eD-tRMKxqvdBsaxpTG0UyCIQ9HrefKTVaE8JD6so1fbGdsMQ3qvjKtSamQYPFWMFhUGj7qmEwzjIqXBXdGEO1j7YTh3jG1fDaXXIvVSffj2_Hl1hCEwuiaPxh7DRQIhZVNV0Gv2IXq1_s7hB6byjXnUdQyJtUZS8xfdCPEP3YHDPe14fRUBZJDJrYsg2XPRGkLrpcPlIDSgDgv9nS_vO19lwX3QN-LmtJ7P2mYgVnG0ELljRAKtZvYhcKfoSyE6R1Amw5XlAiV5OcftdpayJLlqmMStQjakQLuxQVI6KALcCNkHvbOAM5zcAbSCgRNSdZppxlGzePu15ngtBZjL5xSuIEW6Y5NTM6E1v24kUhWUvvi9p42c3r0", true}
#define ESP_DEVICE_2 {"Kitchen", {0xEC, 0x62, 0x60, 0xBC, 0xE8, 0x18}, 5, "1/5/data", "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzODE1MzMsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS81In0.HVn4uZAQRIOvE6ZR16YnU9LhEqBmatBh7w2enEuiLxJt51ievhqYdRFW8DLjGW49zXdXP_u-h1LxRyyfkwrTFXY0fH8__3J4D1dLd3WRlfXyztkmCH846GRwMBoi-DLrXg4OW3BoVt2mGYhauOo4OwixXYNUCT1MfPZEpTb8DpcCbVfYHwEdd1y2WrF5BLguKyIuxUG6qwl2Llcti1porh5D78Onwt-OTiAAMyHlXHYg-DTEJR3qYSI2IuBtvU6jwO5G68BHIxL7Ug8GLbTdE0xtuwUvPKeupwyJPFBVFHTSD1s7p7F7PZhzJ5W6NQq4NibUnDHJNY7GrOXG9anApJpveC05xdKnraueZ41uJrg4leQlDRWTkFyeRPLBsX6Z_5TTA2kj4krEDQR4lmR9zilI1gJQV1h06NYwtO4Gx-48uXzhoo8JdckaxAM6m1mXTPOgGU-U2do3Cs3RhlsceUVTOld5rd0so4C4Ui3_X5TDA8uz8noPxyhxSYPKk4aZ8f80MjNsy2SicCxUv4jVhEH3iIzYi3m_0rFzynIf_oQbbVtkGKGyRjp3EEU-g9v-SvHdf4oYyA6kQ6YFe4IC6rABbH1rUp5jvUpJcsmgPqskbPTfnwZPCxWCRk5Jdc2piV5vHwxhguERu7Sg-fVZ4RIl2Er17FRHzvBZokUYWx8", true}
#define ESP_DEVICE_3 {"Bathroom", {0x94, 0x3C, 0xC6, 0xD1, 0x42, 0x2C}, 3, "1/3/data", "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzAzNzg3MzIsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS8zIn0.a0bD30ukZEU0lcRSPyn6wsXSgL2paInHwrJF4TJl12m9w8JivwD3_cqpsQn6QS_F3AkxHhBl6E2FjJyZEVimZKINMe0tMbmaGy6JejS4RVclgs1lw1t13Ml4BDZEZU9WRm4gSuWPEOeY_cMbbizg1PIx3juKi-_pRjEGMDpcalnQAw0wDbDUmImMNE8ifKV3_JgOsbzAhK_yW_Zn1EB4C8Vvroj7LAeOsBsAja_I1ejh2E0KeAU6aIS1-ZMni6Y3v5JL41RZiEGWhgqhU8XTpgLPfbUbRhAmP08QBRoHd6a7XOMW-uuHiuwTswYgm9wWxHKVWWVfrR--_LR7-26z06H5yUd_wQvegvcXjW30zuwwCKVwN6802jlnDXCXH8-j6WbMf6lVRCLl8MJNQjiAa8IHeW0DxCTDG_xvJ5P8dC5H7qkLa0EnOh65PlN8l2Rbja63MWg5q4hDNNc31AjeFjreJw50QI4Bgg3plpNqHfe9jcxducpMVd8Gn-_FC_isrcAHt1QWRajKtGlZeyOjZgzw6FKFM4UrdwWZ7NKhO1pAmkzCSG79-EbZT-viU9lkTz8WmcayejPt9pvmfPmGwe98EC1TatCB-SYHvmv5MbtnTjjGhaIECmRYiPEDushueAF5gVfJ1gsle8qsYOtOeEEPSz3CCe56HZfP_WUVBqs", false}

#define CONFIG_LIGHT_SLEEP_ENABLE false // < Whether light sleep mode is enabled.

// --------------------------------- Logging Configuration ---------------------------------

/** @brief Logging level for general application logs. */
#define APP_LOG_LEVEL        ESP_LOG_INFO

/** @brief Logging level for MQTT module. */
#define MQTT_LOG_LEVEL       ESP_LOG_INFO

/** @brief Logging level for sensor-related logs. */
#define SENSOR_LOG_LEVEL     ESP_LOG_INFO

/** @brief Logging level for battery-related logs. */
#define BATTERY_LOG_LEVEL    ESP_LOG_INFO

/** @brief Logging level for progress logs. */
#define PROGRESS_LOG_LEVEL   ESP_LOG_INFO

/** @brief Logging level for gauge module. */
#define GAUGE_LOG_LEVEL      ESP_LOG_INFO

/** @brief Logging level for wakeup stub logging. (false or true) */
#define WAKEUP_STUB_LOG_LEVEL true

// --------------------------------- Struct Definitions ---------------------------------

#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Represents the configuration and metadata for a device.
 *
 * This struct is used to define the properties of an ESP device, including its
 * name, MAC address, ID, MQTT topic, security key, and whether battery information
 * is available. The struct can be used to identify devices and manage their specific
 * configurations within the system.
 */
typedef struct {
    const char* device_name;        // < Name of the device (e.g., "Living Room").
    const uint8_t mac_address[6];   // < MAC address of the device (6 bytes).
    int device_id;                  // < Unique identifier for the device.
    const char* device_topic;       // < MQTT topic for publishing device data.
    const char* device_key;         // < Security key for authenticating with the MQTT broker.
    bool battery_info_available;    // < Indicates if the device provides battery information.
} device_info_t;

/**
 * @brief Represents the strict for PIR event
 */
typedef struct {
    uint64_t timestamp;  // < The actual unix time stamp in ms
    int device_id;       // < Unique identifier for the device.
} PIR_Event_t;

// --------------------------------- Extern RTC Variables (Stored in RTC Memory) ---------------------------------

// maximum number of the PIR events stored in RTC memory
extern RTC_DATA_ATTR uint32_t MAX_PIR_EVENTS;

// Definiton of the maximum number of the PIR events stored in RTC memory
extern RTC_DATA_ATTR PIR_Event_t pir_events[CONFIG_MAX_PIR_EVENTS];

// Define intervals as RTC memory variables
extern RTC_DATA_ATTR uint32_t BATTERY_INFO_INTERVAL_SEC;  // Interval for sending battery information
extern RTC_DATA_ATTR uint32_t AUTOMATIC_WAKEUP_INTERVAL_SEC; // Automatic wakeup interval

// Store GPIO pins in RTC memory
extern RTC_DATA_ATTR int PIR_PIN;              // PIR sensor pin
extern RTC_DATA_ATTR int MAGNETIC_SWITCH_PIN;  // Magnetic switch pin

// Sensor inactive delay in milliseconds (wating time in the while loop for deactivation of the sensors)
extern RTC_DATA_ATTR uint32_t SENSOR_INACTIVE_DELAY_MS;

// Extern declarations for variables defining the DEVICE, stored in RTC memory
extern RTC_DATA_ATTR int DEVICE_ID;
extern RTC_DATA_ATTR char DEVICE_TOPIC[512];
extern RTC_DATA_ATTR char DEVICE_KEY[1024];
extern RTC_DATA_ATTR bool battery_info_available;

// Declare the ESPs array as extern
extern RTC_DATA_ATTR device_info_t this_device;

// The counter of the PIR events stored in the RTC memory
extern RTC_DATA_ATTR int pir_event_count;

// Extern declarations for time synchronization variables during wake up stub
extern RTC_DATA_ATTR uint64_t rtc_time_at_last_sync;
extern RTC_DATA_ATTR uint64_t actual_time_at_last_sync;

// --------------------------------- Function Declarations ---------------------------------

/**
 * @brief Configures power management settings.
 *
 * This function sets the CPU frequency scaling and enables or disables light sleep
 * based on the configuration defined in `main.h`.
 *
 * - It ensures the system operates efficiently by dynamically adjusting the CPU frequency.
 * - Light sleep is optionally enabled to reduce power consumption.
 */
void configPM(void);

/**
 * @brief Prints the current CPU frequency.
 *
 * Retrieves and logs the current CPU frequency in MHz. Useful for debugging and verifying
 * that the power management configuration is applied correctly.
 */
void print_cpu_frequency(void);

/**
 * @brief Identifies the current device based on its MAC address.
 *
 * Matches the MAC address of the device with the pre-configured device list in `main.h`.
 * Sets the device's ID, MQTT topic, security key, and battery information availability.
 *
 * @param mac_address Pointer to the MAC address array of the device.
 */
void identify_device(const uint8_t* mac_address);

/**
 * @brief Initializes logging levels for various components based on configuration.
 *
 * Sets the logging levels for different modules (e.g., MQTT, application, sensors, progress, gauge)
 * according to the settings defined in `main.h`. This allows for flexible control
 * over the verbosity of logs for each component.
 */
void initialize_logging(void);

/**
 * @brief Configures the RTC GPIOs for sensors.
 *
 * Initializes the GPIO pins connected to the PIR sensor and the magnetic switch sensor.
 * Configures the pins as inputs with appropriate pull-up or pull-down resistors.
 */
void configure_rtc_gpio(void);

/**
 * @brief Handles the wakeup reason after deep sleep.
 *
 * Determines the cause of the system's wakeup (e.g., timer, PIR sensor, or magnetic switch).
 * Logs the wakeup cause and performs specific actions based on the trigger.
 */
void handle_wakeup_reason(void);

/**
 * @brief Retrieves the current RTC time in microseconds.
 *
 * Reads the RTC clock and calculates the current time in microseconds.
 * This function is used to timestamp events during low-power wakeup.
 *
 * @return Current RTC time in microseconds.
 */
uint64_t my_rtc_time_get_us(void);

/**
 * @brief Custom wakeup stub function for handling minimal tasks during wakeup.
 *
 * Executes in RTC memory during the system's wakeup process. Handles sensor triggers,
 * updates timestamps, and decides whether to wake up the main application or return
 * to deep sleep.
 *
 * @note This function runs in a minimal runtime environment (no standard libraries).
 */
void RTC_IRAM_ATTR my_wakeup_stub(void);

/**
 * @brief Updates the battery status and sends it to the MQTT broker.
 *
 * Checks if the interval for sending battery information has elapsed. If so, retrieves
 * the battery voltage and state of charge (SOC) and publishes the data to the configured
 * MQTT topic.
 */
void updateBatteryStatus(void);

/**
 * @brief Handles the array of PIR events stored in RTC memory.
 *
 * If PIR events are stored in memory, this function flushes them to the MQTT broker
 * as a batch. Resets the event count after successful transmission.
 */
void handlePIReventsArray(void);

/**
 * @brief Retrieves the current time in milliseconds since the Epoch.
 *
 * Uses the system clock to calculate the precise current time, including seconds
 * and milliseconds. This is used for timestamping events and synchronizing logs.
 *
 * @return Current time in milliseconds since the Epoch.
 */
uint64_t get_current_time_in_ms(void);

/**
 * @brief Calculates the time since the system booted in milliseconds.
 *
 * Retrieves the time elapsed since the system booted, based on the high-resolution
 * timer. Useful for measuring durations or synchronizing tasks.
 *
 * @return Time since boot in milliseconds.
 */
uint64_t get_time_since_boot_in_ms(void);
