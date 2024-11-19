#pragma once
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"

void start_mqtt(void);
void sendPIReventToMQTT(void);
void sendBatteryStatusToMQTT(void);
void sendMagneticSwitchEventToMQTT(void);
void sendPIReventsToMQTT(void);
