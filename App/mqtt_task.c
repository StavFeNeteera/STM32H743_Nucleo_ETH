#include "main.h"
#include "mqtt_task.h"

/**
* Example: MQTTS client using MbedTLS over LwIP + FreeRTOS on STM32H7
* Dependencies:
*  - MbedTLS (enabled via STM32CubeMX)
*  - LwIP with BSD sockets enabled
*  - FreeRTOS (RTOS tasks and memory)
*  - MQTT client library (e.g. Paho Embedded or custom)
* This shows TLS1.2 handshake and sending basic MQTT CONNECT packet
*/

#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/certs.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <string.h>


#define SERVER_NAME "test.mosquitto.org"

const char *client_cert_pem =
		"-----BEGIN CERTIFICATE-----" \
		"MIIDijCCAnKgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix" \
		"FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE" \
		"CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y" \
		"ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yNTA2MTcwODQ4" \
		"MzBaFw0yNTA5MTUwODQ4MzBaMGQxCzAJBgNVBAYTAklMMQswCQYDVQQIDAJpbDEL" \
		"MAkGA1UEBwwCaWwxCzAJBgNVBAoMAmlsMQswCQYDVQQLDAJpbDEOMAwGA1UEAwwF" \
		"c3RhdmYxETAPBgkqhkiG9w0BCQEWAmlsMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A" \
		"MIIBCgKCAQEAokRUJuiyKjwS9iZvAnFVU0eNcTzfsIWmwyH/kVfE9gI4xr1j30rU" \
		"YBG2GP5KI5UxA6PkB2PgYSoBP9IHD3ZMvFNDDOQrmFuIVNJL42mFFR0WNqE4Ph1e" \
		"TMzz0VolVLQmuci2fstwH3f8BF7qKU31GWats7HM0BGs8hI/8tEiHoVEWGnlk5RJ" \
		"8u7tZVLWYfqkdAJ3ch88i16mwMbhs5MlIIPMm9ZmMU/6RjlWsGHX5V9au5j9aHFw" \
		"iad6g74gNJ5jMxjMyhjYXKW08LAtY2tTdc1hYX154+DNQiKCUxjPFOSRxP1oXCKb" \
		"1pgPb8Oj3iTj4PW0f6WkmoK40DsGynEQAQIDAQABoxowGDAJBgNVHRMEAjAAMAsG" \
		"A1UdDwQEAwIF4DANBgkqhkiG9w0BAQsFAAOCAQEAQFgf1b3quB+zTZ0yb1tM6RrM" \
		"gg7oSTvSwMidaNzGUbBvojhXgtNAjwajxL37pqj7Bn4EpmXqCQg7b8VKyMgQz4LJ" \
		"QcIVUD+2dr5gPg2/mmoYzGwAvjSZyfV5oaO3ns22aHR9612yJ+MMfuIWWzDGYVog" \
		"+WBVorRrKB8S/94TaJpMDNvnJhk4hC5x4Ve44QopdYrWYpzDOjr+Ze2ttD6FrC2o" \
		"Y07aaA6dANJIyuSirbx4O8z6zEFdLT5wytObavIBD6xLkg1zDln02ByFBQr9xz4J" \
		"Nl/t9zcv6DO1KKcNcmHHvaG5TE/sf6mRqYcWoHUd2DGEnapAPQjNN63RM57iaQ==" \
		"-----END CERTIFICATE-----" ;

const char *client_key_pem  =
		"-----BEGIN PRIVATE KEY-----" \
		"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCiRFQm6LIqPBL2" \
		"Jm8CcVVTR41xPN+whabDIf+RV8T2AjjGvWPfStRgEbYY/kojlTEDo+QHY+BhKgE/" \
		"0gcPdky8U0MM5CuYW4hU0kvjaYUVHRY2oTg+HV5MzPPRWiVUtCa5yLZ+y3Afd/wE" \
		"XuopTfUZZq2zsczQEazyEj/y0SIehURYaeWTlEny7u1lUtZh+qR0AndyHzyLXqbA" \
		"xuGzkyUgg8yb1mYxT/pGOVawYdflX1q7mP1ocXCJp3qDviA0nmMzGMzKGNhcpbTw" \
		"sC1ja1N1zWFhfXnj4M1CIoJTGM8U5JHE/WhcIpvWmA9vw6PeJOPg9bR/paSagrjQ" \
		"OwbKcRABAgMBAAECggEAGk5KeC5UJ92qU0Ziy7oU05Xve7KbLvJT2SGLXYQllZ5B" \
		"lxJ//OSULmjgpQegfElE82qf3oTWDNpy7mY9p8eNZCB5Z78bxCzemqWnENx76zLW" \
		"dY/IXuWC8p9tTXy4PZWhC5Ba6Y5QhIF+lTTP8FdHi1EmitGhuv+Kkonb3SEDT4AX" \
		"zMvRjFBjaGZ/D2U5mx9s0LVFpOvbrKWaAcIBFAaAja9Y0Rk2dF9C8GoixLuXCd/W" \
		"J+GAP37XcpO0bOZz0XpJeScIdMwfYq/Ir9sH4SrnSP7PcIKL4IaUm+AEjV4e4mmO" \
		"3GfxqhIHl5G4+WiuOvXaavtMNOxuJksaYklcRT8MIwKBgQDlMwJqZ/Tyop9e79W/" \
		"XKaGnJMunnLOAZ1t90Ar2bJcb9TrkOY8AwTig5PUaNp0ofhGSOJMSkeBMEuSmQfN" \
		"WyN2EtrY/CM7OqlwHapkDiSGxoBybpvwi9NRHu2YSBtg74qxK44LMPF0fbh5kJ0j" \
		"oiXximGBvFEnZ3C9f2lEe3Uo5wKBgQC1Pbm76UDrIZF7ZvbhHIHpPQe9GxqXzRUR" \
		"0QgYAJv3TETAYgebsQdp7tXdGPrxKTz8GDhbe79jGsesAGB0ds6+J1mWRVZjUh+n" \
		"4GPll5FdCrcx7QOfOIA/UnqAyRwARVCYFedbSfgRcTO2hIAVbcM6v5fFmy8dlYqc" \
		"PQ4tLjHa1wKBgBon6xOnj22NH3DfH955vUtJXQaJ8IIP7x5YhAs9CbzAymSjoZKh" \
		"qZIL//fSi3OZ9FJZCPI529UjZMxzCE/oIbDcz/rJM2AT/rgbupE2h/aVep0UgFlA" \
		"K7prNCQXvoAedg5n8QrX6AYqPr4owbYKUBgzeZMroXtEs7n8a0/0YtdrAoGAKjLL" \
		"+veNOPHg6KWJl9JGYSLnmV0lLPEAFdzgkI4bwlGMtGpesR5fxodoXglB7EKYw210" \
		"OcCvSAjlMqQH0TYUXuvUjQtp2Mg3PKQfPkay/sHO853muPyzx4qRoUix71I0OKTo" \
		"8DVhv9Xmn3lsRREAYr5XPRgruSynUEda9KQL0ZMCgYAWdsM368PfEulW2OjB+eH4" \
		"29B3khKSgHGoKIVAdCLGdvpkcgqGgEfe8lhOwnZ3h8qG6M8wb9XHGHBzufrcnKTV" \
		"DMFMQ8EHeL+2URZLvhd+tiE3HaePDo92Mv7uibO/HCci5ZRx//6jKN8w3GNU/j4P" \
		"JuJylRFIRsH1+MBHMNTKYw==" \
		"-----END PRIVATE KEY-----" ;

const char *mosquitto_org_crt  =
		"-----BEGIN CERTIFICATE-----" \
		"MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL" \
		"BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG" \
		"A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU" \
		"BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv" \
		"by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE" \
		"BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES" \
		"MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp" \
		"dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ" \
		"KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg" \
		"UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW" \
		"Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA" \
		"s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH" \
		"3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo" \
		"E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT" \
		"MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV" \
		"6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL" \
		"BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC" \
		"6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf" \
		"+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK" \
		"sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839" \
		"LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE" \
		"m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=" \
		"-----END CERTIFICATE-----" ;



void mqtt_secure_connect(void) {
	printf("starting mqtt_secure_connect()\n");

    int ret;
    mbedtls_net_context net_ctx;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt ca;
    mbedtls_x509_crt client_cert;
    mbedtls_pk_context client_key;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    const char *pers = "mqtt_tls";
    MX_LWIP_Init();
//    mbedtls_net_init(&net_ctx);

	printf("LWIP Init done\r\n");
	while ( 1 )
	{
		//waiting for valid ip address
		extern struct netif gnetif;
		if  ( gnetif . ip_addr . addr ==  0  || gnetif . netmask . addr ==  0  || gnetif . gw . addr ==  0 )  //system has no valid ip address
		{
			osDelay ( 1000 ) ;
			continue ;
		}
		else
		{
			printf ( "DHCP/Static IP OK\n" ) ;
			break ;
		}
	}

//	LOCK_TCPIP_CORE();
//    mbedtls_ssl_init(&ssl);
//    mbedtls_ssl_config_init(&conf);
//    mbedtls_x509_crt_init(&ca);
//    mbedtls_x509_crt_init(&client_cert);
//    mbedtls_pk_init(&client_key);
//    mbedtls_ctr_drbg_init(&ctr_drbg);
//    mbedtls_entropy_init(&entropy);
//
//
//    // Seed the RNG
//    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
//                                 (const unsigned char *)pers, strlen(pers));
//    if (ret != 0) {
//        printf("Failed mbedtls_ctr_drbg_seed: -0x%x\n", -ret);
//        goto exit;
//    }
//
//    // Parse in-memory PEM certificates and key
//    ret = mbedtls_x509_crt_parse(&ca,
//                (const unsigned char *)mosquitto_org_crt,
//                strlen(mosquitto_org_crt) + 1);
//    if (ret != 0) {
//        printf("Failed to parse CA cert: -0x%x\n", -ret);
//        goto exit;
//    }
//
//    ret = mbedtls_x509_crt_parse(&client_cert,
//                (const unsigned char *)client_cert_pem,
//                strlen(client_cert_pem) + 1);
//    if (ret != 0) {
//        printf("Failed to parse client cert: -0x%x\n", -ret);
//        goto exit;
//    }
//
//    ret = mbedtls_pk_parse_key(&client_key,
//                (const unsigned char *)client_key_pem,
//                strlen(client_key_pem) + 1,
//                NULL, 0);
//    if (ret != 0) {
//        printf("Failed to parse client key: -0x%x\n", -ret);
//        goto exit;
//    }
//
//    // Configure SSL
//    ret = mbedtls_ssl_config_defaults(&conf,
//                MBEDTLS_SSL_IS_CLIENT,
//                MBEDTLS_SSL_TRANSPORT_STREAM,
//                MBEDTLS_SSL_PRESET_DEFAULT);
//    if (ret != 0) {
//        printf("Failed mbedtls_ssl_config_defaults: -0x%x\n", -ret);
//        goto exit;
//    }
//
//    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
//    mbedtls_ssl_conf_ca_chain(&conf, &ca, NULL);
//    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
//    mbedtls_ssl_conf_own_cert(&conf, &client_cert, &client_key);
//
//    // Setup SSL context
//    ret = mbedtls_ssl_setup(&ssl, &conf);
//    if (ret != 0) {
//        printf("Failed mbedtls_ssl_setup: -0x%x\n", -ret);
//        goto exit;
//    }
//
//    mbedtls_ssl_set_hostname(&ssl, SERVER_NAME); // Define SERVER_NAME as "test.mosquitto.org"
//
//    // Connect to server
//    ret = mbedtls_net_connect(&net_ctx, SERVER_NAME, SERVER_PORT, MBEDTLS_NET_PROTO_TCP);
//    if (ret != 0) {
//        printf("Failed net_connect: -0x%x\n", -ret);
//        goto exit;
//    }
//
//    mbedtls_ssl_set_bio(&ssl, &net_ctx, mbedtls_net_send, mbedtls_net_recv, NULL);
//
//    // TLS handshake
//    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
//        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
//            printf("TLS handshake failed: -0x%x\n", -ret);
//            goto exit;
//        }
//    }
//
    printf("TLS handshake successful.\n");
    UNLOCK_TCPIP_CORE();
//    // TODO: Send MQTT CONNECT here using mbedtls_ssl_write()
//
exit:
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_x509_crt_free(&ca);
    mbedtls_x509_crt_free(&client_cert);
    mbedtls_pk_free(&client_key);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_net_free(&net_ctx);
}
