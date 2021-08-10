#include <ESP8266WiFi.h>
#include <espnow.h>

ADC_MODE(ADC_VCC);

#define SLEEP 60   // 1 minite sleep
//#define SLEEP 600   // 10 minute sleep
#define MQTT_TOPIC "/deepsleep/espnow"

// REPLACE WITH RECEIVER MAC Address
uint8_t remoteDevice[] = {0x24, 0x0a, 0xc4, 0xef, 0xaa, 0x65};
//uint8_t remoteDevice[] = {0x30, 0xae, 0xa4, 0xca, 0xe1, 0x42};

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
  uint32_t elaspeed;
  byte data[500]; // User Data (unused)
} rtcData;

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

// Callback when data is received
void ICACHE_FLASH_ATTR simple_cb(u8 *macaddr, u8 *data, u8 len) {

}

void setup() {
  delay(100);
  long startMillis = millis();
  Serial.begin(115200);
  Serial.println();
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
  esp_now_register_recv_cb(simple_cb);
  
  // Register peer
  // If the channel is set to 0, data will be sent on the current channel. 
  esp_now_add_peer(NULL, ESP_NOW_ROLE_CONTROLLER, 0, NULL, 0);
  //esp_now_add_peer(NULL, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);

  long elapsedMillis = millis() - startMillis;
  Serial.print("elapsedMillis: ");
  Serial.println(elapsedMillis);

  // Read data from RTC memory
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    Serial.println("rtcUserMemoryRead Success");
    if (prst->reason != 5) { // Not REANSON_DEEP_SLEEP_AWAKE
      rtcData.interval = SLEEP;
      rtcData.counter = 0;
      rtcData.elaspeed = 0;
    } else  {
      rtcData.counter++;
      rtcData.elaspeed = rtcData.elaspeed + elapsedMillis;
    }
  } else {
    Serial.println("rtcUserMemoryRead Fail");
  }

  // Write data to RTC memory
  if (ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData))) {
    Serial.println("rtcUserMemoryWrite Success");
  } else {
    Serial.println("rtcUserMemoryWrite Fail");
  }

  long average = 0;
  if (rtcData.counter > 0) {
    average = rtcData.elaspeed / rtcData.counter;
    Serial.print("counter=");
    Serial.print(rtcData.counter);
    Serial.print(" elaspeed=");
    Serial.print(rtcData.elaspeed);
    Serial.print(" average=");
    Serial.println(average);
  }

  // Set values to send
  strcpy(myData.topic, MQTT_TOPIC); // "/mqtt/espnow"
  sprintf(myData.payload,"%d %d[Msec] %d[V] %d[Sec]",rtcData.counter,average,ESP.getVcc(),SLEEP);

  // Send message via ESP-NOW
  esp_now_sended = false;
  esp_now_send(remoteDevice, (uint8_t *) &myData, sizeof(myData));

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
