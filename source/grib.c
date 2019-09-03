

#include <stdlib.h>
#include <string.h>

#include "grib_proto_common.h"
#include "grib_gateway_proto.h"
#include "util.h"
#include "grib.h"


static int responseHandler(const char cmdStr[], const char didStr[]);


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

int grib_plat_parse(const char dataStr[])
{
	json *root =NULL;
	root = json_parse(dataStr, -1);
	if (NULL == root){
		LOG("Parse Json Error");
		return 1;
	}
	char cmdStr[128], didStr[128];
	eGribMsg_t msgType;
	json *fArray = NULL;
	msgType = parseGribProto(root, cmdStr, didStr, &fArray);
	if (EGRIB_MSG_NONE == msgType){
		LOG("Erro Message Formate");
	}else if (EGRIB_MSG_RES == msgType){
		responseHandler(cmdStr, didStr);
	}else if (EGRIB_MSG_REQ == msgType){

	}
	json_put(root);
	return 0;
}

int grib_dev_serialPub()
{
	/* TODO */
	return 0;
}

static int responseHandler(const char cmdStr[], const char didStr[])
{
	LOG("Response:%s, Tid:%s", cmdStr, didStr);
	return 0;
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

