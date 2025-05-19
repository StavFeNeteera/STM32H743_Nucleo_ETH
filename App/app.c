#include "app.h"
#include <stdio.h>
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"

#define MQTT_BROKER_IP "10.5.1.95"
#define MQTT_BROKER_PORT 1883

static mqtt_client_t *mqtt_client;

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("MQTT message on topic: %s, length: %lu\r\n", topic, tot_len);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("MQTT data: %.*s\r\n", len, data);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connected!\r\n");
        mqtt_sub_unsub(client, "test/topic", 0, NULL, NULL, 1);

        // You can subscribe or publish here
    } else {
        printf("MQTT connection failed, status: %d\r\n", status);
    }
}

void mqtt_start(void) {
    printf("mqtt_start called\r\n");

    ip_addr_t broker_ip;
    err_t err;
    ipaddr_aton(MQTT_BROKER_IP, &broker_ip);

    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = "stm32_client";
    ci.keep_alive = 60;

    mqtt_client = mqtt_client_new();
    if (mqtt_client != NULL) {
        mqtt_set_inpub_callback(mqtt_client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);
        printf("Connecting to MQTT broker...\r\n");
        mqtt_client_connect(mqtt_client, &broker_ip, MQTT_BROKER_PORT, mqtt_connection_cb, 0, &ci);
    } else {
        printf("Failed to create MQTT client\r\n");
    }
}



void app_init(void)
{
    printf("App init start\r\n");
    MX_LWIP_Init();
    printf("LWIP Init done\r\n");
//    tcpip_callback((tcpip_callback_fn)mqtt_start, NULL); // Comment this out for now
//    printf("mqtt_start scheduled\r\n");
}

void app_run(void *argument)
{
    /* ETH_CODE: Adding lwiperf to measure TCP/IP performance.
     * iperf 2.0.6 (or older?) is required for the tests. Newer iperf2 versions
     * might work without data check, but they send different headers.
     * iperf3 is not compatible at all.
     * Adding lwiperf.c file to the project is necessary.
     * The default include path should already contain
     * 'lwip/apps/lwiperf.h'
     */
    //  LOCK_TCPIP_CORE();
    //  lwiperf_start_tcp_server_default(NULL, NULL);
    //
    //  ip4_addr_t remote_addr;
    //  IP4_ADDR(&remote_addr, 192, 168, 1, 1);
    //  lwiperf_start_tcp_client_default(&remote_addr, NULL, NULL);
    //  UNLOCK_TCPIP_CORE();

    /* Infinite loop */
    static uint8_t mqtt_started = 0;

    for(;;)
    {
        struct netif *netif = netif_default;
        if (netif != NULL && netif_is_up(netif)) {
            printf("IP Address: %d.%d.%d.%d\r\n",
                   ip4_addr1(&netif->ip_addr),
                   ip4_addr2(&netif->ip_addr),
                   ip4_addr3(&netif->ip_addr),
                   ip4_addr4(&netif->ip_addr));
            if (!mqtt_started && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
	            mqtt_started = 1;
				tcpip_callback((tcpip_callback_fn)mqtt_start, NULL);
			    printf("mqtt_start scheduled\r\n");

            }
        }

        /* Toggle all LEDs using direct GPIO pins */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // LED1 - Green
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);  // LED2 - Blue
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // LED3 - Red


        osDelay(1000);
    }
}
