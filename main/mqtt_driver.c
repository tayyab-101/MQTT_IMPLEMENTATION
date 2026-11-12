#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt_driver.h"
#include "esp_mac.h"

// ================= CHANGE THESE =================
// #define MQTT_BROKER_URI "mqtt://broker.hivemq.com:1883" // Free public broker
// ================================================

static const char *TAG = "MQTT_DRIVER";

#define MQTT_PAYLOAD_BUFFER_SIZE 256

static char latest_payload[MQTT_PAYLOAD_BUFFER_SIZE];
static size_t latest_payload_length;
static TaskHandle_t notification_task;
static esp_mqtt_client_handle_t mqtt_client;
static portMUX_TYPE payload_mux = portMUX_INITIALIZER_UNLOCKED;
static const uint32_t serial_number_indoor = 02356;
static const uint32_t serial_number_outdoor1 = 12345;
static const uint32_t serial_number_outdoor2 = 67890;

char MQTT_TOPIC_SUB1[MQTT_TOPIC_MAX_LEN];
char MQTT_TOPIC_SUB2[MQTT_TOPIC_MAX_LEN];
char MQTT_TOPIC_SUB3[MQTT_TOPIC_MAX_LEN];
char MQTT_TOPIC_PUB1[MQTT_TOPIC_MAX_LEN];
char MQTT_TOPIC_PUB2[MQTT_TOPIC_MAX_LEN];
char MQTT_TOPIC_PUB3[MQTT_TOPIC_MAX_LEN];



void dynamic_topics_with_mac(uint32_t SerialNumber_indoor, uint32_t SerialNumber_outdoor1, uint32_t SerialNumber_outdoor2) {
    
    uint8_t mac[6];
    char mac_string[13]; // 12 hex digits + null terminator
    // esp_efuse_mac_get_default(mac);
    // ESP_LOGI(TAG, "Device MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
    //          (unsigned)mac[0], (unsigned)mac[1], (unsigned)mac[2],
    //          (unsigned)mac[3], (unsigned)mac[4], (unsigned)mac[5]);
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "Device MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
             (unsigned)mac[0], (unsigned)mac[1], (unsigned)mac[2],
             (unsigned)mac[3], (unsigned)mac[4], (unsigned)mac[5]);
    snprintf(mac_string, sizeof(mac_string),
                              "%02X%02X%02X%02X%02X%02X",
                              (unsigned)mac[0], (unsigned)mac[1],
                              (unsigned)mac[2], (unsigned)mac[3],
                              (unsigned)mac[4], (unsigned)mac[5]);
    ESP_LOGI(TAG, "MAC String for Topics: %s", mac_string);
    // Create dynamic topics based on MAC address
    snprintf(MQTT_TOPIC_SUB1, sizeof(MQTT_TOPIC_SUB1),
             "roofsafe/IDU-DEMO-%s/heartbeat", mac_string);
    ESP_LOGI(TAG, "MQTT_TOPIC_SUB1: %s", MQTT_TOPIC_SUB1);
    snprintf(MQTT_TOPIC_SUB2, sizeof(MQTT_TOPIC_SUB2),
             "roofsafe/IDU-DEMO-%s/ODU-%" PRIu32 "/telemetry", mac_string, SerialNumber_outdoor1);
    snprintf(MQTT_TOPIC_SUB3, sizeof(MQTT_TOPIC_SUB3),
             "roofsafe/IDU-DEMO-%s/ODU-%" PRIu32 "/telemetry", mac_string, SerialNumber_outdoor2);

    snprintf(MQTT_TOPIC_PUB1, sizeof(MQTT_TOPIC_PUB1),
             "roofsafe/IDU-DEMO-%s/heartbeat", mac_string);
    ESP_LOGI(TAG, "MQTT_TOPIC_PUB1: %s", MQTT_TOPIC_PUB1);
    snprintf(MQTT_TOPIC_PUB2, sizeof(MQTT_TOPIC_PUB2),
             "roofsafe/IDU-DEMO-%s/ODU-%" PRIu32 "/telemetry", mac_string, SerialNumber_outdoor1);
    ESP_LOGI(TAG, "MQTT_TOPIC_PUB2: %s", MQTT_TOPIC_PUB2);
    snprintf(MQTT_TOPIC_PUB3, sizeof(MQTT_TOPIC_PUB3),
             "roofsafe/IDU-DEMO-%s/ODU-%" PRIu32 "/telemetry", mac_string, SerialNumber_outdoor2);
    ESP_LOGI(TAG, "MQTT_TOPIC_PUB3: %s", MQTT_TOPIC_PUB3);
}






/* --- MQTT Event Handler --- */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Connected to Broker!");
        // 1. Subscribe to a topic
        esp_mqtt_client_subscribe(client,MQTT_TOPIC_SUB1, 0);
        // esp_mqtt_client_subscribe(client,MQTT_TOPIC_SUB2, 0);
        // esp_mqtt_client_subscribe(client,MQTT_TOPIC_SUB3, 0);
        break;

    case MQTT_EVENT_DATA: {
        ESP_LOGI(TAG, "Message Received!");
        // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        // printf("DATA=%.*s\r\n", event->data_len, event->data);

        size_t payload_length = event->data_len > 0 ? (size_t)event->data_len : 0;
        if (payload_length > 0 && event->data == NULL) {
            ESP_LOGE(TAG, "Received MQTT payload has no data");
            break;
        }
        if (payload_length >= sizeof(latest_payload)) {
            payload_length = sizeof(latest_payload) - 1;
        }

        portENTER_CRITICAL(&payload_mux);
        if (payload_length > 0) {
            memcpy(latest_payload, event->data, payload_length);
        }
        latest_payload[payload_length] = '\0';
        latest_payload_length = payload_length;
        portEXIT_CRITICAL(&payload_mux);

        if (notification_task != NULL) {
            xTaskNotifyGive(notification_task);
        }
        break;
    }

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT Event Error");
        break;

    default:
        break;
    }
}

/* --- Start MQTT Client --- */
void mqtt_app_start(void) {
    dynamic_topics_with_mac(serial_number_indoor, serial_number_outdoor1, serial_number_outdoor2);
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI, // For ESP-IDF v5.x
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(
        mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));
}

int mqtt_publish(const char *topic, const char *payload, int qos, int retain) {
    if (mqtt_client == NULL || topic == NULL || payload == NULL ||
        qos < 0 || qos > 2 || (retain != 0 && retain != 1)) {
        ESP_LOGE(TAG, "Invalid MQTT publish arguments or client not started");
        return -1;
    }

    int message_id = esp_mqtt_client_enqueue(
        mqtt_client, topic, payload, 0, qos, retain, true);
    if (message_id < 0) {
        ESP_LOGE(TAG, "Failed to queue MQTT message");
    }
    if (message_id >= 0) {
        ESP_LOGI(TAG, "Queued MQTT message, ID: %d", message_id);
    }
    return message_id;
}



void mqtt_set_notification_task(TaskHandle_t task) {
    notification_task = task;
}

int mqtt_get_latest_payload(char *dest_buf, int max_len) {
    if (dest_buf == NULL || max_len <= 0) {
        return 0;
    }

    portENTER_CRITICAL(&payload_mux);
    if (latest_payload_length == 0) {
        dest_buf[0] = '\0';
        portEXIT_CRITICAL(&payload_mux);
        return 0;
    }

    size_t copy_length = latest_payload_length;
    if (copy_length >= (size_t)max_len) {
        copy_length = (size_t)max_len - 1;
    }

    memcpy(dest_buf, latest_payload, copy_length);
    dest_buf[copy_length] = '\0';
    portEXIT_CRITICAL(&payload_mux);
    return (int)copy_length;
}
