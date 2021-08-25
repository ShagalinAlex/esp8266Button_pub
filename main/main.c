#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"

#include <string.h>
#include "esp_event.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>


#include "esp_spiffs.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "mqtt_client.h"


#include "ringbuf.h"
#include "programmdata.h"
#include "processmessage.h"
#include "WEMOS.h"
#include "filter.h"

#include "uart_task.h"
#include "udp_task.h"
#include "tcp_task.h"



#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))


static const char *TAG = "main";

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

const int IPV4_GOTIP_BIT = BIT0;
#ifdef CONFIG_EXAMPLE_IPV6
const int IPV6_GOTIP_BIT = BIT1;
#endif

FlashDataParam g_flashDataParam;
MQTTDataParam g_MQTTDataParam;

static EventGroupHandle_t wifi_event_group;

//typedef enum {
//	MQTT_STATE_DISCONNECTED,
//	MQTT_STATE_CONNECTED
//} esp_mqtt_state_t;

esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqttConnected = false;

void mqtt_app_start(void);
void gpio_task(void *pvParameters);

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    system_event_info_t *info = &event->event_info;

    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        //ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
        //printf("SYSTEM_EVENT_STA_GOT_IP");
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        //ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);
        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G);
        }
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
    default:
        break;
    }
    return ESP_OK;
}

static void wait_for_ip()
{
#ifdef CONFIG_EXAMPLE_IPV6
    uint32_t bits = IPV4_GOTIP_BIT | IPV6_GOTIP_BIT;
#else
    uint32_t bits = IPV4_GOTIP_BIT;
#endif

    ESP_LOGI(TAG, "Waiting for AP connection...");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");
}

static void initialise_wifi_sta(void)
{
	tcpip_adapter_init();
	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	wifi_config_t wifi_config = { };

	strcpy((char*)wifi_config.sta.ssid, g_flashDataParam.apName);
	strcpy((char*)wifi_config.sta.password, g_flashDataParam.apPass);

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_LOGI(TAG, "start the WIFI SSID:[%s]\n\r", g_flashDataParam.apName);
	ESP_LOGI(TAG, "start the WIFI PASS:[%s]\n\r", g_flashDataParam.apPass);
	ESP_LOGI(TAG, "start the WIFI SSID:[%s]\n\r", wifi_config.sta.ssid);
	ESP_LOGI(TAG, "start the WIFI PASS:[%s]\n\r", wifi_config.sta.password);
	ESP_ERROR_CHECK(esp_wifi_start());

    wait_for_ip();
    udp_create_task();
    tcp_create_task();
    mqtt_app_start();
}

static void initialise_wifi_softap(void)
{

	tcpip_adapter_init();
	esp_event_loop_init(event_handler, NULL);
    // stop DHCP server
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    // assign a static IP to the network interface
    tcpip_adapter_ip_info_t info = { 0, };
    IP4_ADDR(&info.ip, 192, 168, 1, 1);
    IP4_ADDR(&info.gw, 192, 168, 1, 1);//ESP acts as router, so gw addr will be its own addr
    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
    printf("DHCP server started \n");


    wifi_config_t wifi_config = {
            .ap = {
            .channel = 0,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 5,
            .beacon_interval = 100
            }
    };

	strcpy((char*)wifi_config.sta.ssid, g_flashDataParam.apName);

	if(strlen(g_flashDataParam.apPass) < 8)
		strcpy((char*)wifi_config.sta.password, "00000000");
	else
		strcpy((char*)wifi_config.sta.password, g_flashDataParam.apPass);


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    udp_create_task();
    tcp_create_task();
}

static void initialise_wifi_softap_free(void)
{

	tcpip_adapter_init();
	esp_event_loop_init(event_handler, NULL);
    // stop DHCP server
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
    // assign a static IP to the network interface
    tcpip_adapter_ip_info_t info = { 0, };
    IP4_ADDR(&info.ip, 192, 168, 1, 1);
    IP4_ADDR(&info.gw, 192, 168, 1, 1);//ESP acts as router, so gw addr will be its own addr
    IP4_ADDR(&info.netmask, 255, 255, 255, 0);
    ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));
    ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
    printf("DHCP server started \n");


    wifi_config_t wifi_config = {
            .ap = {
            .channel = 0,
            .authmode = WIFI_AUTH_OPEN,
            .ssid_hidden = 0,
            .max_connection = 5,
            .beacon_interval = 100
            }
    };

	strcpy((char*)wifi_config.sta.ssid, g_flashDataParam.apName);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    udp_create_task();
    tcp_create_task();
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id = 0;
    // your_context_t *context = event->context;
    char topic[64];

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqttConnected = true;


            memset(topic, 0, 64);
            sprintf((char*)topic, "%s/set", (char*)g_MQTTDataParam.topic);
            msg_id = esp_mqtt_client_subscribe(client, (char*)topic, 0);

            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqttConnected = false;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
//            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
//            printf("DATA=%.*s\r\n", event->data_len, event->data);

            if(strstr(event->data, "ON")) {
            	SetChannelState(STATE_ON);
            }
            else {
            	SetChannelState(STATE_OFF);
            }

            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = mqtt_event_handler,
        // .user_context = (void *)your_context
    };

    mqtt_cfg.host = (char*)g_MQTTDataParam.serverUri;
    mqtt_cfg.username = (char*)g_MQTTDataParam.name;
    mqtt_cfg.password = (char*)g_MQTTDataParam.pass;

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if(mqtt_client)
    	esp_mqtt_client_start(mqtt_client);
}

void app_main()
{
	gpio_config_t output_conf;
	output_conf.intr_type = GPIO_INTR_DISABLE;

	output_conf.mode = GPIO_MODE_OUTPUT;
	output_conf.pin_bit_mask = (1ULL<<GPIO_D1);
	output_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	output_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&output_conf));

	gpio_config_t input_conf;
	input_conf.intr_type = GPIO_INTR_DISABLE;

	input_conf.mode = GPIO_MODE_INPUT;
	input_conf.pin_bit_mask = (1ULL<<GPIO_D4);
	input_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	input_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	ESP_ERROR_CHECK(gpio_config(&input_conf));

	xTaskCreate(gpio_task, "gpio_task", 4096, NULL, 10 , NULL);

	//ESP_ERROR_CHECK(gpio_set_level(GPIO_OUTPUT_IO_1, 1));

	wifi_event_group = xEventGroupCreate();
    uart_create_task();



    memset(&g_MQTTDataParam, 0, sizeof(MQTTDataParam));
	if(ProgMQTTDataRead(&g_MQTTDataParam) == 0) {
		ESP_LOGI(TAG, "Load MQTT params done!\n\r");
		ESP_LOGI(TAG, "MQTT uri:   %s\n\r", g_MQTTDataParam.serverUri);
		ESP_LOGI(TAG, "MQTT name:  %s\n\r", g_MQTTDataParam.name);
		ESP_LOGI(TAG, "MQTT pass:  %s\n\r", g_MQTTDataParam.pass);
		ESP_LOGI(TAG, "MQTT topic: %s\n\r", g_MQTTDataParam.topic);
	}
	else {
		ProgMQTTDataWrite(&g_MQTTDataParam);
	}


    memset(&g_flashDataParam, 0, sizeof(FlashDataParam));
	if(ProgDataRead(&g_flashDataParam) == 0) {
		if(g_flashDataParam.apType == 1) {
			ESP_LOGI(TAG, "Load ST \n\r");
			initialise_wifi_sta();
		}
		else {
			ESP_LOGI(TAG, "Load AP \n\r");
			initialise_wifi_softap();
		}
	}
	else {
		ESP_LOGE(TAG, "Cannot read ProgData");
		memset(&g_flashDataParam, 0, sizeof(FlashDataParam));
		g_flashDataParam.apType = 2;
		g_flashDataParam.deviceNameSize = 9;
		memcpy(g_flashDataParam.deviceName, "ESPDevice", 9);
		memcpy(g_flashDataParam.apName, "ESPDevice", 9);
		ProgDataWrite(&g_flashDataParam);
		initialise_wifi_softap_free();
	}


	ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

}

void gpio_task(void *pvParameters)
{
	int stateCounter = 0;
	int lastState = gpio_get_level(GPIO_D4);
	int currentState;

    while (1) {
    	currentState = gpio_get_level(GPIO_D4);
    	if(currentState != lastState) {
    		if(stateCounter++ > 0) {
    			stateCounter = 0;
    			lastState = currentState;
    			ToggleChannelState();
    		}
    	}
    	else {
    		stateCounter = 0;
    	}

    	vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
