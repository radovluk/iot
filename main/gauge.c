#include "gauge.h"

#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lc709203f.h"
#include "main.h"

i2c_dev_t lc = {};
float voltage = 0, rsoc = 0;

static esp_err_t initialize_lc709203f() {
  ESP_ERROR_CHECK(lc709203f_init_desc(&lc, 0, 21, 22));

  int count = 0;
  while (count < 10 && lc709203f_set_power_mode(&lc, LC709203F_POWER_MODE_OPERATIONAL) != ESP_OK) {
    vTaskDelay(pdMS_TO_TICKS(100));
    count++;
  }
  // Using 2500mAh LiPo battery. Check Datasheet graph for APA values by battery type & mAh
  ESP_ERROR_CHECK(lc709203f_set_apa(&lc, 0x2A));
  ESP_ERROR_CHECK(lc709203f_set_battery_profile(&lc, LC709203F_BATTERY_PROFILE_1));
  ESP_ERROR_CHECK(lc709203f_initial_rsoc(&lc));
  ESP_ERROR_CHECK(lc709203f_set_temp_mode(&lc, LC709203F_TEMP_MODE_I2C));
  ESP_ERROR_CHECK(lc709203f_set_cell_temperature_celsius(&lc, 20));  

  uint16_t value = 0;
  ESP_ERROR_CHECK(lc709203f_get_power_mode(&lc, (lc709203f_power_mode_t *)&value));
  ESP_LOGI("gauge", "Power Mode: 0x%X", value);
  ESP_ERROR_CHECK(lc709203f_get_apa(&lc, (uint8_t *)&value));
  ESP_LOGI("gauge", "APA: 0x%X", value);
  ESP_ERROR_CHECK(lc709203f_get_battery_profile(&lc, (lc709203f_battery_profile_t *)&value));
  ESP_LOGI("gauge", "Battery Profile: 0x%X", value);
  ESP_ERROR_CHECK(lc709203f_get_temp_mode(&lc, (lc709203f_temp_mode_t *)&value));
  ESP_LOGI("gauge", "Temp Mode: 0x%X", value);

  return ESP_OK;
}

void getRSOC() {
  uint16_t voltage_u = 0, rsoc_u = 0;
  ESP_ERROR_CHECK(i2cdev_init());
  initialize_lc709203f();

  int count = 0;
  while (count < 10 && lc709203f_get_cell_voltage(&lc, &voltage_u) != ESP_OK) {
    vTaskDelay(pdMS_TO_TICKS(100));
    count++;
  }
  ESP_ERROR_CHECK(lc709203f_get_rsoc(&lc, &rsoc_u));
  voltage =  voltage_u / 1000.0;
  rsoc = (float) rsoc_u;
  ESP_LOGI("gauge", "Voltage: %.2f\tRSOC: %.1f%%", voltage, rsoc);
  // ESP_LOGI("gauge", "Voltage: %.2f\tRSOC: %.u%%", voltage, rsoc_u);
  ESP_ERROR_CHECK(lc709203f_set_power_mode(&lc, LC709203F_POWER_MODE_SLEEP));
}