/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"

#include "mqtt.h"

static const char *TAG = "PUBLISH";

EventGroupHandle_t mqtt_status_event_group;
#define MQTT_CONNECTED_BIT BIT2

extern QueueHandle_t xQueuePublish;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	// your_context_t *context = event->context;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			xEventGroupClearBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);
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
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
			break;
		default:
			ESP_LOGI(TAG, "Other event id:%d", event->event_id);
			break;
	}
	return ESP_OK;
}

void mqtt_pub(void *pvParameters)
{
	ESP_LOGI(TAG, "Start Publish Broker:%s", CONFIG_BROKER_URL);
	mqtt_status_event_group = xEventGroupCreate();
	configASSERT( mqtt_status_event_group );

	esp_mqtt_client_config_t mqtt_cfg = {
		.uri = CONFIG_BROKER_URL,
		.event_handle = mqtt_event_handler,
	};

	esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	xEventGroupClearBits(mqtt_status_event_group, MQTT_CONNECTED_BIT);
	esp_mqtt_client_start(mqtt_client);
	xEventGroupWaitBits(mqtt_status_event_group, MQTT_CONNECTED_BIT, false, true, portMAX_DELAY);
	ESP_LOGI(TAG, "Connect to MQTT Server");

	MQTT_t mqttBuf;
	while (1) {
		xQueueReceive(xQueuePublish, &mqttBuf, portMAX_DELAY);
		ESP_LOGI(TAG, "type=%d", mqttBuf.topic_type);

		EventBits_t EventBits = xEventGroupGetBits(mqtt_status_event_group);
		ESP_LOGI(TAG, "EventBits=%x", EventBits);
		if (EventBits & MQTT_CONNECTED_BIT) {
			if (mqttBuf.topic_type == PUBLISH) {
				ESP_LOGI(TAG, "topic=%s data=%s", mqttBuf.topic, mqttBuf.data);
				int msg_id = esp_mqtt_client_publish(mqtt_client, mqttBuf.topic, mqttBuf.data, 0, 1, 0);
				ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
			} else if (mqttBuf.topic_type == STOP) {
				ESP_LOGI(TAG, "Task Stop");
				break;
			}
		} else {
			ESP_LOGW(TAG, "Disconnect to MQTT Server. Skip to send");
		}
	}

	ESP_LOGI(TAG, "Task Delete");
	esp_mqtt_client_stop(mqtt_client);
	vTaskDelete(NULL);

}
