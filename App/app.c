#include "app.h"
#include <stdio.h>
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "lwip/tcpip.h"

#define TCP_SERVER_PORT 1234
static struct tcp_pcb *tcp_server_pcb = NULL;

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL) {
        printf("Message received: %.*s\r\n", p->len, (char*)p->payload);
        tcp_recved(tpcb, p->len);
        pbuf_free(p);
    }
    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    printf("TCP connection made\r\n");
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

void tcp_server_init(void)
{
    printf("Trying to set TCP server\r\n");
    tcp_server_pcb = tcp_new();
    if (tcp_server_pcb != NULL) {
        err_t err = tcp_bind(tcp_server_pcb, IP_ADDR_ANY, TCP_SERVER_PORT);
        if (err == ERR_OK) {
            tcp_server_pcb = tcp_listen(tcp_server_pcb);
            tcp_accept(tcp_server_pcb, tcp_server_accept);
            printf("TCP is up, listening on port %d\r\n", TCP_SERVER_PORT);
        } else {
            printf("TCP bind failed\r\n");
        }
    } else {
        printf("TCP PCB allocation failed\r\n");
    }
}

void app_init(void)
{
    /* init code for LWIP */
    MX_LWIP_Init();
    tcpip_callback((tcpip_callback_fn)tcp_server_init, NULL);
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
        struct netif *netif = netif_default;
        if (netif != NULL && netif_is_up(netif)) {
            printf("IP Address: %d.%d.%d.%d\r\n",
                   ip4_addr1(&netif->ip_addr),
                   ip4_addr2(&netif->ip_addr),
                   ip4_addr3(&netif->ip_addr),
                   ip4_addr4(&netif->ip_addr));
        }

        /* Toggle all LEDs using direct GPIO pins */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);  // LED1 - Green
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);  // LED2 - Blue
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14); // LED3 - Red
        
        osDelay(1000);
    }
}
