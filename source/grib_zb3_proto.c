

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

#include "grib_proto_common.h"
#include "grib_gateway_proto.h"


typedef json *(*stExeProtoFunc_t)(json *vlJsonObj);

typedef struct
{
    const char *fnStr;
    stExeProtoFunc_t exeFunc;
}stGribExeProto_t;

typedef struct
{
    char mac[128];
    char tid[128];
}stCtx_t;


static json* status_2gribProto(json *jsonVal);

static json* cmdResult_2gribProto(json *dtJsonObj);

static json* fwupgd_2ubusproto(json *vlJsonObj);

static json* reboot_2ubusProto(json *vlJsonObj);

static json* remotesh_2ubusproto(json *vlJsonObj);

static int protoFnObj_Parse(json *fnObj, char fn[], char tp[], json **vlPtr);


static stCtx_t gGribGwctx;
static stGribExeProto_t gGribExeProtoCovt[]=
{
    {GATEWAY_FNEXE_REMOTESH_VAL,remotesh_2ubusproto},     //remote shell
    {GATEWAY_FNEXE_FWUPGD_VAL, fwupgd_2ubusproto},        //firmware upgrade
    {GATEWAY_FNEXE_REBOOT_VAL, reboot_2ubusProto}         //reboot
};


int grib_gateway_tidCmp(const char tidStr[])
{
   return  strcmp(gGribGwctx.tid, tidStr);
}

int grib_gateway_exeTopic(char topic[])
{
	char gatewayMac[128];
    
	get_mac_address(gatewayMac);
    get_mac_address_compact(gatewayMac);
	sprintf(gGribGwctx.tid, "%s_%s_%s", GRIB_TAG, GATEWAY_TAG, gatewayMac);
    strcpy(gGribGwctx.mac, gatewayMac);
    sprintf(topic, "%s/%s", gGribGwctx.tid, GRIB_TOPIC_EXE);
	LOG("TID:%s", gGribGwctx.tid);
    LOG("Gateway Exe topic:%s", topic);
    return 0;
}

json* ubus_gateway_covt_grib(const char typeStr[], const char attStr[], json *jsonVal)
{
    json *root = NULL;

    if (strcmp(typeStr, UBUS_TYPECMDRT_VAL) == 0){
        LOG("Cmdtype:%s, Attribute:%s", typeStr, attStr);
        root = cmdResult_2gribProto(jsonVal);
    }else if (strcmp(attStr, ATTR_GATEWAY_STATUS) == 0){
        LOG("Cmdtype:%s", typeStr);
        root = status_2gribProto(jsonVal);
    }

    return root;
}

json* grib_gateway_covt_ubus(json *gribFnlArray)
{
	json* root = NULL, *fnJsonObj = NULL;

    fnJsonObj = json_object_array_get_idx(gribFnlArray, 0);
    if (NULL == fnJsonObj) {
        LOG("Get Fn Json Obj Error");
        return NULL;
    }
    char fnStr[128], tpStr[128];
    json *vlJson = NULL;
    int ret;
    ret = protoFnObj_Parse(fnJsonObj, fnStr, tpStr, &vlJson);
    if (ret != 0){
        LOG("Parse Fn Error");
        return NULL;
    }
    LOG("Parse Fn:%s", fnStr);
    int idx;
    for(idx = 0;idx < sizeof(gGribExeProtoCovt) / sizeof(gGribExeProtoCovt[0]);idx++){
        if (strcmp(gGribExeProtoCovt[idx].fnStr, fnStr) == 0 ){
            root = gGribExeProtoCovt[idx].exeFunc(vlJson);
            break;
        }
    }

    if (idx ==sizeof(gGribExeProtoCovt) / sizeof(gGribExeProtoCovt[0])){
        LOG("Unknown Grib Function");
    }

	return root;
}

static int ubusProto_status_parse(json *jsonVal, char *version, char *model, char *uplinkType)
{
    const char *verStr, *modStr, *uplinkStr;
    int ret = 0;

    verStr = json_get_str(jsonVal, GATEWAY_STATUS_VERSION);
    if (NULL == verStr){
        LOG("Get Version from Json Error");
        ret = 1;
    }else{
        strcpy(version, verStr);
    }

    modStr = json_get_str(jsonVal, GATEWAY_STATUS_MOD);
    if (NULL == modStr){
        LOG("Get Version from Json Error");
        ret = 1;
    }else{
        strcpy(model, modStr);
    }

    uplinkStr = json_get_str(jsonVal, GATEWAY_STATUS_UPLINKT);
    if (NULL == uplinkType){
        LOG("Get Uplink Type Val Error");
        ret = 1;
    }else{
        strcpy(uplinkType, uplinkStr);
    }
    return ret;
}

static int protoFn_str_serlise(json *fnlJsonArray, const char fn[], const char tp[], const char vl[])
{
    json *tmpJson = NULL;
    tmpJson = json_new();
    if (tmpJson == NULL) {
        LOG("Allocate Fn json Error");
        return 1;
    }
    json_add_str(tmpJson, GRIB_FN_KEY, fn);
    json_add_str(tmpJson, GRIB_TP_KEY, tp);
    json_add_str(tmpJson, GRIB_VL_KEY, vl);
    json_array_add_obj(fnlJsonArray, tmpJson);
    return 0;
}

static int protoFn_int_serlise(json *fnlJsonArray, const char fn[], const char tp[], int vl)
{
    json *tmpJson = NULL;
    tmpJson = json_new();
    if (tmpJson == NULL) {
        LOG("Allocate Fn json Error");
        return 1;
    }
    json_add_str(tmpJson, GRIB_FN_KEY, fn);
    json_add_str(tmpJson, GRIB_TP_KEY, tp);
    json_add_int(tmpJson, GRIB_VL_KEY, vl);
    json_array_add_obj(fnlJsonArray, tmpJson);
    return 0;

}

static int protoFn_obj_serlise(json *fnlJsonArray, const char fn[], const char tp[], json *jsonVl)
{
    json *tmpJson = NULL;
    tmpJson = json_new();
    if (tmpJson == NULL) {
        LOG("Allocate Fn json Error");
        return 1;
    }
    json_add_str(tmpJson, GRIB_FN_KEY, fn);
    json_add_str(tmpJson, GRIB_TP_KEY, tp);
    json_add_obj(tmpJson, GRIB_VL_KEY, jsonVl);
    json_array_add_obj(fnlJsonArray, tmpJson);
    return 0;
}

static int protoFnObj_Parse(json *fnObj, char fn[], char tp[], json **vlPtr)
{
    const char *fnStr, *tpStr;
    json *vl;

    fnStr = json_get_str(fnObj, GRIB_FN_KEY);
    tpStr = json_get_str(fnObj, GRIB_TP_KEY);
    vl = json_get_obj(fnObj, GRIB_VL_KEY);

    if (fnStr == NULL || tpStr == NULL || vl == NULL){
        LOG("Parse Grib Function Error");
        return 1;
    }
    strcpy(fn, fnStr);
    strcpy(tp, tpStr);
    *vlPtr = vl;
    return 0;
}

static json* gribProto_status_serlise(char *version, char *model, char *uplink)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    protoFn_str_serlise(fnlJsonArray, GRIB_GATEWAY_FNSET_VERSION_VAL, "s", version);
    protoFn_str_serlise(fnlJsonArray, GRIB_GATEWAY_FNSET_MODEL_VAL, "s", model);
    //protoFn_str_serlise(fnlJsonArray, GATEWAY_STATUS_UPLINKT, "s", uplink);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static json *gribProto_cmdResult_serlise(const char fn[], int code)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    protoFn_int_serlise(fnlJsonArray, fn, "s", code);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static json* cmdResult_2gribProto(json *dtJsonObj)
{
    /* parse cmd result data json object*/
    const char *id;
    int code;

    id = json_get_str(dtJsonObj, UBUS_IDSTR_TAG);
    if (id == NULL){
        LOG("Parse ID Error");
        return NULL;
    }
    code = json_get_int(dtJsonObj, UBUS_CODE_TAG);
    if (-1 == code){
        LOG("Parse Code Error");
        return NULL;
    }

    /* grib  fnl json array protocol */
    json *fnlJsonArray;
    fnlJsonArray = gribProto_cmdResult_serlise(id, code);
    if (NULL == fnlJsonArray){
        LOG("Grib fnl json array protocol make error");
        return NULL;
    }
    
    /* make grib proto object*/
    json *gribJsonRoot;
    gribJsonRoot = makeGribProto(GRIB_CMD_VAL_SET, gGribGwctx.tid, fnlJsonArray);
    if (NULL == gribJsonRoot){
        LOG("Grib all json protocol make error");
        return NULL;
    }
    return gribJsonRoot;
}

static json* status_2gribProto(json *jsonVal)
{
    /* parse ubus json object */
    char verStr[128], modStr[128], uplinkStr[128];
    int ret;
    ret = ubusProto_status_parse(jsonVal, verStr, modStr, uplinkStr);
    if (ret){
        LOG("Parse Ubus Status Protocol Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_status_serlise(verStr, modStr, uplinkStr);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* make grib root object */
    char gatewayMac[32]; 
    char didStr[64];
    get_mac_address(gatewayMac);
    get_mac_address_compact(gatewayMac);
    sprintf(didStr, "%s_%s_%s", GRIB_TAG, GATEWAY_TAG, gatewayMac);
    LOG("TID:%s", didStr);

    json *gribRoot = NULL;
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, didStr, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }
    return gribRoot;
}

static json* fwupgd_2ubusproto(json *vlJsonObj)
{
    json *cpVlJsonObj = NULL;
    const char *vlJsonStr;

    /* copy grib value json object */
    vlJsonStr = json_to_str(vlJsonObj);
    if (NULL == vlJsonStr){
        LOG(" Value Json String");
        return NULL;
    }
    LOG("Value Json:%s", vlJsonStr);
    cpVlJsonObj = json_parse(vlJsonStr, -1);
    if (NULL == cpVlJsonObj){
        LOG("Copy Value Json Error");
        return NULL;
    }
    
    /* ubus data json object*/
    json *dataJson = NULL;
    dataJson = makeUbusCmdDataJson(ATTR_GATEWAY_UPGRADE_FIRMWARE, gGribGwctx.mac, 1, cpVlJsonObj, GATEWAY_FNEXE_FWUPGD_VAL);
    if (NULL == dataJson){
        LOG("Ubus Data Json Make Error");
        json_put(cpVlJsonObj);
        return NULL;
    }

    /* ubus all json object */
    json *ubusRoot = NULL;
    ubusRoot = makeUbusProto(UBUS_CLOUD_TAG,UBUS_DEVSTR_VAL, gGribGwctx.mac, UBUS_TYPECMD_VAL, dataJson);
    if (NULL == ubusRoot){
        LOG("Ubus Protocol all Json Make Error");
        json_put(dataJson);
        return NULL;
    }
    return ubusRoot;
}

static json* reboot_2ubusProto(json *vlJsonObj)
{
    json *newVlJsonObj = NULL;

    newVlJsonObj = json_new();
    json_add_int(newVlJsonObj, "reboot_delay", 10);
    
    /* ubus data json object*/
    json *dataJson = NULL;
    dataJson = makeUbusCmdDataJson(ATTR_GATEWAY_REBOOT, gGribGwctx.mac, 1, newVlJsonObj, GATEWAY_FNEXE_REBOOT_VAL);
    if (NULL == dataJson){
        LOG("Ubus Data Json Make Error");
        json_put(newVlJsonObj);
        return NULL;
    }

    /* ubus all json object */
    json *ubusRoot = NULL;
    ubusRoot = makeUbusProto(UBUS_CLOUD_TAG,UBUS_DEVSTR_VAL, gGribGwctx.mac,UBUS_TYPECMD_VAL, dataJson);
    if (NULL == ubusRoot){
        LOG("Ubus Protocol all Json Make Error");
        json_put(dataJson);
        return ubusRoot;
    }
    return ubusRoot;
}

static json* remotesh_2ubusproto(json *vlJsonObj)
{
    json *cpVlJsonObj = NULL;
    const char *vlJsonStr;

    /* copy grib value json object */
    vlJsonStr = json_to_str(vlJsonObj);
    if (NULL == vlJsonStr){
        LOG(" Value Json String");
        return NULL;
    }
    LOG("Value Json:%s", vlJsonStr);
    cpVlJsonObj = json_parse(vlJsonStr, -1);
    if (NULL == cpVlJsonObj){
        LOG("Copy Value Json Error");
        return NULL;
    }
    
    /* ubus data json object*/
    json *dataJson = NULL;
    dataJson = makeUbusCmdDataJson(ATTR_GATEWAY_REMOTE_SHELL, gGribGwctx.mac, 1, cpVlJsonObj, GATEWAY_FNEXE_REMOTESH_VAL);
    if (NULL == dataJson){
        LOG("Ubus Data Json Make Error");
        json_put(cpVlJsonObj);
        return NULL;
    }

    /* ubus all json object */
    json *ubusRoot = NULL;
    ubusRoot = makeUbusProto(UBUS_CLOUD_TAG,UBUS_DEVSTR_VAL, gGribGwctx.mac, UBUS_TYPECMD_VAL, dataJson);
    if (NULL == ubusRoot){
        LOG("Ubus Protocol all Json Make Error");
        json_put(dataJson);
        return NULL;
    }
    return ubusRoot;
}


