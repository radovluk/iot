idf_component_register(SRCS main.c wifi.c sntp.c mqtt.c gauge.c rtc_wake_stub.c
					EMBED_TXTFILES 
                    INCLUDE_DIRS "."
                    REQUIRES esp_event esp_wifi mqtt nvs_flash driver lc709203f) 