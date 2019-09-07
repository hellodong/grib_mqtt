

#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

#include "grib_proto_common.h"
#include "grib_gateway_proto.h"
#include "grib.h"


static json* gribExeProto_covt_ubusProto(const char resStr[], const char didStr[], json *fnlJsonArray);


#if 0
int grib_reqReg(struct mosquitto *mosq)
{
	json *root = NULL;
	char didStr[128], topicReg[256], topicExe[256], topicDel[256];

	root = grib_gateway_regJson(didStr);
	if (NULL == root){
		LOG("Gateway Register Json Error");
		return 1;
	}
	snprintf(topicReg, sizeof(topicReg),"%s/%s", didStr, GRIB_TOPIC_REG);
	snprintf(topicExe, sizeof(topicExe), "%s/%s", didStr, GRIB_TOPIC_EXE);
	snprintf(topicDel, sizeof(topicDel), "%s/%s", didStr, GRIB_TOPIC_DEL);
	LOG("Reg Topic:%s", topicReg);
	LOG("Exe Topic:%s", topicExe);
	LOG("Del Topic:%s", topicDel);
    mosquitto_subscribe(mosq, NULL, topicReg, 1);
    mosquitto_subscribe(mosq, NULL, topicExe, 1);
    mosquitto_subscribe(mosq, NULL, topicDel, 1);

    const char *jsontoStr = NULL;
    int mid, length;

    jsontoStr = json_to_str(root);
    length = strlen(jsontoStr);
    LOG("Send:%s", jsontoStr);
    mosquitto_publish(mosq, &mid, GRIB_PALTFORMREG_TOPIC , length, jsontoStr, 1, 0);
    json_put(root);
    root = NULL;
	return 0;
}
#endif

json* grib_plat_parse(const char dataStr[])
{
	json *root =NULL;

	root = json_parse(dataStr, -1);
	if (NULL == root){
		LOG("Parse Json Error");
		return NULL;
	}
	char resStr[128], didStr[128];
	eGribMsg_t msgType;
	json *ubusJsonRoot = NULL,*fArray = NULL;
	msgType = parseGribProto(root, resStr, didStr, &fArray);
	if (EGRIB_MSG_RES == msgType){
        ubusJsonRoot = gribExeProto_covt_ubusProto(resStr, didStr, fArray);
	}else {
		LOG("Erro Message Formate");
	}
	json_put(root);

	return ubusJsonRoot;
}

json* ubus_conv2_grib(json *ori, char topic[])
{
    json *root = NULL, *jsonVal =NULL;
    int ret, ep;
    char devStr[128], typeStr[128], attrStr[128], idStr[256];

    ret = parseUbusProto(ori, devStr, typeStr ,&ep, attrStr, &jsonVal);
    if (0 != ret){
        LOG("Parse Ubus Message Error\r\n");
        return NULL;
    }
    LOG("Device:%s ", devStr);
    strcpy(topic, GRIB_PLATFORSET_TOPIC);
    if (strcmp(devStr, UBUS_DEVSTR_VAL) == 0 ){     //gateway
        root = ubus_gateway_covt_grib(typeStr, attrStr, jsonVal);
    }else if (strcmp(devStr, UBUS_GPSTR_VAL) == 0){  //greenpower
        /* TODO */
    }

	return root;
}

static json* gribExeProto_covt_ubusProto(const char resStr[], const char didStr[], json *fnlJsonArray)
{
    int gwCmpRet= 1, zbCmpRet = 1;

	LOG("Response:%s, Tid:%s", resStr, didStr);
#if 0
    if (strcmp(resStr, "100") != 0){
        LOG("Bad Response Header:%s", resStr);
        return NULL;
    }
#endif
    gwCmpRet = grib_gateway_tidCmp(didStr);
    json *root = NULL;
    if (0 == gwCmpRet){                         //gateway;
        root = grib_gateway_covt_ubus(fnlJsonArray);
    }else if (0 == zbCmpRet){                   //greenpower
        /* TODO */
    }else{
        LOG("Incorrect Device ID:%s", didStr);
    }

	return root;
}

int grib_test_reqReg(struct mosquitto *mosq)
{
    json *fnlArrayPtr, *tmpRoot, *root = NULL;

    fnlArrayPtr = json_new_array();
    if (NULL == fnlArrayPtr) {
        return 1;
    }

    tmpRoot = json_new();
    if (NULL == tmpRoot) {
        LOG("allocat tmproot obj error");
        json_put(fnlArrayPtr);
        return 2;
    }
    json_add_str(tmpRoot,GRIB_FN_KEY, "Power");
    json_add_str(tmpRoot,GRIB_TP_KEY, "se");
    json_array_add_obj(fnlArrayPtr, tmpRoot);

    root =makeGribProto(GRIB_CMD_VAL_REQ, "grib_multitap_00000001", fnlArrayPtr);
    if (NULL == root){
        LOG("allocat tmproot obj error");
        json_put(fnlArrayPtr);
        return 3;
    }
    mosquitto_subscribe(mosq, NULL, "grib_multitap_00000001/_Reg", 1);

    const char *jsontoStr = NULL;
    int mid, length;

    jsontoStr = json_to_str(root);
    length = strlen(jsontoStr);
    LOG("Send:%s", jsontoStr);
    mosquitto_publish(mosq, &mid, GRIB_PALTFORMREG_TOPIC , length, jsontoStr, 1, 0);
    json_put(root);
    root = NULL;
    return 0;
}

