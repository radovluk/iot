# ESP32 Sensor Monitoring with Wake-Up Stub

## Introduction

This project is an ESP32-based application designed to monitor sensors (such as PIR sensors and magnetic switches), manage power consumption efficiently, and communicate with an MQTT broker to report events and battery status. The application utilizes a wake-up stub to handle events during deep sleep, allowing for low-power operation while maintaining responsiveness to sensor triggers.

## Configuration

The configuration is primarily defined in `main.h`. Key configuration settings include Wi-Fi credentials, SNTP server, MQTT broker, device-specific configurations, GPIO pins, CPU frequency settings, and logging levels.

### Wi-Fi and Network Configuration

```c
#define EXAMPLE_ESP_WIFI_SSID      "XXX"
#define EXAMPLE_ESP_WIFI_PASS      "XXX"
#define SNTP_SERVER_NAME           "pool.ntp.org"
#define MQTT_BROKER                "192.168.81.143"
```
- **EXAMPLE_ESP_WIFI_SSID**: The SSID of the Wi-Fi network.
- **EXAMPLE_ESP_WIFI_PASS**: The password for the Wi-Fi network.
- **SNTP_SERVER_NAME**: The SNTP server for time synchronization.
- **MQTT_BROKER**: The IP address or hostname of the MQTT broker.

### Device Configuration

```c
#define CONFIG_MAX_PIR_EVENTS              2
#define CONFIG_BATTERY_INFO_INTERVAL_SEC   180
#define CONFIG_WAKEUP_INTERVAL_SEC         20
#define CONFIG_SENSOR_INACTIVE_DELAY_MS    3000
#define CONFIG_SENSOR_INACTIVE_DELAY_IN_WAKE_UP_STUB_SEC 4
```
- **CONFIG_MAX_PIR_EVENTS**: Maximum number of PIR events stored in RTC memory.
- **CONFIG_BATTERY_INFO_INTERVAL_SEC**: Interval for sending battery information to MQTT.
- **CONFIG_WAKEUP_INTERVAL_SEC**: Interval for automatic wakeup from deep sleep.
- **CONFIG_SENSOR_INACTIVE_DELAY_MS**: Delay for sensors to become inactive after triggering.
- **CONFIG_SENSOR_INACTIVE_DELAY_IN_WAKE_UP_STUB_SEC**: Delay for sensors to become inactive during wake-up stub.

### GPIO Configuration

```c
#define CONFIG_PIR_PIN                27
#define CONFIG_MAGNETIC_SWITCH_PIN    33
```
- **CONFIG_PIR_PIN**: GPIO pin for the PIR sensor.
- **CONFIG_MAGNETIC_SWITCH_PIN**: GPIO pin for the magnetic switch sensor.

### CPU Frequency Settings

```c
#define CONFIG_MAX_FREQ 240
#define CONFIG_MIN_FREQ 80
```
- **CONFIG_MAX_FREQ**: Maximum CPU frequency in MHz.
- **CONFIG_MIN_FREQ**: Minimum CPU frequency in MHz.

### Logging Configuration

```c
#define APP_LOG_LEVEL        ESP_LOG_INFO
#define MQTT_LOG_LEVEL       ESP_LOG_INFO
#define SENSOR_LOG_LEVEL     ESP_LOG_INFO
#define BATTERY_LOG_LEVEL    ESP_LOG_INFO
#define PROGRESS_LOG_LEVEL   ESP_LOG_INFO
#define GAUGE_LOG_LEVEL      ESP_LOG_INFO
#define WAKEUP_STUB_LOG_LEVEL true
```
- **APP_LOG_LEVEL**: General application logging level.
- **MQTT_LOG_LEVEL**: MQTT module logging level.
- **SENSOR_LOG_LEVEL**: Sensor-related logs.
- **BATTERY_LOG_LEVEL**: Battery-related logs.
- **WAKEUP_STUB_LOG_LEVEL**: Logging in the wake-up stub (true or false).

## Wake-Up Stub Functionality

The wake-up stub is a minimal piece of code that runs immediately upon the ESP32 waking from deep sleep. It operates in the RTC fast memory, allowing for efficient power consumption while executing basic tasks before the main application starts.

### How It Works

1. **Wake-Up Cause Determination**: The wake-up stub identifies the wake-up cause (e.g., timer, external GPIO).

2. **Sensor Handling**:
   - If the wake-up is triggered by a sensor (PIR or magnetic switch), it processes the sensor's status.
   - For PIR sensor triggers:
     - Stores the event with a timestamp in RTC memory.
     - If the maximum number of events is reached, it wakes the main application for further processing.
     - If not, it returns to deep sleep.
   - For magnetic switch triggers, it wakes the main application immediately.

3. **Battery Status Check**: Periodically checks if it's time to update the battery status and wakes the main application to send battery information if needed.

4. **Deep Sleep Management**: Sets the next wake-up time and returns to deep sleep if no immediate action is required.

### Key Features

- **RTC Memory Variables**: Variables prefixed with `RTC_DATA_ATTR` are preserved during deep sleep cycles, allowing consistent behavior across wake-ups.
- **Sensor Debouncing**: Includes logic to avoid multiple wake-ups caused by still-active sensors by waiting a specific delay (`CONFIG_SENSOR_INACTIVE_DELAY_IN_WAKE_UP_STUB_SEC`) before deciding to go back to sleep.
- **Battery Status Update**: Checks if it is time to send battery status and wakes up the main application if necessary.

## How to Use

1. **Setup Wi-Fi and MQTT Broker**: Configure the Wi-Fi credentials and MQTT broker settings in `main.h`.

2. **Configure Devices**: Define each device's information, including MAC address and MQTT topics.

3. **Compile and Flash**: Use ESP-IDF tools to build and flash the firmware to the ESP32 device.

4. **Monitor Logs**: Use `idf.py monitor` to view logs and verify correct operation.

5. **Test Sensor Triggers**: Activate the PIR sensor or magnetic switch to test event handling and verify data is sent to the MQTT broker.

## Dependencies

- **ESP-IDF Framework**: This project uses the ESP-IDF framework. Make sure to have the correct version installed.
- **Third-Party Libraries**:
  - MQTT client library (`esp-mqtt`)
  - SNTP client for time synchronization

## Build Instructions

1. **Set Up ESP-IDF Environment**: Follow the ESP-IDF setup guide to configure your development environment.

2. **Configure the Project**: Run `idf.py menuconfig` to adjust project-specific settings.

3. **Build the Project**: Execute `idf.py build` to compile the firmware.

4. **Flash the Device**: Connect your ESP32 and run `idf.py flash`.

5. **Monitor Output**: Use `idf.py monitor` to view serial output and logs.