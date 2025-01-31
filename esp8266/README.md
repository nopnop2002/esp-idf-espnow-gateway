# Software requirements   
Arduino core for ESP8266.   
Use Arduino-IDE or PlatformIO.   

# Configuration
When you run this project on your ESP32, you will see the ESP32's MAC address and the WiFi channel it is using.   
![Image](https://github.com/user-attachments/assets/8ce882d2-1e52-47fd-8eed-81ef655390d1)

## MAC address of ESP32   
Replace the remote MAC address with your ESP32 MAC address.
```
uint8_t remoteDevice[] = {0xA4, 0xCF, 0x12, 0x05, 0xC6, 0x35};
```

## WiFi channel   
Replace the WiFi channel with your ESP32 WiFi channel.
```
uint8 remoteChannel = 11;
```

## MQTT topic
Replace the mqtt topic to publish.
```
#define MQTT_TOPIC "/mqtt/espnow"
```

# Using PlatformIO
```
$ git clone https://github.com/nopnop2002/esp-idf-espnow-gateway

$ cd esp-idf-espnow-gateway/esp8266/espnow-controller

$ pio init -b d1_mini

$ cp espnow-controller.ino src/

$ pio run -t upload && pio device monitor -b 115200
```
