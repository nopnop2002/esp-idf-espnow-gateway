# Software requirements   
Arduino core for ESP8266.   
Use Arduino-IDE or PlatformIO.   

# MAC address of ESP32   
Replace the remote MAC address with your ESP32 MAC address.

```
uint8_t remoteDevice[] = {0x24, 0x0a, 0xc4, 0xef, 0xaa, 0x65};
```

When you run this project on the ESP32, you will see ESP32 MAC address:   
![mac](https://user-images.githubusercontent.com/6020549/102291484-8dac9f00-3f86-11eb-804a-d06e7e813e02.jpg)

# MQTT topic
Change here:   
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
