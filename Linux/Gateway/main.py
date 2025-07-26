#!/usr/bin/python3
# -*- coding: utf-8 -*-

from ESPythoNOW import *
import time
import argparse
import paho.mqtt.client as mqtt


def on_connect(mqttc, obj, flags, reason_code, properties):
	print("on_connect reason_code: " + str(reason_code))

def on_message(mqttc, obj, msg):
	print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))

def on_publish(mqttc, obj, mid, reason_code, properties):
	print("on_publish mid: " + str(mid))

def on_log(mqttc, obj, level, string):
	print(string)

def callback(from_mac, to_mac, msg):
	#print(type(msg))
	#print(msg)

	list = []
	for i in range(64):
		#print(i)
		if msg[i] == 0: break
		list.append(chr(msg[i]))
		#print(chr(msg[i]))
	#print(list)
	topic = "".join(list)

	list = []
	for i in range(64):
		#print(i+64)
		if msg[i+64] == 0: break
		list.append(chr(msg[i+64]))
		#print(chr(msg[i+64]))
	#print(list)
	payload = "".join(list)
	print("topic=[{}] payload=[{}]".format(topic, payload))
	print("topic=[{}] payload=[{}]".format(len(topic), len(payload)))
	infot = mqttc.publish(topic, payload)
	infot.wait_for_publish()

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('--host', help="mqtt host to connect to", default="broker.emqx.io")
	parser.add_argument('--port', type=int, help="mqtt port to connect to", default=1883)
	parser.add_argument('--interface', required=True, help="espnow interface")
	args = parser.parse_args()
	print("host={}".format(args.host))
	print("port={}".format(args.port))
	print("interface={}".format(args.interface))

	espnow = ESPythoNow(interface=args.interface, callback=callback)
	espnow.start()

	mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
	mqttc.on_message = on_message
	mqttc.on_connect = on_connect
	mqttc.on_publish = on_publish
	# Uncomment to enable debug messages
	# mqttc.on_log = on_log
	mqttc.connect("broker.emqx.io", 1883, 60)

	mqttc.loop_start()

	while True:
		#infot = mqttc.publish("/mqtt/espnow", "bar")
		#infot.wait_for_publish()
		time.sleep(10)
