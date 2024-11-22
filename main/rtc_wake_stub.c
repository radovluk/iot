#include <inttypes.h>
#include "esp_sleep.h"
#include "esp_cpu.h"
#include "esp_rom_sys.h"
#include "esp_wake_stub.h"
#include "sdkconfig.h"

#include "esp_rom_sys.h"
#include "hal/rtc_io_ll.h"
#include "rom/rtc.h"
#include "soc/rtc.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"
#include "soc/timer_group_reg.h"
#include "soc/uart_reg.h"
#include "esp_private/esp_clk.h"
#include "main.h"

// Waiting time in seconds for inactive sensors in wake-up stub.
RTC_DATA_ATTR uint32_t SENSOR_INACTIVE_DELAY_IN_WAKE_UP_STUB_SEC = CONFIG_SENSOR_INACTIVE_DELAY_IN_WAKE_UP_STUB_SEC;

// Wake-up cause stored in RTC memory.
static uint32_t wakeup_cause;

// Wake-up time from CPU start to wake stub.
static uint32_t wakeup_time;

// Variable to prevent multiple wake-ups caused by PIR sensors.
RTC_DATA_ATTR static uint64_t last_wakeup_RTC = 0;

// Information about the last battery update.
RTC_DATA_ATTR uint64_t last_battery_info_time_RTC = 0;

/**
 * @brief Wake-up stub function executed during wake-up from deep sleep.
 *
 * This function runs in the minimal RTC fast memory environment and handles
 * minimal tasks required during wake-up, such as determining the wake-up cause,
 * handling sensor triggers, and managing deep sleep cycles.
 *
 * @note Logging and use of standard library functions are limited in this environment.
 */
void wake_stub(void)
{
    ESP_RTC_LOGI("wake stub: start of the wake-up stub");

    // Feed the watchdog to prevent a reset
    REG_WRITE(TIMG_WDTFEED_REG(0), 1);

    // Get wake-up time.
    wakeup_time = esp_cpu_get_cycle_count() / esp_rom_get_cpu_ticks_per_us();

    // Get wake-up cause.
    wakeup_cause = esp_wake_stub_get_wakeup_cause(); // 8: Timer, 2: Sensor (EXT1)

    ESP_RTC_LOGI("wake stub: wake-up cause is %d, wake-up cost %ld us, RTC clock: %llu, last battery update: %llu",
                 wakeup_cause, wakeup_time, my_rtc_time_get_us() / 1000000, last_battery_info_time_RTC);

    // Ensure the wake-up event was caused by a new trigger, not by still active sensors
    // and the triggered events are SENSOR_INACTIVE_DELAY_IN_WAKE_UP_STUB_SEC from each other.
    ESP_RTC_LOGI("wake stub: RTC clock: %llu, last wake-up RTC: %llu",
                 my_rtc_time_get_us() / 1000000, last_wakeup_RTC);

    if (my_rtc_time_get_us() / 1000000 - last_wakeup_RTC <= SENSOR_INACTIVE_DELAY_IN_WAKE_UP_STUB_SEC) {
        last_wakeup_RTC = my_rtc_time_get_us() / 1000000;
        ESP_RTC_LOGI("wake stub: wake-up stub is caused by still active sensor, waiting for the sensors to become inactive");
        ets_delay_us(1000000); // Delay in microseconds.
        esp_wake_stub_set_wakeup_time(AUTOMATIC_WAKEUP_INTERVAL_SEC * 1000000);
        ESP_RTC_LOGI("wake stub: going to deep sleep");
        // Set stub entry, then go to deep sleep again.
        esp_wake_stub_sleep(&wake_stub);
    }
    last_wakeup_RTC = my_rtc_time_get_us() / 1000000;
    ESP_RTC_LOGI("wake stub: Waiting time for inactive sensors is finished.");

    // Wake-up was caused by a sensor.
    if (wakeup_cause == 2) { // ESP_SLEEP_WAKEUP_EXT1
        ESP_RTC_LOGI("wake stub: wake-up caused by sensor trigger.");
        ESP_RTC_LOGI("wake stub: DEVICE_ID = %d", this_device.device_id);
        
        // Read the EXT1 wake-up status register to identify the sensor.
        uint32_t ext1_status = REG_READ(RTC_CNTL_EXT_WAKEUP1_STATUS_REG);

        ESP_RTC_LOGI("wake stub: ext1_status = 0x%X", ext1_status);

        // Identify which sensor triggered the wake-up.
        if (ext1_status == 0x20000) {
            ESP_RTC_LOGI("wake stub: PIR sensor triggered wake-up");
            if (pir_event_count < MAX_PIR_EVENTS){
                store_pir_event();
            } else {
                ESP_RTC_LOGI("wake stub: Can not store the PIR event, the pir_event array is full!");
            }

            if (pir_event_count >= MAX_PIR_EVENTS) {
                // Do not reset pir_event_count here.
                ESP_RTC_LOGI("wake stub: The PIR events array is full (%d/%d events stored), waking up the application.",
                             pir_event_count, MAX_PIR_EVENTS);
                esp_default_wake_deep_sleep();
                ESP_RTC_LOGI("wake stub: Booting the firmware and the main app.");
                return;
            }
        } else {
            ESP_RTC_LOGI("wake stub: Magnetic Switch triggered wake-up.");
            esp_default_wake_deep_sleep();
            ESP_RTC_LOGI("wake stub: Booting the firmware and the main app.");
            return;
        }

        // Perform required minimal actions for the sensors here.
        // Do not proceed to the main application; return to deep sleep.
        ESP_RTC_LOGI("wake stub: returning to deep sleep after handling sensor trigger");

        // Set the wake-up time for the next cycle if needed.
        esp_wake_stub_set_wakeup_time(AUTOMATIC_WAKEUP_INTERVAL_SEC * 1000000);

        // Return to deep sleep.
        esp_wake_stub_sleep(&wake_stub);
    } else {
        ESP_RTC_LOGI("wake stub: wake-up caused by automatic refresh.");
    }

    // Check if it's time to send the battery status to MQTT.
    if (my_rtc_time_get_us() / 1000000 - last_battery_info_time_RTC >= BATTERY_INFO_INTERVAL_SEC) {
        ESP_RTC_LOGI("wake stub: time to send the battery status.");
        last_battery_info_time_RTC = my_rtc_time_get_us() / 1000000;
        esp_default_wake_deep_sleep();
        ESP_RTC_LOGI("wake stub: Booting the firmware and the main app.");
        return;
    }

    // Set wake-up time in stub if needed to check GPIOs or read some sensor periodically in the stub.
    esp_wake_stub_set_wakeup_time(AUTOMATIC_WAKEUP_INTERVAL_SEC * 1000000);

    // Print status.
    ESP_RTC_LOGI("wake stub: going to deep sleep");

    // Set stub entry, then go to deep sleep again.
    esp_wake_stub_sleep(&wake_stub);
}

/**
 * @brief Stores a PIR event with the current timestamp and device information.
 *
 * Calculates the actual timestamp based on RTC time and synchronization data,
 * then stores the event in the PIR events array.
 */
void store_pir_event(void)
{
    // Get the current RTC time in milliseconds.
    uint64_t rtc_time_now = my_rtc_time_get_us() / 1000;

    // Calculate the time difference since the last synchronization.
    uint64_t rtc_time_diff = rtc_time_now - rtc_time_at_last_sync;

    // Calculate the actual timestamp using the time difference and the last actual time.
    uint64_t actual_timestamp = actual_time_at_last_sync + rtc_time_diff;

    ESP_RTC_LOGI("wake stub: rtc_time_now: %llu, rtc_time_at_last_sync: %llu, actual_time_at_last_sync: %llu, actual_timestamp: %llu",
                 rtc_time_now, rtc_time_at_last_sync, actual_time_at_last_sync, actual_timestamp);

    // Store the new event with the calculated actual timestamp.
    pir_events[pir_event_count].timestamp = actual_timestamp;

    // Since we cannot use memcpy in the wake-up stub, we manually copy each field.
    pir_events[pir_event_count].device.device_id = this_device.device_id;
    pir_events[pir_event_count].device.battery_info_available = this_device.battery_info_available;

    // Note: Assigning pointers directly as below is acceptable in the wake-up stub environment.
    pir_events[pir_event_count].device.device_name = this_device.device_name;
    pir_events[pir_event_count].device.device_topic = this_device.device_topic;
    pir_events[pir_event_count].device.device_key = this_device.device_key;
    pir_events[pir_event_count].device.room_id = this_device.room_id;

    // Copy MAC address manually.
    for (int i = 0; i < 6; i++) {
        pir_events[pir_event_count].device.mac_address[i] = this_device.mac_address[i];
    }

    ESP_RTC_LOGI("wake stub: Stored PIR event: Timestamp = %llu, Device ID = %d, Event Index = %d",
                 actual_timestamp, this_device.device_id, pir_event_count);

    // Increment the event count.
    pir_event_count++;
}

/**
 * @brief Retrieves the current RTC time in microseconds.
 *
 * This function reads the RTC time registers to obtain the current time.
 * It operates in the wake-up stub environment and uses low-level register access.
 *
 * @return Current RTC time in microseconds.
 */
RTC_IRAM_ATTR uint64_t my_rtc_time_get_us(void)
{
    SET_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_UPDATE);
    while (GET_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_VALID) == 0) {
        ets_delay_us(1); // Wait for RTC time to be valid.
    }
    SET_PERI_REG_MASK(RTC_CNTL_INT_CLR_REG, RTC_CNTL_TIME_VALID_INT_CLR);
    uint64_t t = READ_PERI_REG(RTC_CNTL_TIME0_REG);
    t |= ((uint64_t)READ_PERI_REG(RTC_CNTL_TIME1_REG)) << 32;

    uint32_t period = REG_READ(RTC_SLOW_CLK_CAL_REG);

    // Convert RTC clock cycles to microseconds.
    uint64_t now_us = ((t * period) >> RTC_CLK_CAL_FRACT);

    return now_us;
}
