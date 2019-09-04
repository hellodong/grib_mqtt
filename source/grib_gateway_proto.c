

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
    char tid[128];
}stCtx_t;


static json* status_proto(json *jsonVal);

static json* fwupgd_2ubusproto(json *vlJsonObj);

static json* reboot_2ubusProto(json *vlJsonObj);

static int protoFnObj_Parse(json *fnObj, char fn[], char tp[], json **vlPtr);


static stCtx_t gGribGwctx;
static stGribExeProto_t gGribExeProtoCovt[]=
{
    {GATEWAY_FNEXE_FWUPGD_VAL, fwupgd_2ubusproto},        //firmware upgrade
    {GATEWAY_FNEXE_REBOOT_VAL, reboot_2ubusProto}         //reboot
};


/*registe device */
json* grib_gateway_regJson(char didStr[])
{
	json *fnlArrayPtr, *tmpRoot, *root = NULL;
	const char *fnStr;
	int idx;

    fnlArrayPtr = json_new_array();
    if (NULL == fnlArrayPtr) {
		LOG("Can't Allocate json array");
		return NULL;
    }

#if 0
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
#endif
    const char *tidStr;
    tidStr = gGribGwctx.tid;
    
    root = makeGribProto(GRIB_CMD_VAL_REQ, tidStr, fnlArrayPtr);
    if (NULL == root){
        LOG("allocat root obj error");
        json_put(fnlArrayPtr);
        return NULL;
    }
	return root;
}

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
    sprintf(topic, "%s/%s", gGribGwctx.tid, GRIB_TOPIC_EXE);
	LOG("TID:%s", gGribGwctx.tid);
    LOG("Gateway Exe topic:%s", topic);
    return 0;
}


json* ubus_gateway_covt_grib(const char cmdStr[], json *jsonVal)
{
    json *root = NULL;
    if (strcmp(cmdStr, ATTR_GATEWAY_STATUS) == 0){
        root = status_proto(jsonVal);
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

static int ubusProto_status_parse(json *jsonVal, char *version, char *model, char *fac, char uplinkType[], int *cuTimePtr, int *upTimePtr)
{
    const char *verStr, *modStr, *facStr, *uplinkStr;

    verStr = json_get_str(jsonVal, GATEWAY_STATUS_VERSION);
    if (NULL == verStr){
        LOG("Get Version from Json Error");
    }else{
        strcpy(version, verStr);
    }
    modStr = json_get_str(jsonVal, GATEWAY_STATUS_MOD);
    if (NULL == modStr){
        LOG("Get Version from Json Error");
    }else{
        strcpy(model, modStr);
    }
    facStr = json_get_str(jsonVal, GATEWAY_STATUS_FAC);
    if (NULL == facStr){
        strcpy(fac, facStr);
    }
    return 0;
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

#if 0
static json* gribProto_status_serlise(char *version, char *model, char *fac, char *uplink, int cuTime, int upTime)
{
    json *fnlJsonArray = NULL;

    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }

    protoFn_str_serlise(fnlJsonArray, GATEWAY_STATUS_VERSION, "s", version);
    protoFn_str_serlise(fnlJsonArray, GATEWAY_STATUS_MOD, "s", model);
    protoFn_str_serlise(fnlJsonArray, GATEWAY_STATUS_FAC, "s", fac);
    protoFn_int_serlise(fnlJsonArray, GATEWAY_STATUS_CTIME, "s", cuTime);
    protoFn_int_serlise(fnlJsonArray, GATEWAY_STATUS_UPTIME, "s", upTime);
    protoFn_str_serlise(fnlJsonArray, GATEWAY_STATUS_UPLINKT, "s", uplink);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}
#else
static json* gribProto_status_serlise(json *jsonVl)
{
    json *fnlJsonArray = NULL;

    /* function list */
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }

    protoFn_obj_serlise(fnlJsonArray, GATEWAY_STATUS_VAL, "s",jsonVl);

    LOG("Fnl:%s", json_to_str(fnlJsonArray));

    return fnlJsonArray;
}
#endif

static json* status_proto(json *jsonVal)
{
    const char *jsonValStr;
    char gatewayMac[32]; 
    char didStr[64];
    int cpRet;
    json *fnlJsonArray = NULL, *cpJsonVal;

    jsonValStr = json_to_str(jsonVal); 
    cpJsonVal = json_parse(jsonValStr, -1);
    if (NULL == cpJsonVal){
        LOG("Copy JsonVal Error");
        return NULL;
    }
    fnlJsonArray = gribProto_status_serlise(cpJsonVal);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* root object */
    get_mac_address(gatewayMac);
    get_mac_address_compact(gatewayMac);
    sprintf(didStr, "%s_%s_%s", GRIB_TAG, GATEWAY_TAG, gatewayMac);
    LOG("TID:%s", didStr);
    return makeGribProto(GRIB_CMD_VAL_SET, didStr, fnlJsonArray);
}

static json* fwupgd_2ubusproto(json *vlJsonObj)
{
    json *ubusRoot = NULL, *cpVlJsonObj = NULL;
    const char *vlJsonStr;
    vlJsonStr = json_to_str(vlJsonObj);
    if (NULL == vlJsonStr){
        LOG("No");
    }

    return ubusRoot;
}

static json* reboot_2ubusProto(json *vlJsonObj)
{
    json *ubusRoot = NULL;
    /* TODO */
    LOG("\r\n");

    return ubusRoot;
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

