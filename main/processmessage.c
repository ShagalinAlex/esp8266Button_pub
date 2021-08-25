#include "stdint.h"
#include "string.h"

#include "esp_log.h"
#include "esp_err.h"


#include "driver/gpio.h"
#include "mqtt_client.h"


#include "WEMOS.h"
#include "programmdata.h"
#include "message.h"
#include "processmessage.h"

int8_t g_dataToSend[256];
int8_t g_recievedData[256];
extern FlashDataParam g_flashDataParam;
extern MQTTDataParam g_MQTTDataParam;
extern esp_mqtt_client_handle_t mqtt_client;

//static const char *TAG = "process_message";
int currentState = 0;

void SetChannelState(ChannelState_enum state) {
	if(mqtt_client != NULL)
		esp_mqtt_client_publish(mqtt_client, (char*)g_MQTTDataParam.topic, state > 0 ? "ON" : "OFF", 0, 0, 0);
	gpio_set_level(GPIO_D1, state > 0 ? 1 : 0);
}

ChannelState_enum GetChannelState() {
	return gpio_get_level(GPIO_D1) > 0 ? STATE_ON : STATE_OFF;
}

ChannelState_enum ToggleChannelState() {
	ChannelState_enum newState = gpio_get_level(GPIO_D1) > 0 ? STATE_OFF : STATE_ON;

	gpio_set_level(GPIO_D1, newState);
	if(mqtt_client != NULL)
		esp_mqtt_client_publish(mqtt_client, (char*)g_MQTTDataParam.topic, newState == STATE_ON ? "ON" : "OFF", 0, 0, 0);
	return newState;
}

int setResultError()
{
	struct MessageResult outMessage;
	outMessage.type = MESSAGE_RESULT;
	outMessage.result = 255;
	memcpy((uint8_t*)g_dataToSend, (uint8_t*)&outMessage, sizeof(outMessage));
	return sizeof(outMessage);
}

int processMessage(uint8_t *data)
{
	if(data[0] == MESSAGE_SET_PARAM)
	{
		struct MessageParam *message = (struct MessageParam *)data;

    	struct MessageResult outMessage;
    	outMessage.type = MESSAGE_RESULT;


		if(mqtt_client != NULL)
			esp_mqtt_client_publish(mqtt_client, (char*)g_MQTTDataParam.topic, message->paramValue > 0 ? "ON" : "OFF", 0, 0, 0);
		gpio_set_level(GPIO_D1, message->paramValue > 0 ? 1 : 0);
		outMessage.result = 1;


//		switch(message->paramNb){
//		case 1:
//			sprintf(topic, "%s/GPIO_1", g_MQTTDataParam.topic);
//			esp_mqtt_client_publish(mqtt_client, topic, message->paramValue > 0 ? "ON" : "OFF", 0, 0, 0);
//			gpio_set_level(GPIO_D1, message->paramValue > 0 ? 1 : 0);
//			outMessage.result = 1;
//			break;
//		case 2:
//			sprintf(topic, "%s/GPIO_2", g_MQTTDataParam.topic);
//			esp_mqtt_client_publish(mqtt_client, topic, message->paramValue > 0 ? "ON" : "OFF", 0, 0, 0);
//			gpio_set_level(GPIO_D2, message->paramValue > 0 ? 1 : 0);
//			outMessage.result = 1;
//			break;
//		case 3:
//			sprintf(topic, "%s/GPIO_3", g_MQTTDataParam.topic);
//			esp_mqtt_client_publish(mqtt_client, topic, message->paramValue > 0 ? "ON" : "OFF", 0, 0, 0);
//			gpio_set_level(GPIO_D3, message->paramValue > 0 ? 1 : 0);
//			outMessage.result = 1;
//			break;
//		case 4:
//			sprintf(topic, "%s/GPIO_4", g_MQTTDataParam.topic);
//			esp_mqtt_client_publish(mqtt_client, topic, message->paramValue > 0 ? "ON" : "OFF", 0, 0, 0);
//			gpio_set_level(GPIO_D4, message->paramValue > 0 ? 1 : 0);
//			outMessage.result = 1;
//			break;
//		default:
//			outMessage.result = 0;
//			break;
//		}

		memcpy((uint8_t*)g_dataToSend, (uint8_t*)&outMessage, sizeof(outMessage));
    	return sizeof(outMessage);
	}

	if(data[0] == MESSAGE_GET_PARAM)
	{
//		struct MessageParam *message = (struct MessageParam*)data;
		struct MessageParam outMessage;
    	outMessage.type = MESSAGE_PARAM;

    	outMessage.paramNb = 1;
    	outMessage.paramValue = gpio_get_level(GPIO_D1) > 0 ? 1 : 0;

//		switch(message->paramNb){
//		case 1:
//	    	outMessage.paramNb = 1;
//	    	outMessage.paramValue = gpio_get_level(GPIO_D1) > 0 ? 1 : 0;
//			break;
//		case 2:
//	    	outMessage.paramNb = 2;
//	    	outMessage.paramValue = gpio_get_level(GPIO_D2) > 0 ? 1 : 0;
//			break;
//		case 3:
//	    	outMessage.paramNb = 3;
//	    	outMessage.paramValue = gpio_get_level(GPIO_D3) > 0 ? 1 : 0;
//			break;
//		case 4:
//	    	outMessage.paramNb = 4;
//	    	outMessage.paramValue = gpio_get_level(GPIO_D4) > 0 ? 1 : 0;
//			break;
//		default:
//	    	return setResultError();
//		}


		memcpy((int8_t*)g_dataToSend, (int8_t*)&outMessage, sizeof(outMessage));
		return sizeof(outMessage);
	}

	if(data[0] == MESSAGE_SET_FLASHDATAPARAM )
	{
//		ESP_LOGI(TAG, "MESSAGE_SET_FLASHDATAPARAM/n/r");
		struct MessageFlashDataParam *message = (struct MessageFlashDataParam *)data;

		FlashDataParam param;
		memset(&param, 0, sizeof(param));

		memcpy(param.apName, message->apName , message->apNameSize);
		param.apNameSize = message->apNameSize;
		memcpy(param.apPass, message->apPass , message->apPassSize);
		param.apPassSize = message->apPassSize;
		memcpy(param.deviceName, message->deviceName , message->deviceNameSize);
		param.deviceNameSize = message->deviceNameSize;
		param.apType = message->apType;
		param.id = message->id;

		uint8_t result = (uint8_t)ProgDataWrite(&param);

		if(result == 0)
		{
			memcpy(g_flashDataParam.apName, message->apName , message->apNameSize);
			g_flashDataParam.apNameSize = message->apNameSize;
			memcpy(g_flashDataParam.apName, message->apPass , message->apPassSize);
			g_flashDataParam.apPassSize = message->apPassSize;
			memcpy(g_flashDataParam.deviceName, message->deviceName , message->deviceNameSize);
			g_flashDataParam.deviceNameSize = message->deviceNameSize;
			g_flashDataParam.apType = message->apType;
		}


    	struct MessageResult outMessage;
    	outMessage.type = MESSAGE_RESULT;
    	outMessage.result = result;

		memcpy(g_dataToSend, (int8_t*)&outMessage, sizeof(outMessage));
    	return sizeof(outMessage);
	}

	if(data[0] == MESSAGE_GET_FLASHDATAPARAM )
	{
		struct MessageFlashDataParam outMessage;
		uint8_t result;
		memset((char*)&outMessage, 0, sizeof(outMessage));
		result = ProgDataRead(&g_flashDataParam);

		outMessage.type = MESSAGE_GET_FLASHDATAPARAM;

		if(g_flashDataParam.apNameSize > 32 || result != 0) {
			memcpy(outMessage.apName, "undef", sizeof(outMessage.apName));
			outMessage.apNameSize = 5;
		}
		else {
			memcpy(outMessage.apName, g_flashDataParam.apName, g_flashDataParam.apNameSize);
			outMessage.apNameSize = g_flashDataParam.apNameSize;
		}

		if(g_flashDataParam.apPassSize > 32 || result != 0) {
			memcpy(outMessage.apPass, "undef", sizeof(outMessage.apPass));
			outMessage.apPassSize = 5;
		}
		else {
			memcpy(outMessage.apPass, g_flashDataParam.apPass, g_flashDataParam.apPassSize);
			outMessage.apPassSize = g_flashDataParam.apPassSize;
		}

		if(g_flashDataParam.deviceNameSize > 32 || result != 0) {
			memcpy(outMessage.deviceName, "undef", sizeof(outMessage.deviceName));
			outMessage.deviceNameSize = 5;
		}
		else {
			memcpy(outMessage.deviceName, g_flashDataParam.deviceName, g_flashDataParam.deviceNameSize);
			outMessage.deviceNameSize = g_flashDataParam.deviceNameSize;
		}

		outMessage.apType = g_flashDataParam.apType;
		outMessage.id = g_flashDataParam.id;

		memcpy(g_dataToSend, (int8_t*)&outMessage, sizeof(outMessage));
		return sizeof(struct MessageFlashDataParam);
	}

	if(data[0] == MESSAGE_SET_MQTTDATAPARAM )
	{
//		ESP_LOGI(TAG, "MESSAGE_SET_MQTTDATAPARAM/n/r");
		struct MessageMQTTDataParam *message = (struct MessageMQTTDataParam *)data;

		MQTTDataParam param;
		memset(&param, 0, sizeof(param));



		memcpy(param.serverUri, message->serverUri, 64);
		memcpy(param.name, message->name, 32);
		memcpy(param.pass, message->pass, 32);
		memcpy(param.topic, message->topic, 64);

		uint8_t result = (uint8_t)ProgMQTTDataWrite(&param);

		if(result == 0)
		{
			memcpy(g_MQTTDataParam.serverUri, message->serverUri, 64);
			memcpy(g_MQTTDataParam.name, message->name, 32);
			memcpy(g_MQTTDataParam.pass, message->pass, 32);
			memcpy(g_MQTTDataParam.topic, message->topic, 64);
		}

    	struct MessageResult outMessage;
    	outMessage.type = MESSAGE_RESULT;
    	outMessage.result = result;

		memcpy(g_dataToSend, (int8_t*)&outMessage, sizeof(outMessage));
    	return sizeof(outMessage);
	}

	if(data[0] == MESSAGE_GET_MQTTDATAPARAM )
	{
		struct MessageMQTTDataParam outMessage;
//		memset((char*)&outMessage, 0, sizeof(outMessage));

		outMessage.type = MESSAGE_GET_MQTTDATAPARAM;

		memcpy(outMessage.serverUri, g_MQTTDataParam.serverUri, 64);
		memcpy(outMessage.name, g_MQTTDataParam.name, 32);
		memcpy(outMessage.pass, g_MQTTDataParam.pass, 32);
		memcpy(outMessage.topic, g_MQTTDataParam.topic, 64);

		memcpy(g_dataToSend, (int8_t*)&outMessage, sizeof(outMessage));
		return sizeof(outMessage);
	}

	if(data[0] == MESSAGE_RESTART)
	{
		//system_restart();
		return 0;
	}

	return 0;
}
