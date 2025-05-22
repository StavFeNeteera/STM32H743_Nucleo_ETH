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
	printf("got in mqtt_incoming_publish_cb\r\n");
    printf("MQTT message on topic: %s, length: %lu\r\n", topic, tot_len);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("MQTT data: %.*s\r\n", len, data);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connected!\r\n");
        mqtt_sub_unsub(client, "test/topic", 0, NULL, NULL, 1);

        // Publish a message to the broker
        const char *message = "Hello from STM32!";
        err_t err = mqtt_publish(client, "test/topic", message, strlen(message), 0, 0, NULL, NULL);
        if (err == ERR_OK) {
            printf("Message published successfully\r\n");
        } else {
            printf("Failed to publish message, error: %d\r\n", err);
        }
    } else {
        printf("MQTT connection failed, status: %d\r\n", status);
    }
}

static void mqtt_start_from_tcpip(void *arg)
{
    printf("Starting MQTT connection from TCP/IP thread\r\n");
    mqtt_start();
}

static void netif_status_callback(struct netif *netif)
{
    if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
        char ip_str[16];
        ipaddr_ntoa_r(&netif->ip_addr, ip_str, sizeof(ip_str));
        
        printf("IP address acquired: %s\n", ip_str);
        
        printf("Valid IP detected, connecting to MQTT broker at %s:%d\r\n", MQTT_BROKER_IP, MQTT_BROKER_PORT);
        tcpip_callback(mqtt_start_from_tcpip, NULL);
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
    
    /* Register callback for network interface status changes */
    LOCK_TCPIP_CORE();
    netif_set_status_callback(netif_default, netif_status_callback);
    UNLOCK_TCPIP_CORE();
    printf("Waiting for network interface to be ready...\r\n");
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
    for(;;)
    {
        /* Toggle all LEDs using direct GPIO pins */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // LED1 - Green
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);  // LED2 - Blue
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // LED3 - Red

        osDelay(1000);
    }
}
