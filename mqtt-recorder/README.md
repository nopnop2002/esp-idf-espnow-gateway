# mqtt-recorder
A script that simply adds a specific topic to a file.   
paho-mqtt library V2.x is required.   


```
$ python3 -m pip install paho-mqtt
$ pip list | grep paho-mqtt
paho-mqtt                    2.1.0
$ cd mqtt-recorder
$ python3 mqtt-recorder.py --help
usage: mqtt-recorder.py [-h] [--host HOST] [--port PORT] --topic TOPIC [--file FILE] [--debug]

options:
  -h, --help     show this help message and exit
  --host HOST    mqtt host to connect to
  --port PORT    mqtt port to connect to
  --topic TOPIC  mqtt subscribe topic
  --file FILE    output file
  --debug        enable debug print
```
