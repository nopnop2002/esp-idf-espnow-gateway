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

# Forecast of Battery life
The ESP12E can wake up from deep sleep with a voltage of 2.5V.   
It can run for 3 months with 2 AA batteries.   
![スライド3](https://user-images.githubusercontent.com/6020549/104807345-ae844080-5821-11eb-97f0-07a1cac5a000.JPG)

The ESP12S/07S can wake up from deep sleep with a voltage of 2.2V.   
It can run for 4 months with 2 AA batteries.   
![スライド4](https://user-images.githubusercontent.com/6020549/104807347-b7751200-5821-11eb-94fa-5878262d4f1e.JPG)


---

# Installation   

```
git clone https://github.com/nopnop2002/esp-idf-espnow-gateway
cd esp-idf-espnow-gateway
idf.py set-target esp32
idf.py menuconfig
idf.py flash monitor
```

---

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

![config-app](https://user-images.githubusercontent.com/6020549/102085238-b2572880-3e59-11eb-94b0-1d73f058f3bc.jpg)

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
Publish every 2 seconds.   
```
$ mosquitto_sub -v -h 192.168.10.40 -p 1883  -t "/mqtt/espnow" | ts "%y/%m/%d %H:%M:%S"
21/08/11 07:36:41 /mqtt/espnow Hello 2001 3093
21/08/11 07:36:43 /mqtt/espnow Hello 4002 3093
21/08/11 07:36:45 /mqtt/espnow Hello 6003 3093
21/08/11 07:36:47 /mqtt/espnow Hello 8004 3093
21/08/11 07:36:49 /mqtt/espnow Hello 10005 3093
21/08/11 07:36:51 /mqtt/espnow Hello 12006 3093
21/08/11 07:36:53 /mqtt/espnow Hello 14007 3093
```

- espnow-controller-deepSleep   
Wake up from Deep Sleep every 60 seconds and publish.   
You need to connect Resets and GPIO16.   
```
$ mosquitto_sub -v -h 192.168.10.40 -p 1883  -t "/deepsleep/espnow" | ts "%y/%m/%d %H:%M:%S"
21/08/11 07:28:01 /deepsleep/espnow 0 0[Msec] 3098[V] 60[Sec]
21/08/11 07:29:01 /deepsleep/espnow 1 5[Msec] 3098[V] 60[Sec]
21/08/11 07:30:01 /deepsleep/espnow 2 5[Msec] 3098[V] 60[Sec]
21/08/11 07:31:01 /deepsleep/espnow 3 4[Msec] 3098[V] 60[Sec]
21/08/11 07:32:01 /deepsleep/espnow 4 4[Msec] 3098[V] 60[Sec]
```
