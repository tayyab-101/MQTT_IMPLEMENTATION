#ifndef WIFI_DRIVER_H
#define WIFI_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif
#define WIFI_SSID       "StormFiber-4930"
#define WIFI_PASS       "HaEm7GG3"
#define MAXIMUM_RETRY      10
/**
 * @brief Initialize Wi-Fi in station mode and connect to the configured AP.
 *        Blocks until connection is established and IP is acquired.
 */
void wifi_init_sta(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_DRIVER_H
