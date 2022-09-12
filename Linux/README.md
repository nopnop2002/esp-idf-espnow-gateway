# Linux version of Gateway

I ported from [here](https://github.com/thomasfla/Linux-ESPNOW).   
I tried it on Ubuntu 20.04.   

## Insert a USB-WiFi dongle into your Linux machine and use the following command to see if the driver supports it.
```
$ nmcli device wifi
*  SSID                MODE   CHAN  RATE       SIGNAL  BARS  SECURITY
   ap-wpsG-a8bf10      Infra  1     54 Mbit/s  89      ????  WPA2
   Picking             Infra  1     54 Mbit/s  50      ??__  WPA2
   88888888            Infra  11    54 Mbit/s  40      ??__  WPA2
   inumber             Infra  1     54 Mbit/s  44      ??__  WPA2
   aterm-e625c0-gw     Infra  1     54 Mbit/s  100     ????  WEP
   ESP_1B06BB          Infra  1     54 Mbit/s  100     ????  --
   ESP_011834          Infra  1     54 Mbit/s  70      ???_  --
   ESP_CCE2F9          Infra  1     54 Mbit/s  57      ???_  --
```

## Use the following command to find the device name and MAC address of your WiFi device.   
__wlx1cbfceaae44d__ is the device name.   
__1c:bf:ce:aa:e4:4d__ is the MAC address.   
```

$ iwconfig
lo        no wireless extensions.

enp2s0    no wireless extensions.

wlx1cbfceaae44d  IEEE 802.11  ESSID:off/any
          Mode:Managed  Access Point: Not-Associated   Tx-Power=20 dBm
          Retry short limit:7   RTS thr:off   Fragment thr:off
          Power Management:off


$ sudo ifconfig wlx1cbfceaae44d
wlx1cbfceaae44d: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        ether 1c:bf:ce:aa:e4:4d  txqueuelen 1000  (イーサネット)
        RX packets 0  bytes 0 (0.0 B)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

## Change your WiFi device to monitor mode.   
```
$ sudo ifconfig wlx1cbfceaae44d down

$ sudo iwconfig wlx1cbfceaae44d mode monitor

$ sudo ifconfig wlx1cbfceaae44d up

$ sudo ifconfig wlx1cbfceaae44d
wlx1cbfceaae44d: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
        unspec 1C-BF-CE-AA-E4-4D-00-00-00-00-00-00-00-00-00-00  txqueuelen 1000  (不明なネット)
        RX packets 2412317  bytes 380570756 (380.5 MB)
        RX errors 0  dropped 109773  overruns 0  frame 0
        TX packets 0  bytes 0 (0.0 B)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```

You can now use the ESP-NOW protocol.   
Clone the repository from github and compile it.   
Specify the name of the WiFi device in the run-time argument.

## Install MQTT C Client library.   
```
$ git clone https://github.com/eclipse/paho.mqtt.c
$ cd paho.mqtt.c/
$ make
$ sudo make install
```

## Install espnow gateway for linux.   
```
$ git clone https://github.com/nopnop2002/esp-idf-espnow-gateway

$ cd esp-idf-espnow-gateway/Linux

$ make
mkdir -p bin
gcc main.c -Wall -o bin/receiver

$ sudo ./bin/receiver wlx1cbfceaae44d

 Waiting to receive packets ........
myData.topic=[/mqtt/espnow]
myData.payload=[Hello 738369 3089]
Waiting for up to 10 seconds for publication
on topic /mqtt/espnow for client with ClientID: ExampleClientPub
Message with delivery token 1 delivered
myData.topic=[/mqtt/espnow]
myData.payload=[Hello 740370 3089]
Waiting for up to 10 seconds for publication
on topic /mqtt/espnow for client with ClientID: ExampleClientPub
Message with delivery token 2 delivered
myData.topic=[/mqtt/espnow]
myData.payload=[Hello 742371 3089]
Waiting for up to 10 seconds for publication
on topic /mqtt/espnow for client with ClientID: ExampleClientPub
Message with delivery token 3 delivered

```

# ESP8266 Example Sketch
Replace the remote MAC address with your Linux MAC address.
```
//uint8_t remoteDevice[] = {0x24, 0x0a, 0xc4, 0xef, 0xaa, 0x65};
uint8_t remoteDevice[] = {0x1c, 0xbf, 0xce, 0xaa, 0xe4, 0x4d};
```
