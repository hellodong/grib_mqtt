## Platform ZegBee3.0 Protocol 


### Device List
| Device | tid | Fn | explain|
| ---- | ---- | ---- | ---- |
| DoorSensor | ZB3_DoorSensor_xxxxx | Version<br>Battery<br>Online<br>DoorStatus<br>|DoorStatus:<br> 0 -- closed, 1 -- opened|
| LeakSensor| ZB3_LeakSensor_xxxxx |Version<br/>Battery<br/>Online<br/>LeakStatus<br/>|LeakStatus:<br>0 -- 没水， 1 -- 有水|
| PIRSensor | ZB3_PIRSensor_xxxxxx |Version<br/>Battery<br/>Online<br/>PIRStatus<br/>|PIRStatus:<br>0 -- 没人， 1 -- 有人|


###  Function list
| fn             |explain |tp   |
| ----- |----|---- |
| Version | 软件版本 | s |
|Battery|电池电量(0 ~ 100)|s|
| Online | 是否在线(0 -- 在线，1 -- 离线) | s |
|DoorStatus|门磁状态|s|
|LeakStatus|液漏状态|s|
|PIRStatus|人体红外状态|s|

### Function Json


1. Fn[**Version**]:  

Gateway --> Platform, gateway publish topic:**PLATFORM/Set**

```c
{
	"req":"Set",
	"msg": {
		"tid": "ZB3_XXXSensor_00158d00000000",
		"fnl": 
		[
			{
				"fn":"Version",
				"tp":"s",
				"vl": "1.0"
			}
		]
	}
}
```



2. Fn[**Battery**]:

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**

```c
{
	"req":"Set",
	"msg": {
		"tid":"ZB3_XXXSensor_00158d00000000",
		"fnl": {
			"fn": "Battery",
            "tp": "s",
			"vl": "100",
		},
	}
}
```

3. Fn[**Online**]:

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**

```c
{
	"req":"Set",
	"msg": {
		"tid":"ZB3_XXXSensor_00158d00000000",
		"fnl": {
			"fn": "Online",
            "tp": "s",
			"vl": "1",
		},
	}
}
```
4. Fn[**DoorStatus**]
Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**:
```c
{
	"req":"Set",
	"msg": {
		"tid":"ZB3_DoorSensor_00158d00000000",
		"fnl": {
			"fn": "DoorStatus",
            "tp": "s",
			"vl": "1",
		},
	}
}
```
5. Fn[**LeakStatus**]
Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**:
```c
{
	"req":"Set",
	"msg": {
		"tid":"ZB3_LeakSensor_00158d00000000",
		"fnl": {
			"fn": "LeakStatus",
            "tp": "s",
			"vl": "1",
		}
	}
}
```
6. Fn[**PIRStatus**]
 Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**
 
 ```c
 {
 	"req":"Set",
 	"msg": {
 		"tid":"ZB3_PIRSensor_00158d00000000",
 		"fnl": {
 			"fn": "PIRStatus",
             "tp": "s",
 			"vl": "1",
 		},
 	}
 }
 ```




