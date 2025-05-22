#ifndef APP_H
#define APP_H

#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/netif.h"
#include "lwip/apps/mqtt.h"
#include "lwip/netifapi.h"  /* For netif_set_status_callback */

/* Function declarations */
void app_init(void);
void app_run(void *argument);
void mqtt_start(void);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void netif_status_callback(struct netif *netif);

#endif /* APP_H */
