# -*- coding:utf-8 -*-
import paho.mqtt
import paho.mqtt.client as mqtt
import random
import argparse
import datetime

def on_connect(client, userdata, flags, respons_code):
	print('connect status:{}'.format(respons_code))
	client.subscribe(args.topic)

def on_message(client, userdata, msg):
	#print("{}:{}".format(msg.topic, msg.payload))
	dt_now = datetime.datetime.now()
	now = dt_now.strftime('%Y%m%d,%H%M%S')
	record = "{},{},{}".format(now, msg.topic, msg.payload.decode('utf-8'))
	#print("{},{},{}".format(now, msg.topic, msg.payload.decode('utf-8')))
	if (args.debug): print(record)
	with open(args.file, 'a') as f:
		print(record, file=f)

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('--host', help="mqtt host to connect to", default="broker.emqx.io")
	parser.add_argument('--port', type=int, help="mqtt port to connect to", default=1883)
	parser.add_argument('--topic', required=True, help="mqtt subscribe topic")
	parser.add_argument('--file', help="output file", default="mqtt.csv")
	parser.add_argument('--debug', action='store_true', help="enable debug print")
	args = parser.parse_args()
	print("host={}".format(args.host))
	print("port={}".format(args.port))
	print("topic={}".format(args.topic))
	print("file={}".format(args.file))
	print("debug={}".format(args.debug))

	print("paho.mqtt.__version__={}".format(paho.mqtt.__version__))
	client_id = f'subscribe-{random.randint(0, 100)}'
	#client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
	client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id)
	client.on_connect = on_connect
	client.on_message = on_message
	client.connect(args.host, port=args.port, keepalive=60)
	client.loop_forever()
