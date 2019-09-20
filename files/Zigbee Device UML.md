```sequence
Title:Add ZigBee3.0 Device
Platform->Gateway:add (topic[ZB3_DeviceOp_30ae7b16ff32/_Exe])
Gateway->Gateway:searching Zigbee3.0 Device
Device->Gateway:new Device added to Gateway
Gateway->Platform:Status (topic[PLATFORM/Set])
```



1. ZB3 添加设备广播:

```C

Platform --> GW: topic [ZB3_OpDevice_30ae7b162b19/_Exe]
{
	"res":"100",
	"msg":{
		"tid":"ZB3_DeviceOp_30ae7b162b19",

​		"fnl":[
​			{
​				"fn":"Add",
​				"tp":"e",
​				"vl":"ffffffffffffffff"
​			}
​		]
​	
​	}
}
```



2. 网关成功添加ZB3.0设备，上线设备不止一个:

GW --> Platform: topic [PLATFORM/Set]:

```c

{
	"req":"Set",
	"msg":{
		"tid":"ZB3_DoorSensor_000000000000001"
		"fnl":[
			{
				"fn":"Battery",
				"tp":"s",
				"vl":100
			},
			{
				"fn":"Version",
				"tp":"s",
				"vl":"1.0"
			}，
			{
				"fn":"Model",
				"tp":"s",
				"vl":"DoorSensor"
			},
			{
				"fn":"Online",
				"tp":"s",
				"vl":1
			}
		]
	}
}

```



```c
{
	"req":"Set",
	"msg":{
		"tid":"ZB3_DoorSensor_000000000000002"
		"fnl":[
			{
				"fn":"Battery",
				"tp":"s",
				"vl":100
			},
			{
				"fn":"Version",
				"tp":"s",
				"vl":"1.0"
			}，
			{
				"fn":"Model",
				"tp":"s",
				"vl":"DoorSensor"
			},
			{
				"fn":"Online",
				"tp":"s",
				"vl":1
			}
		]
	}
}
```