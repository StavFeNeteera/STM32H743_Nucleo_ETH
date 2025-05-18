#include "app.h"
#include <stdio.h>

void app_init(void)
{
    /* init code for LWIP */
    MX_LWIP_Init();
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
