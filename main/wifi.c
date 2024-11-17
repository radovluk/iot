#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi.h"

#include "esp_log.h"
#include "esp_system.h"
#include "main.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < 10) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI("wifi", "retry to connect to the AP");
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI("wifi", "connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    ESP_LOGI("wifi", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}


void finish_wifi(void){
  esp_wifi_disconnect();
}

void start_wifi(void) {
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = EXAMPLE_ESP_WIFI_SSID,
              .password = EXAMPLE_ESP_WIFI_PASS,
              .threshold.authmode = WIFI_AUTH_WPA2_PSK,
              .pmf_cfg = {.capable = true, .required = false},
          },
  };
  ESP_LOGI("connecting to:", EXAMPLE_ESP_WIFI_SSID);
  ESP_LOGI("wifi", EXAMPLE_ESP_WIFI_SSID);
  ESP_LOGI("wifi", EXAMPLE_ESP_WIFI_PASS);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI("wifi", "wifi_init_sta finished.");

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
   * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
   * happened. */
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI("wifi", "connected");

     // Print IP address
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"); // Get default STA network interface
    esp_netif_get_ip_info(netif, &ip_info);
    ESP_LOGI("wifi", "IP Address: " IPSTR, IP2STR(&ip_info.ip));
    // End of the modified part
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI("wifi", "Failed to connect");
    esp_restart();
  } else {
    ESP_LOGE("wifi", "UNEXPECTED EVENT");
    esp_restart();
  }

  ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
  ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
  vEventGroupDelete(s_wifi_event_group);
}