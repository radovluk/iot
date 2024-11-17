// Standard Libraries
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// ESP-IDF Core Components
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

// FreeRTOS Headers
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

// LWIP Libraries
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

// Project-Specific Headers
#include "mqtt_client.h"
#include "main.h"
#include "gauge.h"



esp_mqtt_client_handle_t mqtt_client = NULL;
EventGroupHandle_t mqtt_event_group;
static int qos_test = 1;

const static int CONNECTED_BIT = BIT0;

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  esp_mqtt_event_t *data = (esp_mqtt_event_t *)event_data;
  switch (event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI("mqtt", "MQTT_EVENT_CONNECTED\n");
      xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI("mqtt", "MQTT_EVENT_DISCONNECTED\n");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI("mqtt", "MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", data->msg_id);
      break;

    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI("mqtt", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", data->msg_id);
      break;

    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI("mqtt", "MQTT_EVENT_PUBLISHED, msg_id=%d\n", data->msg_id);
      break;

    case MQTT_EVENT_DATA:
      ESP_LOGI("mqtt", "MQTT_EVENT_DATA\n");
      ESP_LOGI("mqtt", "TOPIC=%.*s\r\n", data->topic_len, data->topic);
      ESP_LOGI("mqtt", "DATA=%.*s\r\n", data->data_len, data->data);
      ESP_LOGI("mqtt", "ID=%d, total_len=%d, data_len=%d, current_data_offset=%d\n", data->msg_id, data->total_data_len, data->data_len, data->current_data_offset);
      // count++;
      break;

    case MQTT_EVENT_ERROR:
      ESP_LOGI("mqtt", "MQTT_EVENT_ERROR\n");
      break;

    case MQTT_EVENT_BEFORE_CONNECT:
      ESP_LOGI("mqtt", "MQTT_EVENT_BEFORE_CONNECT\n");
      break;

    default:
      ESP_LOGI("mqtt", "Other event id:%d\n", data->event_id);
      break;
  }
}

void start_mqtt(void) {
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.broker.address.hostname = MQTT_BROKER;
  mqtt_cfg.broker.address.port = 1883;
  mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
  mqtt_cfg.session.protocol_ver = MQTT_PROTOCOL_V_3_1_1;
  mqtt_cfg.credentials.username = "JWT";
  mqtt_cfg.network.timeout_ms = 30000;
  mqtt_cfg.credentials.authentication.password = DEVICE_KEY;
  ESP_LOGI("mqtt", "Device Key: %s", DEVICE_KEY);

  mqtt_cfg.credentials.authentication.password = DEVICE_KEY;

  ESP_LOGI("mqtt", "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);

  mqtt_event_group = xEventGroupCreate();
  esp_mqtt_client_start(mqtt_client);
  ESP_LOGI("mqtt", "Note free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI("mqtt", "Waiting for connection to MQTT\n");
  xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
  ESP_LOGI("mqtt", "Connected to MQTT\n");
}

void sendPIReventToMQTT(void) {
  time_t now = 0;

  char msg[150];
  time(&now);

  // Define room_id based on device location
  const char* room_id;
  // Assign the room_id based on DEVICE_ID
  if (strcmp(DEVICE_ID, "4") == 0) {
      room_id = "livingroombedarea";
  } else if (strcmp(DEVICE_ID, "5") == 0) {
      room_id = "kitchen";
  } else if (strcmp(DEVICE_ID, "3") == 0) {
      room_id = "bathroom";
  } else {
      room_id = "unknown";  // Default or fallback room ID
  }

  // Format the message with the correct room ID
  int size = snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"PIR\",\"values\":[{\"timestamp\":%llu, \"roomID\":\"%s\"}]}]}", 
                      (unsigned long long)(now * 1000), room_id);

  ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, DEVICE_TOPIC);
  auto err = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msg, size, 1, 0);
  if (err == -1) {
    printf("Error while publishing to mqtt\n");
    ESP_LOGI("functions", "SendToMqttFunction terminated");
    return ESP_FAIL;
  }
}

void sendMagneticSwitchEventToMQTT(void) {
    time_t now = 0;
    char msg[150];
    time(&now);

    int size = snprintf(msg, sizeof(msg), 
        "{\"sensors\":[{\"name\":\"MagneticSwitch\",\"values\":[{\"timestamp\":%llu, \"roomID\":\"livingroomdoor\"}]}]}",
        (unsigned long long)(now * 1000));
    ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, DEVICE_TOPIC);

    int msg_id = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msg, size, 1, 0);
    if (msg_id == -1) {
        ESP_LOGE("mqtt", "Error publishing magnetic switch event");
    }
}


void sendBatteryStatusToMQTT(void) {
  time_t now = 0;

  char msg[150];
  time(&now);

  int size = snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"battery\",\"values\":[{\"timestamp\":%llu, \"voltage\":%.1f, \"soc\":%.1f}]}]}", now * 1000, voltage, rsoc);
  ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, DEVICE_TOPIC);
  auto err = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msg, size, 1, 0);
  if (err == -1) {
    printf("Error while publishing to mqtt\n");
    ESP_LOGI("functions", "SendToMqttFunction terminated");
    return ESP_FAIL;
  }
}
