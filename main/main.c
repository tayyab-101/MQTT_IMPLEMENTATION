#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_driver.h"
#include "mqtt_driver.h"
#include "json_parser.h"
#include "esp_mac.h"
uint8_t mac[6];

static const char *TAG = "MAIN";

static void mqtt_processor_task(void *pvParameters) {
    char payload_buf[256];
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        int len = mqtt_get_latest_payload(payload_buf, sizeof(payload_buf));
        if (len > 0) {
            ESP_LOGI(TAG, "MQTT payload Before Json Parser: %s", payload_buf);
            // mqtt_json_data_t data;
            if (json_parser_parse_payload(payload_buf, &data)) {
                ESP_LOGI(TAG, "After JSON Parser: status=%s temperature=%.2f counter=%" PRIu32,
                         data.status, data.temperature, data.counter);
            } else {
                ESP_LOGE(TAG, "Received payload is not valid expected JSON: %s", payload_buf);
            }
        }
    }
}


/* --- Main App --- */
void app_main(void) {
    // 1. Initialize Non-Volatile Storage (required by Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Connect to Wi-Fi
    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
    wifi_init_sta();

    // Create the task before MQTT starts so it cannot miss an early notification.
    TaskHandle_t processor_task_handle = NULL;
    if (xTaskCreate(mqtt_processor_task, "mqtt_processor_task", 4096, NULL, 5,
                    &processor_task_handle) != pdPASS) 
    {
        ESP_LOGE(TAG, "Failed to create MQTT processor task");
        return;
    }
    mqtt_set_notification_task(processor_task_handle);

    ESP_LOGI(TAG, "Start MQTT...");
    mqtt_app_start();
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay one second
    mqtt_publish("esp32/test1", "Hello from ESP32!", 1, 0);
    // int message_id = mqtt_publish("esp32/test1", "Hello from ESP32!", 1, 0);
    // if (message_id < 0) {
    //     ESP_LOGE(TAG, "Could not queue MQTT message");
    // } else {
    //     ESP_LOGI(TAG, "Queued MQTT message, ID: %d", message_id);
    // }
    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));
    ESP_LOGI(TAG, "Wi-Fi MAC: %02X %02X %02X %02X %02X %02X",
         (unsigned)mac[0], (unsigned)mac[1], (unsigned)mac[2],
         (unsigned)mac[3], (unsigned)mac[4], (unsigned)mac[5]);

    unsigned int counter = 0;
    char message1[128];
    char message2[128];
    char message3[128];
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000)); // Delay 10 seconds
        snprintf(message1, sizeof(message1),
                 "{\"status\":\"ON\",\"temperature\":25.4,\"counter\":%u}",
                 ++counter);
        snprintf(message2, sizeof(message2),
                 "{\"status\":\"ON\",\"temperature\":25.4,\"counter\":%u}",
                 ++counter);
        snprintf(message3, sizeof(message3),
                 "{\"status\":\"ON\",\"temperature\":25.4,\"counter\":%u}",
                 ++counter);
        
        ESP_LOGI(TAG, "MQTT_TOPIC_PUB1: %s", MQTT_TOPIC_PUB1);
        ESP_LOGI(TAG, "MQTT_TOPIC_SUB1: %s", MQTT_TOPIC_SUB1);
        ESP_LOGI(TAG, "MQTT_TOPIC_PUB2: %s", MQTT_TOPIC_PUB2);
        ESP_LOGI(TAG, "MQTT_TOPIC_PUB3: %s", MQTT_TOPIC_PUB3);
        
        mqtt_publish(MQTT_TOPIC_PUB1, message1, 1, 0);
        mqtt_publish(MQTT_TOPIC_PUB2, message2, 1, 0);
        mqtt_publish(MQTT_TOPIC_PUB3, message3, 1, 0);
    }
}
