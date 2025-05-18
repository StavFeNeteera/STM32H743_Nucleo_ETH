#ifndef APP_H
#define APP_H

#include "main.h"
#include "cmsis_os.h"
#include "lwip.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/netif.h"

void app_init(void);
void app_run(void *argument);

#endif /* APP_H */
