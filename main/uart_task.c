/*
 * uart_task.c
 *
 *  Created on: 25 џэт. 2020 у.
 *      Author: Nemo
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"


#include <string.h>


#include "driver/uart.h"

#include "ringbuf.h"
#include "packetmanager.h"
#include "processmessage.h"

extern int8_t g_dataToSend[256];
extern int8_t g_recievedData[256];

#define EX_UART_NUM UART_NUM_0

#define BUF_SIZE (256)
static QueueHandle_t uart0_queue;
static RINGBUF uartRingBuff;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t *dtmp = (uint8_t *) malloc(BUF_SIZE);
    packetManagerInit(&uartRingBuff);

    for (;;) {
        // Waiting for UART event.
        if (xQueueReceive(uart0_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
            switch (event.type) {
                // Event of UART receving data
                // We'd better handler data event fast, there would be much more data events than
                // other types of events. If we take too much time on data event, the queue might be full.
                case UART_DATA:
                    uart_read_bytes(EX_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    for(int i = 0; i < event.size; i++) {
                    	packetManagerAppend(&uartRingBuff, dtmp[i]);
                    }
                    int bytesRecieved = packetManagerGetMessage(&uartRingBuff, (uint8_t *)g_recievedData);
                    if(bytesRecieved > 0) {
                    	int bytesToSend = processMessage((uint8_t*)g_recievedData);
                    	if(bytesToSend > 0) {
                    		bytesToSend = createPacket((char*)g_dataToSend, bytesToSend, (char*)dtmp);
                    		uart_write_bytes(EX_UART_NUM, (const char *)dtmp, bytesToSend);
                    	}
                    }
                    break;

                // Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;

                // Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;

                case UART_PARITY_ERR:
                    break;

                // Event of UART frame error
                case UART_FRAME_ERR:
                    break;

                // Others
                default:
                    break;
            }
        }
    }

    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void uart_create_task() {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(EX_UART_NUM, &uart_config);

    // Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 100, &uart0_queue, 0);
	xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 5, NULL);
}
