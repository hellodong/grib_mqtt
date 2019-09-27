## Platform ZegBee3.0 Protocol 


### Device List
| Device | tid | Fn | explain|
| ---- | ---- | ---- | ---- |
| DoorSensor | ZB3_DoorSensor_xxxxx | Version<br>Battery<br>Online<br>DoorStatus<br>|DoorStatus:<br> 0 -- closed, 1 -- opened|
| LeakSensor| ZB3_LeakSensor_xxxxx |Version<br/>Battery<br/>Online<br/>LeakStatus<br/>|LeakStatus:<br>0 -- 没水， 1 -- 有水|
| PIRSensor | ZB3_PIRSensor_xxxxxx |Version<br/>Battery<br/>Online<br/>PIRStatus<br/>|PIRStatus:<br>0 -- 没人， 1 -- 有人|
| SmartPlug | ZB3_SmartPlug_xxxxxx |Version<br/>Online<br/>Energy_Kwh<br/>Power_Kw<br/>Current_A<br/>Onoff<br/>|Onoff:<br>0 -- 断开， 1 -- 闭合|
| Power Strip | ZB3_SmartPlug_xxxxxx |Version<br/>Online<br/>Onoff<br/>|Onoff:<br>0 -- 断开， 1 -- 闭合|
| Switch | ZB3_Switch_xxxxxx |Version<br/>Online<br/>Onoff[ep]<br/>|Onoff[ep]:<br>0 -- 断开， 1 -- 闭合|
| DoorLock | ZB3_DoorLock_xxxxx | Version<br> Model<br> Online<br>PasswdAdd<br>PasswdDel<br>|PasswdAdd: 添加密码<br>PasswdDel:删除密码|
| SmokeSensor | ZB3_SmokeSensor_xxxxx | Version<br>Online<br>Battery<br>SmokeStatus | SmoeStatus:<br>1 -- 有烟， 0 -- 无烟 |
| Emergency Call Button | ZB3_EmergCaBtn_xxxx |Version<br>Online<br>Battery<br>EmergCaStatus|EmergCaStatus:<br>1 -- 紧急呼叫， 0 -- 紧急呼叫停止|
| Gas Sensor | ZB3_GasSensor_xxxx |Version<br>Online<br>Battery<br>GasStatus| GasStatus:<br>1 -- 探测到燃气， 0 -- 未探测到燃气 |


###  Function list
| fn             |explain |tp   |
| ----- |----|---- |
| Version | 软件版本 | s |
|Battery|电池电量(0 ~ 100)|s|
| Online | 是否在线(0 -- 在线，1 -- 离线) | s |
|DoorStatus|门磁状态|s|
|LeakStatus|液漏状态|s|
|PIRStatus|人体红外状态|s|
|SmokeStatus| 烟感状态 |s|
|EmergCaStatus |紧急呼叫状态 |s |
|GasStatus | 燃气状态 | s|
|Energy_Kwh|电量计量(KWH)| s |
|Power_Kw|功率(KW)| s |
|Current_A|电流(A)| s |
|Onoff|开关| se |
|Onoff[ep]|开关, <br> Onoff1 -- 开关1， Onoff2 -- 开关2|se |
| PasswdAdd | 添加密码 | se |
| PasswdDel | 删除密码 | se |

### Function vl  Expalin

##### PasswdAdd 

| uint16 | uint32         | uint8                 | uint32      |
| ------ | -------------- | --------------------- | ----------- |
| userId | Password Value | Password Value Length | Duration(s) |

**userId**: 0 ~ 0xff00

**Password Value, Passwod Value Length**: 密码长度只支持6个数字

**Duration**: 密码有效持续时间，unit: second

Example: userId -- 0x0010, PasswdVal -- 0x1e240, PasswdValLen -- 0x06, Duration -- 0xE10

vl string: "00100001e2400600000e10"

##### PasswdDel 

**userId<uint16>**: 0 ~ 0xff00

Example: userId -- 0x0010

vl string: "0010"

### Function Json


1. Fn[**Version**]:  

Gateway --> Platform, gateway publish topic:**PLATFORM/Set**

```c
{
	"req":"Set",
	"msg": {
		"tid": "ZB3_XXX_00158d00000000",
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
		"tid":"ZB3_XXX_00158d00000000",
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
 7. Fn[**SmokeStatus**]
 Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**

 ```c
 {
 	"req":"Set",
 	"msg": {
 		"tid":"ZB3_SmokeSensor_00158d00000000",
 		"fnl": {
 			"fn": "SmokeStatus",
             "tp": "s",
 			"vl": "1",
 		},
 	}
 }
 ```

8. Fn[**EmergCaStatus**]

   Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**

   ```c 
   {
   	"req": "Set",
   	"msg": {
   		"tid": "ZB3_EmergCaBtn_xxxxxxxx",
   		"fnl": [{
   			"fn": "EmergCaStatus",
   			"tp": "s",
   			"vl": "0"
   		}]
   	}
   }
   ```

   

9. Fn[**GasStatus**]

   Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**

   ```c 
   {
   	"req": "Set",
   	"msg": {
   		"tid": "ZB3_GasSensor_xxxxxxx",
   		"fnl": [{
   			"fn": "GasStatus",
   			"tp": "s",
   			"vl": "1"
   		}]
   	}
   }
   ```

   

10. Fn[**Energy_Kwh**]  

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**

```c 
{
	"req":"Set",
	"msg":{
		"tid":"ZB3_SmartPlug_00155f007483a275"
		"fnl":[
			{
				"fn":"Energy_Kwh",
				"tp":"s",
				"vl":"0.000"
			}
		]
	}
}
```



11. Fn[**Power_Kw**]

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set** 

```c 
{
	"req":"Set",
	"msg":{
		"tid":"ZB3_SmartPlug_00155f007483a275"
		"fnl":[
			{
				"fn":"Power_Kw",
				"tp":"s",
				"vl":"0.000"
			}
		]
	}
}
```



12. Fn[**Current_A**]

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set** 

   ```c 
   {
   	"req":"Set",
   	"msg":{
   		"tid":"ZB3_SmartPlug_00155f007483a275"
   		"fnl":[
   			{
   				"fn":"Current_A",
   				"tp":"s",
   				"vl":"1.5"
   			}
   		]
   	}
   }
   ```

   

13. Fn[**Onoff**]

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set** 

```c 
{
	"req": "Set",
	"msg": {
		"tid": "ZB3_SmartPlug_14b457fffe7f96a3",
		"fnl": [{
			"fn": "Onoff",
			"tp": "s",
			"vl": "0"
		}]
	}
}
```

Service --> Platform,Service Publish topic:**PLATFORM/Set**

```c 
{
	"req": "Exe",
	"msg": {
		"tid": "ZB3_SmartPlug_14b457fffe7f96a3",
		"fnl": [{
			"fn": "Onoff",
			"tp": "e",
			"vl": "1"
		}]
	}
}
```

Platform --> Gateway, gateway subscribe topic:**ZB3_SmartPlug_14b457fffe7f96a3/_Exe**

```c 
{
	"req": "Exe",
	"msg": {
		"tid": "ZB3_SmartPlug_14b457fffe7f96a3",
		"fnl": [{
			"fn": "Onoff",
			"tp": "e",
			"vl": "1"
		}]
	}
}
```



14. Fn[**Onoff[ep]**]

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set** 

```c 
{
	"req": "Set",
	"msg": {
		"tid": "ZB3_Switch_14b457fffe83841a",
		"fnl": [{
			"fn": "Onoff1",		//Onoff1 -- Endpoint 1, Onoff2 -- Endpoint 2
			"tp": "s",
			"vl": "1"
		}]
	}
}
```

Service --> Platform, Service publish topic:**PLATFORM/Exe**:

```c
{
	"req": "Exe",
	"msg": {
		"tid": "ZB3_Switch_14b457fffe83841a",
		"fnl": [{
			"fn": "Onoff1",   //Onoff1 -- Endpoint 1, Onoff2 -- Endpoint 2
			"tp": "e",
			"vl": "1"
		}]
	}
}
```

Platform --> Gateway, gateway subscribe topic:**ZB3_Switch_14b457fffe83841a/_Exe**

```c 
{
	"res": "100",
	"msg": {
		"tid": "ZB3_Switch_14b457fffe83841a",
		"fnl": [{
			"fn": "Onoff1",   //Onoff1 -- Endpoint 1, Onoff2 -- Endpoint 2
			"tp": "e",
			"vl": "1"
		}]
	}
}
```

15.Fn[**PasswdAdd**]

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set** 

```c 
{
	"req": "Set",
	"msg": {
		"tid": "ZB3_DoorLock_00158d0001b65c71",
		"fnl": [{
			"fn": "PasswdAdd",	
			"tp": "s",
			"vl": "0"
		}]
	}
}
```

Service --> Platform, Service publish topic:**PLATFORM/Exe**:

```c
{
	"req": "Exe",
	"msg": {
		"tid": "ZB3_DoorLock_00158d0001b65c71",
		"fnl": [{
			"fn": "PasswdAdd",  
			"tp": "e",
			"vl": "00100001e2400600000e10"
		}]
	}
}
```

Platform --> Gateway, gateway subscribe topic:**ZB3_DoorLock_00158d0001b65c71/_Exe**

```c 
{
	"res": "100",
	"msg": {
		"tid": "ZB3_DoorLock_00158d0001b65c71",
		"fnl": [{
			"fn": "PasswdAdd",  
			"tp": "e",
			"vl": "00100001e2400600000e10"
		}]
	}
}
```

16. Fn[**PasswdDel**]

Gateway-->Platform, gateway publish topioc:**PLATFORM/Set** 

```c 
{
	"req": "Set",
	"msg": {
		"tid": "ZB3_DoorLock_00158d0001b65c71",
		"fnl": [{
			"fn": "PasswdDel",	
			"tp": "s",
			"vl": "0"
		}]
	}
}
```

Service --> Platform, Service publish topic:**PLATFORM/Exe**:

```c
{
	"req": "Exe",
	"msg": {
		"tid": "ZB3_DoorLock_00158d0001b65c71",
		"fnl": [{
			"fn": "PasswdDel",  
			"tp": "e",
			"vl": "0010"
		}]
	}
}
```

Platform --> Gateway, gateway subscribe topic:**ZB3_DoorLock_00158d0001b65c71/_Exe**

```c
{
	"res": "100",
	"msg": {
		"tid": "ZB3_DoorLock_00158d0001b65c71",
		"fnl": [{
			"fn": "PasswdDel",  
			"tp": "e",
			"vl": "0010"
		}]
	}
}

```