#include <limits.h>
#include <string.h>

#include "cJSON.h"
#include "json_parser.h"

mqtt_json_data_t data;

bool json_parser_parse_payload(const char *payload, mqtt_json_data_t *result) {
    if (payload == NULL || result == NULL) {
        return false;
    }

    cJSON *root = cJSON_Parse(payload);
    if (root == NULL || !cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return false;
    }

    const cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
    const cJSON *temperature = cJSON_GetObjectItemCaseSensitive(root, "temperature");
    const cJSON *counter = cJSON_GetObjectItemCaseSensitive(root, "counter");

    bool valid = cJSON_IsString(status) && status->valuestring != NULL &&
                 strlen(status->valuestring) < sizeof(result->status) &&
                 cJSON_IsNumber(temperature) &&
                 cJSON_IsNumber(counter) &&
                 counter->valuedouble >= 0 &&
                 counter->valuedouble <= UINT32_MAX &&
                 (double)(uint32_t)counter->valuedouble == counter->valuedouble;

    if (valid) {
        strcpy(result->status, status->valuestring);
        result->temperature = temperature->valuedouble;
        result->counter = (uint32_t)counter->valuedouble;
    }

    cJSON_Delete(root);
    return valid;
}
