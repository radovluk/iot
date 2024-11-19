#pragma once

#include <stdio.h>
#include <stdbool.h>

// #define EXAMPLE_ESP_WIFI_SSID      "CAPS-Seminar-Room"
// #define EXAMPLE_ESP_WIFI_PASS      "caps-schulz-seminar-room-wifi"
// #define SNTP_SERVER_NAME           "ntp1.in.tum.de"
// #define MQTT_BROKER                "131.159.85.125" 

#define EXAMPLE_ESP_WIFI_SSID      "vbMobile24G"
#define EXAMPLE_ESP_WIFI_PASS      "vbMobile"
#define SNTP_SERVER_NAME            "pool.ntp.org"
#define MQTT_BROKER                "192.168.81.143"

// Struct for PIR event
typedef struct {
    uint64_t timestamp;
    int device_id;
} PIR_Event_t;

// Definiton of the maximum number of the PIR events stored in RTC memory
#define PIR_EVENTS_ARRAY_SIZE 3  // Configurable number of PIR events to store. Make sure it is at least MAX_PIR_EVENTS!
extern RTC_DATA_ATTR uint32_t MAX_PIR_EVENTS;  // Configurable number of PIR events to store, adjust together with PIR_EVENTS_ARRAY_SIZE
extern RTC_DATA_ATTR PIR_Event_t pir_events[PIR_EVENTS_ARRAY_SIZE];

// Define intervals as RTC memory variables
extern RTC_DATA_ATTR uint32_t BATTERY_INFO_INTERVAL_SEC;  // Interval for sending battery information
extern RTC_DATA_ATTR uint32_t AUTOMATIC_WAKEUP_INTERVAL_SEC; // Automatic wakeup interval

// Store GPIO pins in RTC memory
extern RTC_DATA_ATTR int PIR_PIN;              // PIR sensor pin
extern RTC_DATA_ATTR int MAGNETIC_SWITCH_PIN;  // Magnetic switch pin

// Define max and min frequencies
#define MAX_FREQ 240
#define MIN_FREQ 80

// Define approximate slow clock freuency
#define RTC_SLOW_CLK_FREQ_APPROX 150000ULL 

// Define light sleep (in current implementation the light sleep colides with wakeup reason detection)
#define LIGHT_SLEEP_ENABLE false

// Sensor inactive delay in milliseconds (wating time in the while loop for deactivation of the sensors)
extern RTC_DATA_ATTR uint32_t SENSOR_INACTIVE_DELAY_MS;

// Device information structure
typedef struct {
    const char* device_name;
    const uint8_t mac_address[6];
    const int device_id;
    const char* device_topic;
    const char* device_key;
    bool battery_info_available;
} device_info_t;

// Extern declarations for variables defining the DEVICE, stored in RTC memory
extern RTC_DATA_ATTR int DEVICE_ID;
extern RTC_DATA_ATTR char DEVICE_TOPIC[512];
extern RTC_DATA_ATTR char DEVICE_KEY[1024];
extern RTC_DATA_ATTR bool battery_info_available;

// Declare the ESPs array as extern
extern const device_info_t ESPs[];

// The counter of the PIR events stored in the RTC memory
extern RTC_DATA_ATTR int pir_event_count;

// Extern declarations for time synchronization variables during wake up stub
extern RTC_DATA_ATTR uint64_t rtc_time_at_last_sync;
extern RTC_DATA_ATTR uint64_t actual_time_at_last_sync;

// Declare the functions used in main.c
void configPM();
void print_cpu_frequency();
void identify_device(const uint8_t* mac_address);
void initialize_logging();
void configure_rtc_gpio();
void handle_wakeup_reason();
uint64_t my_rtc_time_get_us(void);
void RTC_IRAM_ATTR my_wakeup_stub(void);
void updateBatteryStatus();
void handlePIReventsArray();
uint64_t get_current_time_in_ms();
uint64_t get_time_since_boot_in_ms();