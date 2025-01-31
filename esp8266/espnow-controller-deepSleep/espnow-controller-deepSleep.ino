#include <ESP8266WiFi.h>
#include <espnow.h>

ADC_MODE(ADC_VCC);

#define MQTT_TOPIC "/mqtt/espnow"
#define SLEEP 60   // 1 minite sleep

// REPLACE WITH RECEIVER MAC Address
uint8_t remoteDevice[] = {0xA4, 0xCF, 0x12, 0x05, 0xC6, 0x35};

// REPLACE WITH RECEIVER WiFi Channel
uint8 remoteChannel = 11;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char topic[64];
  char payload[64];
} struct_message;

// Create a struct_message called myData
struct_message myData;

bool esp_now_send_status;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
  esp_now_send_status = true;
}

// Callback when data is received
void ICACHE_FLASH_ATTR simple_cb(u8 *macaddr, u8 *data, u8 len) {

}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Serial.println();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  wifi_set_channel(remoteChannel);
  uint8_t chan = wifi_get_channel();
  Serial.print("current chanel=");
  Serial.println(chan);
  
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(remoteDevice, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  // Set values to send
  strcpy(myData.topic, MQTT_TOPIC); // "/mqtt/espnow";
  sprintf(myData.payload, "Hello %lu %d", millis(), ESP.getVcc());

  // Send message via ESP-NOW
  esp_now_send_status = false;
  esp_now_send(remoteDevice, (uint8_t *) &myData, sizeof(myData));

  // Wait until send complete
  while(1) {
    if (esp_now_send_status) break;
    delay(1);
  }

  // Goto DEEP SLEEP
  Serial.println("DEEP SLEEP START!!");
  uint32_t time_us = SLEEP * 1000 * 1000;
  ESP.deepSleep(time_us, WAKE_RF_DEFAULT);
  delay(1000);
}
