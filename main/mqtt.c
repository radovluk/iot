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

/**
 * @brief MQTT event handler callback function.
 *
 * This function handles various MQTT events such as connection, disconnection,
 * subscription, data reception, etc. It is registered with the MQTT client to
 * receive event notifications and acts accordingly based on the event type.
 *
 * @param handler_args  User-defined argument provided during registration (unused).
 * @param base          Event base (should be MQTT_EVENT).
 * @param event_id      ID of the MQTT event.
 * @param event_data    Data associated with the event.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  esp_mqtt_event_t *data = (esp_mqtt_event_t *)event_data;
  switch (event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI("mqtt", "MQTT_EVENT_CONNECTED\n");
      xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
      mqtt_broker_connected = true;
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI("mqtt", "MQTT_EVENT_DISCONNECTED\n");
      mqtt_broker_connected = false;
      break;

    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI("mqtt", "MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", data->msg_id);
      mqtt_broker_connected = false;
      break;

    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI("mqtt", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", data->msg_id);
      mqtt_broker_connected = false;
      break;

    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI("mqtt", "MQTT_EVENT_PUBLISHED, msg_id=%d\n", data->msg_id);
      mqtt_broker_connected = false;
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

/**
 * @brief Initializes and starts the MQTT client.
 *
 * This function configures the MQTT client with the necessary parameters,
 * initializes it, registers the event handler, and starts the client.
 * It also waits for the client to connect before returning, ensuring that
 * MQTT communication is ready for subsequent operations.
 *
 * - Configures MQTT broker settings such as hostname, port, and transport.
 * - Sets up authentication using device-specific credentials.
 * - Registers the event handler for MQTT events.
 * - Starts the MQTT client and waits for a successful connection.
 */
void start_mqtt(void) {
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.broker.address.hostname = MQTT_BROKER;
  mqtt_cfg.broker.address.port = 1883;
  mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
  mqtt_cfg.session.protocol_ver = MQTT_PROTOCOL_V_3_1_1;
  mqtt_cfg.credentials.username = "JWT";
  mqtt_cfg.network.timeout_ms = 30000;
  mqtt_cfg.credentials.authentication.password = this_device.device_key;
  ESP_LOGI("mqtt", "Device Key: %s", this_device.device_key);

  ESP_LOGI("mqtt", "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);

  mqtt_event_group = xEventGroupCreate();
  esp_mqtt_client_start(mqtt_client);
  ESP_LOGI("mqtt", "Note free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI("mqtt", "Waiting for connection to MQTT\n");

  // Wait for connection with a timeout of 10 seconds
  EventBits_t bits = xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, pdMS_TO_TICKS(10000));
  if (bits & CONNECTED_BIT) {
    ESP_LOGI("mqtt", "Connected to MQTT\n");
  } else {
    ESP_LOGI("mqtt", "Could not connect to MQTT broker\n");
    mqtt_broker_connected = false;
  }
}

/**
 * @brief Sends a single PIR (Passive Infrared) event to the MQTT broker.
 *
 * This function constructs a JSON message containing the current timestamp
 * and room ID associated with the device. It then publishes this message
 * to the MQTT broker under the device's specific topic.
 *
 * - Retrieves the current time to timestamp the event.
 * - Formats the message as a JSON string.
 * - Publishes the message to the MQTT broker with QoS level 1.
 *
 * @note This function should be called when a PIR event is detected and needs
 *       to be reported immediately.
 */
void sendPIReventToMQTT(void) {
  time_t now = 0;
  char msg[150];
  time(&now);
  const char* room_id = this_device.room_id;

  if (mqtt_broker_connected) {
    // Format the message with the correct room ID
    int size = snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"PIR\",\"values\":[{\"timestamp\":%llu, \"roomID\":\"%s\"}]}]}",
                        (unsigned long long)(now * 1000), room_id);

    ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, this_device.device_topic);
    int err = esp_mqtt_client_publish(mqtt_client, this_device.device_topic, msg, size, 1, 0);
    if (err == -1) {
      ESP_LOGE("mqtt", "Error while publishing to mqtt");
      ESP_LOGI("functions", "SendToMqttFunction terminated");
    }
  } else {
    // Store the event in pir_events[]
    if (pir_event_count < MAX_PIR_EVENTS) {
      pir_events[pir_event_count].timestamp = (unsigned long long)(now * 1000);
      // Copy device info
      pir_events[pir_event_count].device = this_device;
      pir_event_count++;
      ESP_LOGI("PIR", "Stored PIR event locally, total stored events: %d", pir_event_count);
    } else {
      ESP_LOGW("PIR", "PIR event buffer is full, cannot store more events");
    }
  }
}


/**
 * @brief Sends a magnetic switch event to the MQTT broker.
 *
 * This function constructs a JSON message containing the current timestamp
 * and a predefined room ID ("livingroomdoor"), indicating that a magnetic
 * switch event (e.g., door open/close) has occurred. It then publishes this
 * message to the MQTT broker under the device's topic.
 *
 * - Retrieves the current time to timestamp the event.
 * - Formats the message as a JSON string.
 * - Publishes the message to the MQTT broker with QoS level 1.
 *
 * @note This function is specific to devices equipped with a magnetic switch sensor.
 */
void sendMagneticSwitchEventToMQTT(void) {
    if (!mqtt_broker_connected) {
      ESP_LOGI("PIR", "Cannot send stored PIR events, MQTT is not connected");
      return;
    }
    
    time_t now = 0;
    char msg[150];
    time(&now);

    int size = snprintf(msg, sizeof(msg), 
        "{\"sensors\":[{\"name\":\"MagneticSwitch\",\"values\":[{\"timestamp\":%llu, \"roomID\":\"livingroomdoor\"}]}]}",
        (unsigned long long)(now * 1000));
    ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, this_device.device_topic);

    int msg_id = esp_mqtt_client_publish(mqtt_client, this_device.device_topic, msg, size, 1, 0);
    if (msg_id == -1) {
        ESP_LOGE("mqtt", "Error publishing magnetic switch event");
    }
}

/**
 * @brief Sends the battery status information to the MQTT broker.
 *
 * This function constructs a JSON message containing the current timestamp,
 * battery voltage, and state of charge (SoC) of the device. It then publishes
 * this message to the MQTT broker under the device's topic.
 *
 * - Retrieves the current time to timestamp the status.
 * - Formats the message as a JSON string with battery metrics.
 * - Publishes the message to the MQTT broker with QoS level 1.
 *
 * @note The variables `voltage` and `rsoc` should be updated with the latest
 *       battery measurements before calling this function.
 */
void sendBatteryStatusToMQTT(void) {
  if (!mqtt_broker_connected) {
      ESP_LOGI("PIR", "Cannot send stored PIR events, MQTT is not connected");
      return;
  }

  time_t now = 0;

  char msg[150];
  time(&now);

  int size = snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"battery\",\"values\":[{\"timestamp\":%llu, \"voltage\":%.1f, \"soc\":%.1f}]}]}", now * 1000, voltage, rsoc);
  ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, this_device.device_topic);
  auto err = esp_mqtt_client_publish(mqtt_client, this_device.device_topic, msg, size, 1, 0);
  if (err == -1) {
    printf("Error while publishing to mqtt\n");
    ESP_LOGI("functions", "SendToMqttFunction terminated");
    return ESP_FAIL;
  }
}

/**
 * @brief Sends stored PIR events to the MQTT broker in batch.
 *
 * This function iterates over the array of stored PIR events (`pir_events`),
 * constructs a JSON message containing the timestamps and room IDs of all events,
 * and publishes it to the MQTT broker under the device's topic. After successful
 * publication, it resets the event count to prevent re-sending the same events.
 *
 * - Builds a JSON array of PIR event values.
 * - Formats the message as a JSON string with all stored events.
 * - Publishes the message to the MQTT broker with QoS level 1.
 *
 * @note This function is useful for sending multiple events that were stored during
 *       periods when the device was in deep sleep or had no network connectivity.
 */
void sendPIReventsToMQTT()
{
    if (!mqtt_broker_connected) {
      ESP_LOGI("PIR", "Cannot send stored PIR events, MQTT is not connected");
      return;
    }

    // Build the JSON message
    char msg[1024]; // Adjust size as needed
    char values[512] = ""; // Buffer for "values" array
    char temp[128];

    for (int i = 0; i < pir_event_count; i++)
    {
        // Get room ID from the stored device information
        const char* room_id = pir_events[i].device.room_id;

        // Append comma if not the first element
        if (i > 0)
            strcat(values, ",");

        // Format the timestamp in milliseconds
        uint64_t timestamp_ms = pir_events[i].timestamp;

        snprintf(temp, sizeof(temp), "{\"timestamp\":%llu,\"roomID\":\"%s\"}", timestamp_ms, room_id);
        strcat(values, temp);
    }

    // Build the final JSON message
    snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"PIR\",\"values\":[%s]}]}", values);

    // Send the message via MQTT
    ESP_LOGI("mqtt", "Sending PIR events: %s", msg);
    int msg_id = esp_mqtt_client_publish(mqtt_client, this_device.device_topic, msg, 0, 1, 0);
    if (msg_id == -1)
    {
        ESP_LOGE("mqtt", "Error publishing PIR events to MQTT");
        ESP_LOGI("functions", "SendToMqttFunction terminated");
        return ESP_FAIL;
    } else {
        // Reset the PIR event count if success
        pir_event_count = 0;
    }
}
