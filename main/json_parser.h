#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <stdbool.h>
#include <stdint.h>

#define JSON_STATUS_MAX_LENGTH 32

typedef struct {
    char status[JSON_STATUS_MAX_LENGTH];
    double temperature;
    uint32_t counter;
} mqtt_json_data_t;

extern mqtt_json_data_t data;

/**
 * @brief Parse a JSON payload containing status, temperature, and counter.
 * @return true if all required fields were present and valid.
 */
bool json_parser_parse_payload(const char *payload, mqtt_json_data_t *result);

#endif
