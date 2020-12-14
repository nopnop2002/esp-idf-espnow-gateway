# esp-idf-espnow-gateway

Gateway between esp-now and MQTT using esp-idf.   

I was inspired by [this](https://github.com/m1cr0lab-esp32/esp-now-network-and-wifi-gateway).

![ƒXƒ‰ƒCƒh1](https://user-images.githubusercontent.com/6020549/102082642-a8332b00-3e55-11eb-827c-a12c505c0b10.JPG)

ESP-NOW can be used with ESP8266/8285, but we cannot use WiFi at the same time.   
This project transfers the data received by ESP-NOW to the MQTT server.   
ESP8266/8285 + battery + ESP-NOW + DeelSpeep enables long-time operation.   

---

# Install

```
git clone https://github.com/nopnop2002/esp-idf-espnow-gateway
cd esp-idf-espnow-gateway
make menuconfig
make flash monitor
```

---

# Firmware configuration
You have to set this config value using menuconfig.   

- CONFIG_STA_WIFI_SSID   
SSID of your wifi.
- CONFIG_STA_WIFI_PASSWORD   
PASSWORD of your wifi.
- CONFIG_STA_MAXIMUM_RETRY   
Maximum number of retries when connecting to wifi.
- CONFIG_BROKER_URL   
URL of MQTT broker.
- CONFIG_ESPNOW_ENABLE_LONG_RANGE   
Enable long-range ESP-NOW.   
When enable long range, the PHY rate of ESP32 will be 512Kbps or 256Kbps.   

![config-main](https://user-images.githubusercontent.com/6020549/102085230-aff4ce80-3e59-11eb-85f2-f53babe33f0d.jpg)

![config-app](https://user-images.githubusercontent.com/6020549/102085238-b2572880-3e59-11eb-94b0-1d73f058f3bc.jpg)

---

# ESP8266 Example Sketch
Replace with receiver(ESP32) MAC address.   
```
uint8_t broadcastAddress[] = {0x3c, 0x71, 0xbf, 0x4f, 0xc1, 0xa1};
```

![mac](https://user-images.githubusercontent.com/6020549/102086437-70c77d00-3e5b-11eb-855f-b792797afd34.jpg)

- esp-now-controller

- esp-now-controller-deepSleep
