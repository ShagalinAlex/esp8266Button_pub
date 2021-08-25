/*
 * tcp_task.c
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
#include "tcp_task.h"

#include "ringbuf.h"
#include "packetmanager.h"
#include "processmessage.h"


#define BUF_SIZE (256)
#define INCOMING_PORT (5555)

extern int8_t g_dataToSend[256];
extern int8_t g_recievedData[256];

static const char *TAG = "tcp_task";

//extern FlashDataParam g_flashDataParam;

static void tcp_client_task(void *pvParameters)
{
	uint8_t buffer[BUF_SIZE];
	RINGBUF tcpClientRingBuff;
	int sock = *(int*)pvParameters;
	packetManagerInit(&tcpClientRingBuff);
	ESP_LOGI(TAG, "Client added");
	while (1) {
		if(sock == -1)
			break;
		int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
		// Error occured during receiving
		if (len < 0) {
			ESP_LOGE(TAG, "recv failed: errno %d", errno);
			break;
		}
		// Connection closed
		else if (len == 0) {
			ESP_LOGI(TAG, "Connection closed");
			break;
		}
		// Data received
		else {
			for(int i = 0; i < len; i++) {
				packetManagerAppend(&tcpClientRingBuff, buffer[i]);
			}
			int bytesRecieved = packetManagerGetMessage(&tcpClientRingBuff, (uint8_t *)g_recievedData);
			if(bytesRecieved > 0) {
				int bytesToSend = processMessage((uint8_t*)g_recievedData);
				if(bytesToSend > 0) {
					bytesToSend = createPacket((char*)g_dataToSend, bytesToSend, (char*)buffer);
					int err = send(sock, buffer, bytesToSend, 0);
					if (err < 0) {
						break;
					}
				}
			}
//			ESP_LOGI(TAG, "Received %d bytes", len);
		}
	}

	if (sock != -1) {
		ESP_LOGI(TAG, "Shutting down client socket...");
		shutdown(sock, 0);
		close(sock);
	}
	ESP_LOGI(TAG, "Client task ended");
	vTaskDelete(NULL);
}

static void tcp_server_task(void *pvParameters)
{
	char addr_str[128];
	int addr_family;
	int ip_protocol;

	struct sockaddr_in destAddr;
	destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(INCOMING_PORT);
	addr_family = AF_INET;
	ip_protocol = IPPROTO_IP;
	inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

	int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
	if (listen_sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket");
		vTaskDelete(NULL);
		return;
	}
	ESP_LOGI(TAG, "Socket created");

	int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to bind");
		vTaskDelete(NULL);
		return;
	}
	ESP_LOGI(TAG, "Socket binded");

	err = listen(listen_sock, 1);
	if (err != 0) {
		ESP_LOGE(TAG, "Error occured during listen");
		vTaskDelete(NULL);
		return;
	}
	ESP_LOGI(TAG, "Socket listening");

	while (1) {



		struct sockaddr_in sourceAddr;
		uint addrLen = sizeof(sourceAddr);
		int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
		if (sock < 0) {
			ESP_LOGE(TAG, "Unable to accept connection");
			break;
		}
		ESP_LOGI(TAG, "Socket accepted");
		xTaskCreate(tcp_client_task, "tcp_client_task", 4096, &sock, 12 , NULL);
	}
	shutdown(listen_sock, 0);
	close(listen_sock);

	ESP_LOGE(TAG, "Task deleted");
	vTaskDelete(NULL);
}



void tcp_create_task() {
	xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 1 , NULL);
}
