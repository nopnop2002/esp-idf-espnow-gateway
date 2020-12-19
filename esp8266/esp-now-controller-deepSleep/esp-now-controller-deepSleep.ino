#include <ESP8266WiFi.h>
#include <espnow.h>

ADC_MODE(ADC_VCC);

#define SLEEP 60   // 1 minite sleep
//#define SLEEP 600   // 10 minute sleep
#define MQTT_TOPIC "/deepsleep/espnow"

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

// RTC memory (128*4byte,512Byte) structure
struct {
  uint32_t interval;
  uint32_t counter;
  byte data[504]; // User Data
} rtcData;

// Print RTC memory
void printMemory(int sz) {
  char buf[3];
//  for (int i = 0; i < sizeof(rtcData); i++) {
  for (int i = 0; i < sz; i++) {
    sprintf(buf, "%02X", rtcData.data[i]);
    Serial.print(buf);
    if ((i + 1) % 32 == 0) {
      Serial.println();
    }
    else {
      Serial.print(" ");
    }
  }
  Serial.println();
}

bool esp_now_sended;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
  esp_now_sended = true;
}


void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  long startMillis = millis();
  Serial.print("ESP.getResetReason()=");
  Serial.println(ESP.getResetReason());
  String resetReason = ESP.getResetReason();
  
  /*
  enum rst_reason {
  REANSON_DEFAULT_RST = 0, // ノーマルスタート。電源オンなど。
  REANSON_WDT_RST = 1, // ハードウェアウォッチドッグによるリセット
  REANSON_EXCEPTION_RST = 2, // 例外によるリセット。GPIO状態は変化しない
  REANSON_SOFT_WDT_RST = 3, // ソフトウェアウォッチドッグによるリセット。GPIO状態は変化しない
  REANSON_SOFT_RESTART = 4, // ソフトウェアによるリセット。GPIO状態は変化しない
  REANSON_DEEP_SLEEP_AWAKE= 5, // ディープスリープ復帰
  REANSON_EXT_SYS_RST = 6, // 外部要因(RSTピン)によるリセット。
  };
  */

  rst_info *prst = ESP.getResetInfoPtr();
  /*
  struct rst_info{
      uint32 reason;
      uint32 exccause;
      uint32 epc1;
      uint32 epc2;
      uint32 epc3;
      uint32 excvaddr;
      uint32 depc;
  };
  */
  Serial.print("reset reason=");
  Serial.println(prst->reason);

  // Read data from RTC memory
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    Serial.println("Read: ");
    printMemory(10);
    if (prst->reason == 0) { // Normal Start
      rtcData.interval = SLEEP;
      rtcData.counter = 0;
      for (int i = 0; i < sizeof(rtcData); i++) {
        rtcData.data[i] = 0;
      }
    } else if (prst->reason == 6) { // External Reset
      rtcData.interval = SLEEP;
      rtcData.counter = 0;
      for (int i = 0; i < sizeof(rtcData); i++) {
        rtcData.data[i] = 0;
      }
    } else  {
      rtcData.counter++;
    }
  }

  // Write data to RTC memory
  if (ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    Serial.println("Write: ");
    printMemory(10);
  }

  
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
  // esp_now_add_peer(uint8 mac_addr, uint8 role, uint8 channel, uint8 key, uint8 key_len)
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  // Set values to send
  long endMillis = millis();
  long diffMillis = endMillis - startMillis;
  strcpy(myData.topic, MQTT_TOPIC); // "/mqtt/espnow"
  //sprintf(myData.payload,"%d %d %d %d %d",rtcData.counter,ESP.getVcc(),prst->reason,SLEEP,diffMillis);
  sprintf(myData.payload,"%d espnow %d %d %d",rtcData.counter,ESP.getVcc(),prst->reason,SLEEP);

  // Send message via ESP-NOW
  esp_now_sended = false;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  // Wait until send complete
  while(1) {
    if (esp_now_sended) break;
    delay(1);
  }
  
  // Goto DEEP SLEEP
  Serial.println("DEEP SLEEP START!!");
  uint32_t time_us = SLEEP * 1000 * 1000;
  ESP.deepSleep(time_us, WAKE_RF_DEFAULT);
  
}

void loop() {
}
