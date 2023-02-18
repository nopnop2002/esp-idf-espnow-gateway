# esp-idf-espnow-gateway

Gateway between esp-now and MQTT using esp-idf.   

I was inspired by [this](https://github.com/m1cr0lab-esp32/esp-now-network-and-wifi-gateway).

![スライド1](https://user-images.githubusercontent.com/6020549/102082642-a8332b00-3e55-11eb-827c-a12c505c0b10.JPG)

ESP-NOW can be used with ESP8266/8285, but we cannot use WiFi at the same time.   
This project transfers the data received by ESP-NOW to MQTT.   

# Battery life
ESP8266/8285 + battery + ESP-NOW + DeelSpeep enables long-time operation.   
This is a comparison of battery consumption between Wifi and ESP-NOW.   

![スライド1](https://user-images.githubusercontent.com/6020549/104807005-d0c88f00-581e-11eb-9769-a252cefef043.JPG)

![スライド2](https://user-images.githubusercontent.com/6020549/104807010-d9b96080-581e-11eb-8265-82b2fa0450f3.JPG)

# Battery life after 112 days using ESP-NOW   
![スライド1](https://user-images.githubusercontent.com/6020549/155865309-e554042d-b605-4b3c-9a5b-6d6a881666a8.JPG)

# Software requirements
ESP-IDF V4.4/V5.0.   

# Installation   

```
git clone https://github.com/nopnop2002/esp-idf-espnow-gateway
cd esp-idf-espnow-gateway
idf.py set-target esp32
idf.py menuconfig
idf.py flash monitor
```

# Configuration   
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

![config-app](https://user-images.githubusercontent.com/6020549/219843372-dda3a668-082d-46dd-8c98-14c057c1e46c.jpg)

---

# ESP8266 Example Sketch
There is two example.   
Replace the remote MAC address with your ESP32 MAC address.

```
uint8_t remoteDevice[] = {0x24, 0x0a, 0xc4, 0xef, 0xaa, 0x65};
```

When you run this project on the ESP32, you will see ESP32 MAC address:   
![mac](https://user-images.githubusercontent.com/6020549/102291484-8dac9f00-3f86-11eb-804a-d06e7e813e02.jpg)

- espnow-controller   
Publish every 10 seconds.   
I used broker.emqx.io as MQTT broker.   
```
$ mosquitto_sub -v -h broker.emqx.io -p 1883  -t "/mqtt/espnow" | ts "%y/%m/%d %H:%M:%S"
21/08/11 16:52:25 /mqtt/espnow Hello 10001 2968
21/08/11 16:52:35 /mqtt/espnow Hello 20002 2966
21/08/11 16:52:45 /mqtt/espnow Hello 30003 2966
21/08/11 16:52:55 /mqtt/espnow Hello 40004 2965
21/08/11 16:53:05 /mqtt/espnow Hello 50005 2966
21/08/11 16:53:15 /mqtt/espnow Hello 60006 2965
```

- espnow-controller-deepSleep   
Wake up from Deep Sleep every 60 seconds and publish.   
You need to connect Resets and GPIO16.   
I used broker.emqx.io as MQTT broker.   

```
$ mosquitto_sub -v -h broker.emqx.io -p 1883  -t "/mqtt/espnow" | ts "%y/%m/%d %H:%M:%S"
21/08/11 16:40:32 /mqtt/espnow Hello 66 2904
21/08/11 16:41:30 /mqtt/espnow Hello 68 2908
21/08/11 16:42:29 /mqtt/espnow Hello 67 2908
21/08/11 16:43:29 /mqtt/espnow Hello 67 2907
21/08/11 16:44:28 /mqtt/espnow Hello 68 2911
21/08/11 16:45:27 /mqtt/espnow Hello 67 2906
```
