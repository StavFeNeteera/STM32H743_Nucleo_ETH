
#ifndef MQTT_TASK_H_
#define MQTT_TASK_H_

//#include "MQTTClient.h"
//#include "MQTTInterface.h"
#include "cmsis_os.h"
#include <string.h>
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_internal.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/debug.h"

//#define MQTT_BROKER_IP "10.5.1.95"
//#define MQTT_BROKER_IP "192.168.1.112"
#define MQTT_BROKER_PORT 1883

#define BROKER_IP		"192.168.1.112"

#define MQTTS //changes port and function call


#ifdef MQTTS
#define SERVER_PORT "8883"
#else
#define SERVER_PORT 1883
#endif

void mqtt_secure_connect(void);


#endif /* MQTT_TASK_H_ */
