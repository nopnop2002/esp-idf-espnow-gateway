# Software requirements   
Arduino core for ESP8266.   
Use Arduino-IDE or PlatformIO.   

# Configuration
When you run this project on the ESP32, you will see ESP32 MAC address and WiFi channel:   
![Image](https://github.com/user-attachments/assets/c8f1d109-2512-422e-b65f-d8ee0e996173)

## MAC address of ESP32   
Replace the remote MAC address with your ESP32 MAC address.
```
uint8_t remoteDevice[] = {0x24, 0x0a, 0xc4, 0xef, 0xaa, 0x65};
```

## WiFi channe   
Replace the WiFi Channel with your ESP32 WiFi channel.
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
