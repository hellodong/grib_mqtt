

import paho.mqtt.client as mqtt
import json

platExe = "PLATFORM/Exe"
devId = "GRIB_GATEWAY_30ae7b63f832/_Set"
jsonExe = '{"req":"Exe", "msg": {"tid":"GRIB_GATEWAY_30ae7b63f832", "fnl": [ {"fn":"RemoteSh", "tp":"e", "vl":{"server": "114.215.195.44","port": 3234}}] } }'


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    if 0 == rc:
        print("Subscribe:"+devId)
        client.subscribe(devId, 1)
        client.publish(platExe, str(jsonExe), 1)
        print(str(jsonExe))


# Subscribing in on_connect() means that if we lose the connection and
# reconnect then subscriptions will be renewed.

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))


client = mqtt.Client('30:ae:7b:16:ff:30')
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set('dhkswl', '@65$@');
client.connect('interface.jiutengkj.com', 1883, 120)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
