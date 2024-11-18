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

// Waiting time in seconds for inactive sensors in wake up stub.
RTC_DATA_ATTR static const uint32_t waiting_time_for_inactive_sensors_s = 3;

// wakeup_cause stored in RTC memory
static uint32_t wakeup_cause;

// wakeup_time from CPU start to wake stub
static uint32_t wakeup_time;

// Variable to prevent the multiple wakeups cased by PIR sensors
RTC_DATA_ATTR static uint64_t last_wakeup_RTC = 0;

// Counter of the already stored PIR events
RTC_DATA_ATTR int pir_event_count = 0;

// Information about the last battery update, needs to extern
RTC_DATA_ATTR uint64_t last_battery_info_time_RTC = 0;

// wake up stub function stored in RTC memory
void wake_stub(void)
{
    ESP_RTC_LOGI("wake stub: start of the wake up stub");

    // Get wakeup time.
    wakeup_time = esp_cpu_get_cycle_count() / esp_rom_get_cpu_ticks_per_us();
    // Get wakeup cause.
    wakeup_cause = esp_wake_stub_get_wakeup_cause(); // 8: Timer, 2: Sensor (EXT1)
    ESP_RTC_LOGI("wake stub: wakeup cause is %d, wakeup cost %ld us, RTC clock: %llu, last battery update: %llu", 
                wakeup_cause, wakeup_time, my_rtc_time_get_us()/1000000, last_battery_info_time_RTC);

    // Make sure that the wake up event was caused by a new trigger and not the still active sensors
    ESP_RTC_LOGI("wake up stub: RTC clock: %llu, last wake up RTC: %llu", my_rtc_time_get_us()/1000000, last_wakeup_RTC);
    if (my_rtc_time_get_us()/1000000 - last_wakeup_RTC <= waiting_time_for_inactive_sensors_s){
        last_wakeup_RTC = my_rtc_time_get_us()/1000000;
        ESP_RTC_LOGI("wake stub: wake up stub is caused by still active sensor, waiting for the sensors to become inactive");
        ets_delay_us(1000000); // in microseconds
        esp_wake_stub_set_wakeup_time(AUTOMATIC_WAKEUP_INTERVAL_SEC * 1000000);
        ESP_RTC_LOGI("wake stub: going to deep sleep");
        // Set stub entry, then going to deep sleep again.
        esp_wake_stub_sleep(&wake_stub);
    }
    last_wakeup_RTC = my_rtc_time_get_us()/1000000;
    ESP_RTC_LOGI("wake stub: Wating time for inactive sensors is finished.");

    // Wake up was caused by sensor
    if (wakeup_cause == 2) {
        ESP_RTC_LOGI("wake stub: wakeup caused by sensor trigger.");
        ESP_RTC_LOGI("wake stub: DEVICE_ID = %d, DEVICE_TOPIC = %s", DEVICE_ID, DEVICE_TOPIC);

        #define DR_REG_RTCCNTL_BASE 0x3ff48000
        #define RTC_CNTL_EXT_WAKEUP1_STATUS_REG (DR_REG_RTCCNTL_BASE + 0x90)
        #define REG_READ(_r) (*(volatile uint32_t *)(_r))

        uint32_t ext1_status = REG_READ(RTC_CNTL_EXT_WAKEUP1_STATUS_REG);
        ESP_RTC_LOGI("wake stub: ext1_status = 0x%X", ext1_status);

        // Identify which sensor triggered the wakeup
        if (ext1_status == 0x149970) {
            ESP_RTC_LOGI("wake stub: PIR sensor triggered wakeup\n");
            store_pir_event(my_rtc_time_get_us(), DEVICE_ID);
            if (pir_event_count == MAX_PIR_EVENTS){
                pir_event_count = 0;
                ESP_RTC_LOGI("wake stub: The pir events array is full, waking up the application.\n");
                esp_default_wake_deep_sleep();
                ESP_RTC_LOGI("wake stub: Booting the firmware and the main app.") 
                return;
            }
        } else {
            ESP_RTC_LOGI("wake stub: Magnetic Switch triggered wakeup.\n");
            esp_default_wake_deep_sleep();
            ESP_RTC_LOGI("wake stub: Booting the firmware and the main app.") 
            return;
        }

        // Perform required minimal actions for the sensors here (e.g., logging or handling GPIOs).
        // Do not proceed to the main application; return to deep sleep.
        ESP_RTC_LOGI("wake stub: returning to deep sleep after handling sensor trigger\n");

        // Set the wakeup time for the next cycle if needed
        esp_wake_stub_set_wakeup_time(AUTOMATIC_WAKEUP_INTERVAL_SEC * 1000000);

        // Return to deep sleep
        esp_wake_stub_sleep(&wake_stub);
    } else {
        ESP_RTC_LOGI("wake stub: wakeup caused by automatic refresh.");
    }

    // Time to send the battery status to the MQTT
    if (my_rtc_time_get_us()/1000000 - last_battery_info_time_RTC >= BATTERY_INFO_INTERVAL_SEC){
        ESP_RTC_LOGI("wake stub: time to send the battery status.\n");
        last_battery_info_time_RTC = my_rtc_time_get_us()/1000000;
        esp_default_wake_deep_sleep();
        ESP_RTC_LOGI("wake stub: Booting the firmware and the main app.") 
        return;
    }

    // Set wakeup time in stub, if need to check GPIOs or read some sensor periodically in the stub.
    esp_wake_stub_set_wakeup_time(AUTOMATIC_WAKEUP_INTERVAL_SEC * 1000000);
    // Print status.
    ESP_RTC_LOGI("wake stub: going to deep sleep");
    // Set stub entry, then going to deep sleep again.
    esp_wake_stub_sleep(&wake_stub);
}

void store_pir_event(uint64_t timestamp, int device_id)
{
    // Store the new event
    pir_events[pir_event_count].timestamp = timestamp;
    pir_events[pir_event_count].device_id = device_id;

    // Log the stored event message
    ESP_RTC_LOGI("wake stub: stored PIR event: Timestamp = %llu, Device ID = %d, Event Index = %d\n",
    (unsigned long long)timestamp, device_id, pir_event_count);

    // Increment the event count
    pir_event_count++;
}

// my_rtc_time_get_us() returns the rtc clock value in the wakeup stub
RTC_IRAM_ATTR uint64_t my_rtc_time_get_us(void) {
  SET_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_UPDATE);
  while (GET_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_VALID) == 0) {
    ets_delay_us(1);  // might take 1 RTC slowclk period, don't flood RTC bus
  }
  SET_PERI_REG_MASK(RTC_CNTL_INT_CLR_REG, RTC_CNTL_TIME_VALID_INT_CLR);
  uint64_t t = READ_PERI_REG(RTC_CNTL_TIME0_REG);
  t |= ((uint64_t)READ_PERI_REG(RTC_CNTL_TIME1_REG)) << 32;

  uint32_t period = REG_READ(RTC_SLOW_CLK_CAL_REG);

  // Convert microseconds to RTC clock cycles
  uint64_t now1 = ((t * period) >> RTC_CLK_CAL_FRACT);

  return now1;
}