idf_component_register(
    SRCS "main.c"
         "mqtt_driver.c"
         "json_parser.c"
         "wifi_driver.c"
    PRIV_REQUIRES mqtt json nvs_flash esp_netif esp_wifi esp_event
    INCLUDE_DIRS "."
)