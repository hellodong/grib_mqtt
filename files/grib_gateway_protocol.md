## Platform Gateway Protocol 

 Gateway Device Id: **GRIB_GATEWAY_xxxxxxxxxx**

###  Function list
| fn             |explain |tp   |
| ----- |----|---- |
|Version|网关版本|s|
|Model|硬件型号|s|
|Reboot|平台下发网关重启|e|
|FirmwareUpgrade|平台下发升级网关固件|se|
|RemoteSh|平台下发远程回连|se|



### Function Json
1. Fn[**Version**]:  

Gateway --> Platform, gateway publish topic:**PLATFORM/Set**

```c
{
	"req":"Set",
	"msg": {
		"tid": "GRIB_GATEWAY_1234567890",
		"fnl": 
		[
			{
				"fn":"Version",
				"tp":"s",
				"vl": "v1.0.32.5_forNxp-0-g291a973"
			}
		]
	}
}
```
2. Fn[**Model**]:  

Gateway --> Platform, gateway publish topic:**PLATFORM/Set**

```c
{
	"req":"Set",
	"msg": {
		"tid": "GRIB_GATEWAY_1234567890",
		"fnl": 
		[
			{
				"fn":"Model",
				"tp":"s",
				"vl": "DSI0177"
			}
		]
	}
}
```

3. Fn[**Reboot**]

Platform --> Gateway, gateway subscribe topic:**GRIB_GATEWAY_1234567890/_Exe**

```c
{
	"res":"100",
	"msg":
	{
		"tid":"GRIB_GATEWAY_1234567890",
		"fnl":
		[
			{
				"fn":"Reboot",
				"tp":"e",
				"vl":"0"
			}
		]
	}
}
```

4. Fn[**FrimwareUpgrade**]:

   Platform --> Gateway, gateway subscribe topic:**GRIB_GATEWAY_1234567890/_Exe**

```c
{
	"res":"100",
	"msg": {
		"tid":"GRIB_GATEWAY_30ae7b2b4917",
		"fnl": {
			"fn": "FirmwareUpgrade",
            "tp": "e",
			"vl": {
				"md5sum": "f600f6b5b171b1c19c8eeacdcfde7339",
				"url": "http://saascloud.oss-cn-shanghai.aliyuncs.com/fireware/fw-1566985560775-262176.bin"
			},
		},
	}
}
```
Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**

```c
{
	"req":"Set",
	"msg": {
		"tid":"GRIB_GATEWAY_1234567890",
		"fnl": {
			"fn": "FirmwareUpgrade",
            "tp": "s",
			"vl": 98,
		},
	}
}
```

5. Fn[**RemoteSh**]:

   Platform --> Gateway, gateway subscribe topic:**GRIB_GATEWAY_1234567890/_Exe**

```c
{
	"res":"100",
	"msg": {
		"tid":"GRIB_GATEWAY_30ae7b2b4917",
		"fnl": {
			"fn": "RemoteSh",
            "tp": "e",
			"vl":{
				"server": "114.215.195.44",
				"port": 3234
			}
		}
	}
}
```
Gateway-->Platform, gateway publish topioc:**PLATFORM/Set**

```c
{
	"req":"Set",
	"msg": {
		"tid":"GRIB_GATEWAY_1234567890",
		"fnl": {
			"fn": "RemoteSh",
            "tp": "s",
			"vl": 0,
		},
	}
}
```
### vl Explain
表格中值只针对 fn: **FrimwareUpgrade**, **RemoteSh**  <br>

| **vl** | **Description** |
| --- | --- |
| 0 | 成功 |
| 99 | 命令已经收到，等待执行 |
| 97 | 升级开始 |
| 98 | 升级成功 |
| 108 | 升级时MD5SUM校验失败 |
| 109 | 升级时固件下载失败 |
| 110 | 升级失败 |




