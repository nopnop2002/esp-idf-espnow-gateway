# esp-idf-espnow-gateway

Gateway between esp-now and MQTT using esp-idf.   

I was inspired by [this](https://github.com/m1cr0lab-esp32/esp-now-network-and-wifi-gateway).

![スライド1](https://user-images.githubusercontent.com/6020549/102082642-a8332b00-3e55-11eb-827c-a12c505c0b10.JPG)

ESP-NOW can be used with ESP8266/8285, but we cannot use WiFi at the same time.   
But with ESP32, we can use ESP-NOW and WiFi at the same time.   
This project transfers the data received by ESP-NOW to MQTT.   

# Battery life
ESP8266/8285 + battery + ESP-NOW + DeelSpeep enables long-time operation.   
This is a comparison of battery consumption between Wifi and ESP-NOW.   

![スライド1](https://user-images.githubusercontent.com/6020549/104807005-d0c88f00-581e-11eb-9769-a252cefef043.JPG)

![スライド2](https://user-images.githubusercontent.com/6020549/104807010-d9b96080-581e-11eb-8265-82b2fa0450f3.JPG)

# Battery life after 112 days using ESP-NOW   
This is the measurement result when the DeepSleep interval was ```600 seconds```.   
Two AA batteries will give you about four months of operation.   
![スライド1](https://user-images.githubusercontent.com/6020549/155865309-e554042d-b605-4b3c-9a5b-6d6a881666a8.JPG)

# Using NodeMCU Development board   
![NodeMcu](https://github.com/user-attachments/assets/3233df6e-7c2f-4cee-94dd-544fe4f594f2)   
![NodeMcu-2](https://github.com/user-attachments/assets/2bb9e8f7-a726-42a7-9f11-0f3ab890badc)   
This is the measurement result when the DeepSleep interval was ```60 seconds.```   
Two AA batteries will give you about one months of operation.   
![Chart](https://github.com/user-attachments/assets/e4f870b3-636f-4e48-aa91-2bb6ede0f491)

# Software requirements   
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   
ESP-IDF V5.1 is required when using ESP32-C6.   

# Hardware requirements   
- ESP8266/8285.   
	Some development boards have a Power LED.   
	Some development boards have a Serail TX signal LED.   
	They consume battery.   
	Since the ESP12 has an LED on the module, it is not suitable for battery operation.   
	Care should be taken when selecting hardware to reduce battery consumption.   
	Some WeMos D1 Mini clones cannot be started on two AA batteries.   

# Installation   

```
git clone https://github.com/nopnop2002/esp-idf-espnow-gateway
cd esp-idf-espnow-gateway
idf.py set-target {esp32/esp32s2/esp32s3/esp32c2/esp32c3/esp32c6}
idf.py menuconfig
idf.py flash monitor
```

# Configuration   
![Image](https://github.com/user-attachments/assets/cb29c75b-6743-42a7-80b0-abb95d972abf)
![Image](https://github.com/user-attachments/assets/f5a0a948-fae8-4966-95e1-c9938af1b406)

## WiFi Setting
Set the information of your access point.   
![Image](https://github.com/user-attachments/assets/08f9f659-b3f4-4bdc-b782-4ab10cbe9cb5)

## Broker Setting
Set the information of your MQTT broker.   
![Image](https://github.com/user-attachments/assets/71a4ec4e-87c8-49c8-96a6-5b1711ee34c1)

### Select Transport   
This project supports TCP,SSL/TLS,WebSocket and WebSocket Secure Port.   

- Using TCP Port.   
	TCP Port uses the MQTT protocol.   

- Using SSL/TLS Port.   
	SSL/TLS Port uses the MQTTS protocol instead of the MQTT protocol.   

- Using WebSocket Port.   
	WebSocket Port uses the WS protocol instead of the MQTT protocol.   

- Using WebSocket Secure Port.   
	WebSocket Secure Port uses the WSS protocol instead of the MQTT protocol.   

__Note for using secure port.__   
The default MQTT server is ```broker.emqx.io```.   
If you use a different server, you will need to modify ```getpem.sh``` to run.   
```
chmod 777 getpem.sh
./getpem.sh
```
### Specifying an MQTT Broker   
You can specify your MQTT broker in one of the following ways:   
- IP address   
 ```192.168.10.20```   
- mDNS host name   
 ```mqtt-broker.local```   
- Fully Qualified Domain Name   
 ```broker.emqx.io```

You can use this as broker.   
https://github.com/nopnop2002/esp-idf-mqtt-broker

### Select MQTT Protocol   
This project supports MQTT Protocol V3.1.1/V5.   
![Image](https://github.com/user-attachments/assets/a6898216-03e6-49b3-b482-c49a007dd373)

### Enable Secure Option   
Specifies the username and password if the server requires a password when connecting.   
[Here's](https://www.digitalocean.com/community/tutorials/how-to-install-and-secure-the-mosquitto-mqtt-messaging-broker-on-debian-10) how to install and secure the Mosquitto MQTT messaging broker on Debian 10.   
![Image](https://github.com/user-attachments/assets/5990c9aa-8fa9-4c96-8631-fc0260c62704)

---

# ESP8266 Example Sketch
There is two example.   
Follow [this](https://github.com/nopnop2002/esp-idf-espnow-gateway/tree/main/esp8266) page to change your settings.   

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
	You need to connect reset pin and GPIO16 with a wire cable.   
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

# Multiple ESP8266   
ESPNOW allows one-to-many communication.   
This is logging when two ESP8266s are operated at the same time.   
```
23/11/15 16:17:55 /mqtt/espnow Hello 410041 2965 --> This is from ESP8266 #1
23/11/15 16:18:04 /mqtt/espnow Hello 10001 2981 --> This is from ESP8266 #2
23/11/15 16:18:05 /mqtt/espnow Hello 420042 2965
23/11/15 16:18:14 /mqtt/espnow Hello 20002 2976
23/11/15 16:18:15 /mqtt/espnow Hello 430043 2965
23/11/15 16:18:24 /mqtt/espnow Hello 30003 2976
23/11/15 16:18:25 /mqtt/espnow Hello 440044 2965
23/11/15 16:18:34 /mqtt/espnow Hello 40004 2976
23/11/15 16:18:35 /mqtt/espnow Hello 450045 2965
23/11/15 16:18:44 /mqtt/espnow Hello 50005 2973
23/11/15 16:18:45 /mqtt/espnow Hello 460046 2965
23/11/15 16:18:54 /mqtt/espnow Hello 60006 2977
23/11/15 16:18:55 /mqtt/espnow Hello 470047 2965
23/11/15 16:19:04 /mqtt/espnow Hello 70007 2974
23/11/15 16:19:05 /mqtt/espnow Hello 480048 2965
23/11/15 16:19:14 /mqtt/espnow Hello 80008 2976
23/11/15 16:19:15 /mqtt/espnow Hello 490049 2965
23/11/15 16:19:24 /mqtt/espnow Hello 90009 2972
23/11/15 16:19:25 /mqtt/espnow Hello 500050 2965
23/11/15 16:19:34 /mqtt/espnow Hello 100010 2973
```
