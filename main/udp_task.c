/*
 * udp_task.c
 *
 *  Created on: 25 џэт. 2020 у.
 *      Author: Nemo
 */
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "programmdata.h"
#include "message.h"
#include "udp_task.h"

static const char *TAG = "udp_task";

static UdpSendData udpSendData;

extern FlashDataParam g_flashDataParam;

#define INCOMING_PORT (5555)

void udp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(INCOMING_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create UDP socket");
            break;
        }
//        ESP_LOGI(TAG, "Socket created");

        int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err < 0) {
            ESP_LOGE(TAG, "UDP socket unable to bind");
        }

        while (1) {


            struct sockaddr_in sourceAddr;
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&sourceAddr, &socklen);

            // Error occured during receiving
            if (len < 0) {
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...

                memset((int8_t*)&udpSendData, 0, sizeof(udpSendData));
                udpSendData.id = destAddr.sin_addr.s_addr & 0xff;
                udpSendData.type = DEV_TYPE_ONOFF;
                udpSendData.deviceNameSize = g_flashDataParam.deviceNameSize;
                memset(udpSendData.deviceName, 0, 32);
                memcpy(udpSendData.deviceName, g_flashDataParam.deviceName, g_flashDataParam.deviceNameSize);
              	//sourceAddr.sin_port = 5556;
                ((struct sockaddr_in *)&sourceAddr)->sin_port = 0xB415; // 5556 т big endian

              	int err = sendto(sock, (int8_t*)&udpSendData, sizeof(UdpSendData), 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr));
                if (err < 0) {
                    break;
                }
            }
        }

        if (sock != -1) {
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void udp_create_task() {
	xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 1 , NULL);
}
