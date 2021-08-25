#ifndef INCLUDE_MESSAGE_H_
#define INCLUDE_MESSAGE_H_

#include "stdint.h"

/*

	Название				|Размер
   ---------------------------------
	1-й ЗАГОЛОВОЧНЫЙ БАЙТ 	|  1
	2-й ЗАГОЛОВОЧНЫЙ БАЙТ	|  1
	РАЗМЕР ПАКЕТА			|  1
							.
	ДАННЫЕ					. 1-256
							.
	КОНТРОЛЬНАЯ СУММА		|  1

*/

#define PACKET_HEADER1 '\x0f'
#define PACKET_HEADER2 '\xf0'

#define PACKET_FINITE '\x55'

#define PACKET_HEADER_SIZE 3
#define MESSAGE_LENGTH_OFFSET 2
#define MIN_MESSAGE_SIZE 4

enum {
	DEV_TYPE_LIGHTER,
	DEV_TYPE_GATE,
	DEV_TYPE_ONOFF,
	DEV_TYPE_SENSOR_BME280
};


#pragma pack(1)

struct MessageCommand
{
    unsigned char type;
};

struct MessageResult
{
    uint8_t type;
    uint8_t result;
};

struct MessageStatus
{
    uint8_t type;
    uint8_t gateStatus;
    uint8_t lightStatus;
    float temperature;
    float humidity;
};

struct MessageLight
{
    uint8_t type;
    uint8_t state;
};

struct MessageGate
{
    uint8_t type;
    uint8_t status;
};

struct MessageParam
{
    uint8_t type;
    uint8_t paramNb;
    uint32_t paramValue;
};


struct MessageColor
{
    uint8_t type;
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

struct MessageFlashDataParam {
    uint8_t type;
    uint8_t apNameSize;
    int8_t  apName[32];
    uint8_t apPassSize;
    int8_t  apPass[32];
    uint8_t apType;
    uint8_t id;
    uint8_t deviceNameSize;
    int8_t  deviceName[32];
};

struct MessageMQTTDataParam {
    uint8_t type;
    int8_t  serverUri[64];
    int8_t  name[32];
    int8_t  pass[32];
    int8_t  topic[64];
};

#pragma pack()

enum {
    GATE_IDLE,
    GATE_OPEN,
    GATE_CLOSE
};

enum {
    VALUE_LOW,
    VALUE_HIGH
};


// Коды типов сообщений
enum {
    MESSAGE_GET_STATUS = 0,
    MESSAGE_STATUS,
    MESSAGE_SET_GATE_STATUS,
    MESSAGE_PULSE_GATE,
    MESSAGE_PULSE_DOOR,
    MESSAGE_SET_LIGHT_STATUS,
    MESSAGE_PULSE_LIGHT,

    MESSAGE_SET_FLASHDATAPARAM,
    MESSAGE_GET_FLASHDATAPARAM,
    MESSAGE_RESTART,

    MESSAGE_RESULT,

    MESSAGE_SET_PARAM,
    MESSAGE_GET_PARAM,
    MESSAGE_PARAM,

    MESSAGE_SET_MQTTDATAPARAM = 200,
    MESSAGE_GET_MQTTDATAPARAM
};

#endif /* INCLUDE_MESSAGE_H_ */
