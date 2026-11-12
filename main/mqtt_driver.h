#ifndef MQTT_DRIVER_H
#define MQTT_DRIVER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_BROKER_URI "mqtt://broker.hivemq.com:1883"

// static const char MQTT_TOPIC_SUB[] = "esp32/test";
// static const char MQTT_TOPIC_PUB[] = "esp32/test1";


#define MQTT_TOPIC_MAX_LEN 128
extern char MQTT_TOPIC_SUB1[MQTT_TOPIC_MAX_LEN];
extern char MQTT_TOPIC_SUB2[MQTT_TOPIC_MAX_LEN];
extern char MQTT_TOPIC_SUB3[MQTT_TOPIC_MAX_LEN];

extern char MQTT_TOPIC_PUB1[MQTT_TOPIC_MAX_LEN];
extern char MQTT_TOPIC_PUB2[MQTT_TOPIC_MAX_LEN];    
extern char MQTT_TOPIC_PUB3[MQTT_TOPIC_MAX_LEN];

// extern char MQTT_TOPIC_SUB1[] = "roofsafe/IDU-DEMO-0001/heartbeat";
// extern char MQTT_TOPIC_SUB2[] = "roofsafe/IDU-DEMO-0001/ODU-DEMO-0001/telemetry";
// extern char MQTT_TOPIC_SUB3[] = "roofsafe/IDU-DEMO-0001/ODU-DEMO-0002/telemetry";

// extern char MQTT_TOPIC_PUB1[] = "roofsafe/IDU-DEMO-0001/heartbeat";
// extern char MQTT_TOPIC_PUB2[] = "roofsafe/IDU-DEMO-0001/ODU-DEMO-0001/telemetry";
// extern char MQTT_TOPIC_PUB3[] = "roofsafe/IDU-DEMO-0001/ODU-DEMO-0002/telemetry";


/**
 * @brief Initialize and start the MQTT client.
 */
void mqtt_app_start(void);

/**
 * @brief Queue a message for publishing.
 * @return MQTT message ID, or -1 if the message could not be queued.
 */
int mqtt_publish(const char *topic, const char *payload, int qos, int retain);

/**
 * @brief Set the task to notify when an MQTT payload is received.
 */
void mqtt_set_notification_task(TaskHandle_t task);

/**
 * @brief Retrieve the latest stored MQTT payload safely.
 * 
 * @param dest_buf Pointer to destination buffer where payload will be copied.
 * @param max_len Size of destination buffer.
 * @return int Length of copied string, or 0 on failure.
 */
int mqtt_get_latest_payload(char *dest_buf, int max_len);




void dynamic_topics_with_mac(uint32_t SerialNumber_indoor, uint32_t SerialNumber_outdoor1, uint32_t SerialNumber_outdoor2);

#ifdef __cplusplus
}
#endif

#endif // MQTT_DRIVER_H
