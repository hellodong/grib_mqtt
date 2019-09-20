

import paho.mqtt.client as mqtt
import json

platExe = "PLATFORM/Exe"
devSwitch_Id = "ZB3_Switch_14b457fffe83841a/_Set"
devSmartPlug_Id = "ZB3_SmartPlug_14b457fffe7f96a3/_Set"
devDoorLock_Id = "ZB3_DoorLock_00158d0001b65c71/_Set"

smartPlugExe = '{"req":"Exe", "msg": {"tid":"ZB3_SmartPlug_14b457fffe7f96a3", "fnl": [ {"fn":"Onoff", "tp":"e", "vl":"0"}] } }'
switch1Exe = '{"req":"Exe", "msg": {"tid":"ZB3_Switch_14b457fffe83841a", "fnl": [ {"fn":"Onoff1", "tp":"e", "vl":"0"}] } }'
switch2Exe = '{"req":"Exe", "msg": {"tid":"ZB3_Switch_14b457fffe83841a", "fnl": [ {"fn":"Onoff2", "tp":"e", "vl":"0"}] } }'
passwdAddExe = '{"req":"Exe", "msg": {"tid":"ZB3_DoorLock_00158d0001b65c71", "fnl": [ {"fn":"PasswdAdd", "tp":"e", "vl":"00100001e2400600000e10"}] } }'



# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    if 0 == rc:
        client.subscribe(devSmartPlug_Id, 1)
        client.subscribe(devSwitch_Id, 1)
        client.subscribe(devDoorLock_Id, 1)
        client.publish(platExe, str(passwdAddExe), 1)
        #client.publish(platExe, str(switch1Exe), 1)
        #client.publish(platExe, str(switch2Exe), 1)
        print(str(passwdAddExe))


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
