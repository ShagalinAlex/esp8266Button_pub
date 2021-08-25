/*
 * udp_task.h
 *
 *  Created on: 25 џэт. 2020 у.
 *      Author: Nemo
 */

#ifndef INCLUDE_UDP_TASK_H_
#define INCLUDE_UDP_TASK_H_

#include "stdint.h"

#pragma pack(1)
typedef struct _udpSendData {
	uint8_t id;
	uint8_t type;
	uint8_t deviceNameSize;
	int8_t deviceName[32];
} UdpSendData;
#pragma pack()

void udp_create_task();

#endif /* INCLUDE_UDP_TASK_H_ */
