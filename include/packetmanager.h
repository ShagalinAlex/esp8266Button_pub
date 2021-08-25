/*
 * packetmanager.h
 *
 *  Created on: 28 февр. 2017 г.
 *      Author: nemo
 */

#ifndef INCLUDE_PACKETMANAGER_H_
#define INCLUDE_PACKETMANAGER_H_

#include "stdint.h"
#include "ringbuf.h"


void packetManagerInit(RINGBUF *ringbuff);
int packetManagerGetMessage(RINGBUF *ringbuff, uint8_t *data);
void packetManagerAppend(RINGBUF *ringbuff, uint8_t nData);
short createPacket(char *buffer, int len, char *outBuffer);

#endif /* INCLUDE_PACKETMANAGER_H_ */
