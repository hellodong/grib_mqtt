```sequence
Title:DoorLock AddDel Password
Platform->Gateway:add/del Password (topic[ZB3_DoorLock_xxxxxx/_Exe])
Gateway->DoorLock:add/del Password
DoorLock->Gateway:add/del Password Result
Gateway->Platform:add/del Password Result (topic[PLATFORM/Set])
```

### DoorLock  add/del password:

```C
/* add password */
Platform --> GW: topic [ZB3_DoorLock_xxxxxx/_Exe]
{
	"res": "100",
	"msg": {
		"tid": "ZB3_DoorLock_xxxxxx",
		"fnl": [{
			"fn": "PasswdAdd",  
			"tp": "e",
			"vl": "00100001e2400600000e10"
		}]
	}
}
GW --> Platform:topic [PLATFORM/Set] 
{
	"req": "Set",
	"msg": {
		"tid": "ZB3_DoorLock_xxxxxx",
		"fnl": [{
			"fn": "PasswdAdd",	
			"tp": "s",
			"vl": "0"					// 0 -- success, 199 -- failed
		}]
	}
}

/* delete password */
Platform --> GW: topic [ZB3_DoorLock_xxxxxx/_Exe]
{
	"res": "100",
	"msg": {
		"tid": "ZB3_DoorLock_xxxxxx",
		"fnl": [{
			"fn": "PasswdDel",  
			"tp": "e",
			"vl": "0010"
		}]
	}
}
GW --> Platform:topic [PLATFORM/Set] 
{
	"req": "Set",
	"msg": {
		"tid": "ZB3_DoorLock_xxxxxx",
		"fnl": [{
			"fn": "PasswdDel",	
			"tp": "s",
			"vl": "0"					// 0 -- success, 199 -- failed
		}]
	}
}
```

### Function vl 

##### PasswdAdd 

| uint16 | uint32         | uint8                 | uint32      |
| ------ | -------------- | --------------------- | ----------- |
| userId | Password Value | Password Value Length | Duration(s) |

**userId**: 0 ~ 0xff00

**Password Value, Passwod Value Length**: 密码长度只支持6个数字

**Duration**: 密码有效持续时间，unit: second

Example: <br>userId -- 0x0010, PasswdVal -- 0x1e240, PasswdValLen -- 0x06, Duration -- 0xE10

vl string: "00100001e2400600000e10"

##### PasswdDel 

**userId<uint16>**: 0 ~ 0xff00

Example: <br>userId -- 0x0010

vl string: "0010"