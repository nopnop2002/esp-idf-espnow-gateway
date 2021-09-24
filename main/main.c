/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
   This example shows how to use ESPNOW.
   Prepare two device, one for sending ESPNOW data and another for receiving
   ESPNOW data.
*/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"


#include "espnow.h"
#include "mqtt.h"

static const char *TAG = "MAIN";

QueueHandle_t xQueuePublish;
QueueHandle_t s_espnow_queue;
EventGroupHandle_t s_wifi_event_group;
SemaphoreHandle_t xSemaphoreData;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT	   BIT1


static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
								int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_STA_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA

/* WiFi should start before using ESPNOW */
static void initialise_wifi(void)
{
	esp_log_level_set("wifi", ESP_LOG_WARN);
	static bool initialized = false;
	if (initialized) {
		return;
	}

	s_wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
	assert(ap_netif);
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL) );
	ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );

	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	//ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );
	ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_NONE) );
	ESP_ERROR_CHECK( esp_wifi_start() );

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
	ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif

	initialized = true;
}


static bool wifi_apsta(void)
{
	wifi_config_t sta_config = { 0 };
	strcpy((char *)sta_config.sta.ssid, CONFIG_STA_WIFI_SSID);
	strcpy((char *)sta_config.sta.password, CONFIG_STA_WIFI_PASS);

	//ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );
	ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config) );

	ESP_ERROR_CHECK( esp_wifi_start() );
	ESP_LOGI(TAG, "WIFI_MODE_AP started.");

	ESP_ERROR_CHECK( esp_wifi_connect() );

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	esp_err_t ret = ESP_OK;
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
			WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
			pdFALSE,
			pdFALSE,
			portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", CONFIG_STA_WIFI_SSID, CONFIG_STA_WIFI_PASS);

		uint8_t sta_mac[6] = {0};
		uint8_t ap_mac[6] = {0};
		esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
		esp_wifi_get_mac(ESP_IF_WIFI_AP, ap_mac);
		//ESP_LOGI(pcTaskGetTaskName(0), "sta_mac:" MACSTR ,MAC2STR(sta_mac));
		ESP_LOGI(pcTaskGetTaskName(0), "ap_mac:" MACSTR ,MAC2STR(ap_mac));

	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", CONFIG_STA_WIFI_SSID, CONFIG_STA_WIFI_PASS);
		ret = ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		ret = ESP_FAIL;
	}
	vEventGroupDelete(s_wifi_event_group);
	return ret;

}

/* ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void example_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
	example_espnow_event_t evt;
	example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

	if (mac_addr == NULL) {
		ESP_LOGE(TAG, "Send cb arg error");
		return;
	}

	evt.id = EXAMPLE_ESPNOW_SEND_CB;
	memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
	send_cb->status = status;
	if (xQueueSend(s_espnow_queue, &evt, portMAX_DELAY) != pdTRUE) {
		ESP_LOGW(TAG, "Send send queue fail");
	}
}

static void example_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
	ESP_LOGD(TAG, "espnow_recv_cb Start");
	example_espnow_event_t evt;
	example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

	if (mac_addr == NULL || data == NULL || len <= 0) {
		ESP_LOGE(TAG, "Receive cb arg error");
		return;
	}

	evt.id = EXAMPLE_ESPNOW_RECV_CB;
	memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
	xSemaphoreTake(xSemaphoreData, portMAX_DELAY);
	recv_cb->data = malloc(len);
	if (recv_cb->data == NULL) {
		ESP_LOGE(TAG, "Malloc receive data fail");
		return;
	}
	memcpy(recv_cb->data, data, len);
	recv_cb->data_len = len;
	if (xQueueSend(s_espnow_queue, &evt, portMAX_DELAY) != pdTRUE) {
		ESP_LOGW(TAG, "Send receive queue fail");
		free(recv_cb->data);
	}
	xSemaphoreGive(xSemaphoreData);
	ESP_LOGD(TAG, "espnow_recv_cb Finish");
}

static void espnow_task(void *pvParameter)
{
	ESP_LOGI(pcTaskGetTaskName(0), "Start");
	example_espnow_event_t evt;
	example_espnow_data_t recv_data;

	MQTT_t mqttBuf;
	while (xQueueReceive(s_espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {
		if (evt.id == EXAMPLE_ESPNOW_SEND_CB) {

		} else if (evt.id == EXAMPLE_ESPNOW_RECV_CB) {
			xSemaphoreTake(xSemaphoreData, portMAX_DELAY);
			example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
			memcpy((char *)&recv_data, (char *)recv_cb->data, recv_cb->data_len);
			free(recv_cb->data);
			xSemaphoreGive(xSemaphoreData);

			// Send MQTT
			ESP_LOGI(pcTaskGetTaskName(0), "recv_data.topic=[%s]", recv_data.topic);
			ESP_LOGI(pcTaskGetTaskName(0), "recv_data.payload=[%s]", recv_data.payload);
			mqttBuf.topic_type = PUBLISH;
			strcpy(mqttBuf.topic, recv_data.topic);
			mqttBuf.topic_len = strlen(mqttBuf.topic);
			strcpy(mqttBuf.data, recv_data.payload);
			mqttBuf.data_len = strlen(mqttBuf.data);
			xQueueSend(xQueuePublish, &mqttBuf, 0);

		} else {
			ESP_LOGE(pcTaskGetTaskName(0), "Callback type error: %d", evt.id);
			break;
		}
	} // end while

	vTaskDelete( NULL );
}

void mqtt_pub(void *pvParameters);

void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK( nvs_flash_erase() );
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	initialise_wifi();
	if (wifi_apsta() != ESP_OK) {
		while(1) {
			vTaskDelay(10);
		}
	}

	// Create Queue
	s_espnow_queue = xQueueCreate( 10, sizeof(example_espnow_event_t));
	configASSERT( s_espnow_queue );
	xQueuePublish = xQueueCreate( 10, sizeof(MQTT_t) );
	configASSERT( xQueuePublish );

	// Create Semaphore
	xSemaphoreData = xSemaphoreCreateBinary();
	configASSERT( xSemaphoreData );
	xSemaphoreGive(xSemaphoreData);

	/* Initialize ESPNOW and register sending and receiving callback function. */
	ESP_ERROR_CHECK( esp_now_init() );
	// send callback not use
	//ESP_ERROR_CHECK( esp_now_register_send_cb(example_espnow_send_cb) );
	ESP_ERROR_CHECK( esp_now_register_recv_cb(example_espnow_recv_cb) );

	xTaskCreate(espnow_task, "ESPNOW", 1024*4, NULL, 4, NULL);
	xTaskCreate(mqtt_pub, "PUB", 1024*4, NULL, 4, NULL);
}
