# Linux version of Sender
You can send ESPNOW packets on your Linux PC.   
I based this off of [this](https://github.com/thomasfla/Linux-ESPNOW/tree/master/wifiRawSender).   
I tried it on Ubuntu 22.04.   

# Hardware setting   

- Insert a USB-WiFi dongle into your Linux machine and use the following command to see if the driver supports it.
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

- Use the following command to find the device name and MAC address of your WiFi device.   
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
	        ether 1c:bf:ce:aa:e4:4d  txqueuelen 1000  (Ethernet)
	        RX packets 0  bytes 0 (0.0 B)
	        RX errors 0  dropped 0  overruns 0  frame 0
	        TX packets 0  bytes 0 (0.0 B)
	        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
	```

- Change your WiFi device to monitor mode.   
	```
	$ sudo ifconfig wlx1cbfceaae44d down

	$ sudo iwconfig wlx1cbfceaae44d mode monitor

	$ sudo ifconfig wlx1cbfceaae44d up
	```

- Specify the WiFi channel you want to use.   
	The WiFi channel must match the ESP32.   
	```
	$ sudo iwconfig wlx1cbfceaae44d channel 11

	$ sudo ifconfig wlx1cbfceaae44d
	wlx1cbfceaae44d: flags=4099<UP,BROADCAST,MULTICAST>  mtu 1500
	        unspec 1C-BF-CE-AA-E4-4D-00-00-00-00-00-00-00-00-00-00  txqueuelen 1000  (UNSPEC)
	        RX packets 19666  bytes 4191675 (4.1 MB)
	        RX errors 0  dropped 2390  overruns 0  frame 0
	        TX packets 0  bytes 0 (0.0 B)
	        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
	```

	You can now use the ESP-NOW protocol.   

# Start ESPNOW on ESP32
ESPNOW in ESP-IDF V5.3 is Version 1.0, but from ESP-IDF V5.4, ESPNOW has become Version 2.0.   
In Version 2.0, the maximum data length has been extended from 250 (ESP_NOW_MAX_DATA_LEN) bytes to 1470 (ESP_NOW_MAX_DATA_LEN_V2) bytes.   
```
cd $IDF_PATH/examples/wifi/espnow/
idf.py menuconfig
idf.py flash monitor
```

# Install for linux   
```
$ cd $HOME
$ git clone https://github.com/nopnop2002/esp-idf-espnow-gateway
$ cd esp-idf-espnow-gateway/Linux/Sender
$ make
mkdir -p bin
gcc main.c -Wall -o bin/sender
$ sudo ./bin/sender wlx1cbfceaae44d

Sending data using raw socket over wlx1cbfceaae44d
0 0 18 0 2e 40 0 a0 20 8 0 0 0 2 6c 9 a0 0 db 0 0 0 db 0 d0 0 0 0 ff ff ff ff ff ff 8 3a f2 50 de 5c ff ff ff ff ff ff 0 0 7f 18 fe 34 18 f4 56 a6 dd ff 18 fe 34 4 1 0 0 0 0 ff 7c 94 aa 29 23 c9 f2 81 83 87 a9 9f 7a 23 39 78 49 35 4 48 e8 49 43 2a ad 5c 79 dd 8a 9 6a b 2d 7e 3a 50 63 15 b3 d8 48 d d4 19 e5 d1 10 7d 78 78 e7 49 94 3b 7a f9 95 aa ad 40 cc d1 c4 65 37 8e 89 9f 16 56 31 76 de ba 41 dc 6e ea c8 b8 8b c1 7 20 3d ce 43 8d ab dd 26 ef d7 9 e3 bb 6c 7 1c 8b 6f 6e b1 fd e1 f1 40 21 af 8d b6 ae fc 7c 92 71 63 10 96 b0 97 e0 a7 a1 23 f5 24 45 16 12 14 e8 a 4a e1 25 0 ce 6b d6 77 ba b4 c1 f3 47 51 c6 32 5e 15 cb ca f9 77 ca 19 48 8e cf 93 79 9b 9e 8d b b3 4c cb e2 14 21 1a 22 2c 7a 34 d9 7 2b d6 30 e5 4 85 42 61 d6 ea 69 d1 e2 21 a9 6 9d 67 bb b7 35 d1 52 f0 63 9e 8a af 28 f7 78 bb e0 d 87 36 73 40 8f 7f de 95 57 3b 98 e 63 60 b5 5c 59 bd 48 14 58 99 20 d9 f6 5a b0 18 75 e0 76 e7 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
```

