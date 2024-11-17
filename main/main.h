#pragma once

// #define EXAMPLE_ESP_WIFI_SSID      "CAPS-Seminar-Room"
// #define EXAMPLE_ESP_WIFI_PASS      "caps-schulz-seminar-room-wifi"
// #define SNTP_SERVER_NAME           "ntp1.in.tum.de"
// #define MQTT_BROKER                "131.159.85.125" 

#define EXAMPLE_ESP_WIFI_SSID      "vbMobile24G"
#define EXAMPLE_ESP_WIFI_PASS      "vbMobile"
#define SNTP_SERVER_NAME            "pool.ntp.org"
#define MQTT_BROKER                "192.168.81.143"

#include <stdio.h>
#include <stdbool.h>

// Define GPIO pins
#define PIR_PIN 27
#define MAGNETIC_SWITCH_PIN 33

// Define max and min frequencies
#define MAX_FREQ 240
#define MIN_FREQ 80

// Define light sleep (in current implementation the light sleep colides with wakeup reason detection)
#define LIGHT_SLEEP_ENABLE false

// Sensor inactive delay in milliseconds
#define SENSOR_INACTIVE_DELAY_MS 5000

// Device information structure
typedef struct {
    const char* device_name;
    const uint8_t mac_address[6];
    const char* device_id;
    const char* device_topic;
    const char* device_key;
    bool battery_info_available;
} device_info_t;

// Extern declarations for global variables
extern const char* DEVICE_ID;
extern const char* DEVICE_TOPIC;
extern const char* DEVICE_KEY;
extern bool battery_info_available;

// Declare the ESPs array as extern
extern const device_info_t ESPs[];

// Declare the functions used in main.c
void configPM();
void print_cpu_frequency();
void identify_device(const uint8_t* mac_address);
void initialize_logging();
void configure_rtc_gpio();