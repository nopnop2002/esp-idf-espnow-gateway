#include <ESP8266WiFi.h>
#include <espnow.h>

ADC_MODE(ADC_VCC);

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x3c, 0x71, 0xbf, 0x4f, 0xc1, 0xa1};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char topic[64];
  char payload[64];
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}


void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  Serial.println();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

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
  // If the channel is set to 0, data will be sent on the current channel. 
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 0, NULL, 0);
  //esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    lastTime = millis();
    // Set values to send
    strcpy(myData.topic, "/mqtt/espnow");
    //strcpy(myData.payload, "Hello World!");
    sprintf(myData.payload, "Hello %d %d", lastTime, ESP.getVcc());

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  }
}
