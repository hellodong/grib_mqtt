

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "grib_proto_common.h"
#include "grib_gateway_proto.h"


typedef void(*stExeFunc_t)(void);
typedef json*(*stSetFunc_t)(void);
typedef void(*stResFunc_t)(void);

typedef struct _protoHandler
{
	const char *fnStr;
	stExeFunc_t exeFunc;
	stSetFunc_t setFunc;
}stProtoHandler_t;


static int resHandler(const char resStr[], const char didStr[]);
static json* status_proto(void);
static void chaserver_proto(void);

#if 0
static void reboot_proto(void);
static void facreset_proto(void);
static void upgrad_proto(void);
#endif

static stProtoHandler_t protoHandler[]={
	{ATTR_GATEWAY_STATUS, 			NULL, 				status_proto, NULL},
	//{ATTR_GATEWAY_CHANGE_SEVER, 	chaserver_proto, 	NULL,		  NULL},
	//{ATTR_GATEWAY_REBOOT, 			reboot_proto, 		NULL},
	//{ATTR_GATEWAY_FACTORY_RESET,	facreset_proto, 	NULL },
	//{ATTR_GATEWAY_UPGRADE_FIRMWARE, upgrad_proto, 		NULL},
	{NULL, NULL, NULL}
};

/*registe device */
json* grib_gateway_regJson(char didStr[])
{
	json *fnlArrayPtr, *tmpRoot, *root = NULL;
	const char *fnStr;
	char gatewayMac[128], macStr[128];
	int idx;

    fnlArrayPtr = json_new_array();
    if (NULL == fnlArrayPtr) {
		LOG("Can't Allocate json array");
		return NULL;
    }

	for(idx = 0;idx < sizeof(protoHandler)/ sizeof(protoHandler[0]);idx++){
		char se[4];	
		int strIdx = 0;

		fnStr = protoHandler[idx].fnStr;
		if (fnStr == NULL){
			break;
		}
		if (protoHandler[idx].setFunc != NULL){
			se[strIdx++] = 's';
		}
		if (protoHandler[idx].exeFunc != NULL){
			se[strIdx++] = 'e';
		}
		se[strIdx] = '\0';
		
		tmpRoot = json_new();
		if (NULL == tmpRoot) {
			LOG("allocat tmproot obj error");
			json_put(fnlArrayPtr);
			return NULL;
		}

		json_add_str(tmpRoot,GRIB_FN_KEY, fnStr);
		json_add_str(tmpRoot,GRIB_TP_KEY, se);
		json_array_add_obj(fnlArrayPtr, tmpRoot);
	}

	get_mac_address(gatewayMac);
	mac2string(gatewayMac, macStr);
	sprintf(didStr, "%s_%s_%s", GRIB_TAG, GATEWAY_TAG, macStr);
	LOG("TID:%s", didStr);
    root = makeGribProto(GRIB_CMD_VAL_REQ, didStr, fnlArrayPtr);
    if (NULL == root){
        LOG("allocat root obj error");
        json_put(fnlArrayPtr);
        return NULL;
    }
	return root;
}

json* grib_gateway_parse(uint8_t cmdMode, const char cmd[], const char didStr[], json *gribFnlArray)
{
	json* root = NULL;
}

/* response handler */
static int resHandler(const char resStr[], const char didStr[])
{
	/* TODO */
	return 0;
}

static json* status_proto(void)
{
	/* TODO */
}

#if 0
static json* chaserver_proto(json *oriJson)
{
	/* TODO */
}

static json* reboot_proto(void)
{
	/* TODO */
}



static void facreset_proto(void)
{
	/* TODO */
}

static void upgrad_proto(void)
{
	/* TODO */
}
}

#endif
