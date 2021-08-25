/*
 * programmdata.h
 *
 *  Created on: 21 џэт. 2020 у.
 *      Author: Nemo
 */

#ifndef INCLUDE_PROGRAMMDATA_H_
#define INCLUDE_PROGRAMMDATA_H_

typedef struct _FlashDataParam {
	char 		  apName[32];
	char 		  apPass[32];
	unsigned char apNameSize;
	unsigned char apPassSize;
	unsigned char apType;
	unsigned char id;
    char          deviceName[32];
    unsigned char deviceNameSize;
} FlashDataParam;

typedef struct _MQTTDataParam {
    int8_t  serverUri[64];
    int8_t  name[32];
    int8_t  pass[32];
    int8_t  topic[64];
} MQTTDataParam;

int ProgDataRead(FlashDataParam *params);
int ProgDataWrite(FlashDataParam *params);

int ProgMQTTDataRead(MQTTDataParam *params);
int ProgMQTTDataWrite(MQTTDataParam *params);

#endif /* INCLUDE_PROGRAMMDATA_H_ */
