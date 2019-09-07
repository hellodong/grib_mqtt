## Gateway
gateway tid: grib_gateway_xxxxx

### fn attribute list

| fn             |explain |tp   |
| ----- |----|---- |
| gateway.status |网关状态| s    |
|gateway.reboot|平台下发网关重启|se|
|gateway.upgrade_firmware|平台下发升级网关固件|se|



### Sample
gateway.status, 设备上报
```c
{
	"req":"Set",
	"msg": {
		"tid": "grib_gateway_30ae7b2b4917",
		"fnl": 
		[
			{
				"fn":"gateway.status",
				"tp":"s",
				"vl": {
						"version": "v1.0.32.5_forNxp-0-g291a973",
						"model": "DSI0177",
						"factory": "dusun",
						"current_time": 1567494167,
						"uptime": 103206,
						"uplinkType": "Ethernet"
				}
			}
		]
	}
}
```
gateway.reboot, 平台下发
```c
{
	"req":"Exe",
	"msg":
	{
		"tid":"grib_gateway_30ae7b2b4917",
		"fnl":
		[
			{
				"fn":"gateway.reboot",
				"tp":"e",
				"vl":{
					"id":"xxxx",
					"reboot_delay": 0,
					"ep": 1
				}
			}
		]
	}
}
```
网关回复
```c
{
	"res":"100",
	"msg":
	{
		"tid":"grib_gateway_30ae7b2b4917",
		"fnl":
		[
			{
				"fn":"gateway.reboot",
				"tp":"s",
				"vl":{
					"id":"xxxx",
					"ep": 1,
					"code": 0
				}
			}
		]
	}
}
```
gateway.upgrad_firmware 平台下发
```c
{
	"req":"Exe",
	"msg": {
		"tid":"grib_gateway_30ae7b2b4917",
		"fnl": {
			"fn": "gateway.upgrade_firmware",
            "tp": "e",
			"vl": {
				"ep": 1,
				"id": "xxxxxxx",
				"md5sum": "f600f6b5b171b1c19c8eeacdcfde7339",
				"url": "http://saascloud.oss-cn-shanghai.aliyuncs.com/fireware/fw-1566985560775-262176.bin"
			},
		},
	}
}
```
网关回复
```c
{
	"res":"100",
	"msg": {
		"tid":"grib_gateway_30ae7b2b4917",
		"fnl": {
			"fn": "gateway.upgrade_firmware",
            "tp": "s",
			"vl": {
				"ep": 1,
				"id": "xxxxxxx",
				"code":99
			},
		},
	}
}
{
	"res":"100",
	"msg": {
		"tid":"grib_gateway_30ae7b2b4917",
		"fnl": {
			"fn": "gateway.upgrade_firmware",
            "tp": "s",
			"vl": {
				"ep": 1,
				"id": "xxxxxxx",
				"code":97
			},
		},
	}
}

{
	"res":"100",
	"msg": {
		"tid":"grib_gateway_30ae7b2b4917",
		"fnl": {
			"fn": "gateway.upgrade_firmware",
            "tp": "s",
			"vl": {
				"ep": 1,
				"id": "xxxxxxx",
				"code":98
			},
		},
	}
}
```



