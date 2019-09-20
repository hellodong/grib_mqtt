

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <libubox/list.h>

#include "timer.h"
#include "grib_proto_common.h"
#include "grib_zb3_proto.h"


typedef struct
{
    const char *ubusTypeStr;
    const char *devStr;
    uint8_t    sleepy;
}stUbusDevType_t;

typedef struct
{
    const char *ubusZoneName;
    const char *gribFnName;
}stUbusZoneGribFn_t;

typedef json *(*stRptProtoFunc_t)(json *vlJsonObj, const char *mac);

typedef json *(*stCmdGribFunc_t)(const char *vlStr, const char *mac, int ep);

typedef struct
{
    const char *attrStr;
    stRptProtoFunc_t rptFunc;
}stUbusRptProto_t;

typedef struct
{
    const char *devStr;
    stCmdGribFunc_t cmdFunc;
}stGribCmdProto_t;

typedef struct
{
    int rptDevs;
}stZB3Ctx_t;

typedef struct {
    char devStr[128];
    char macStr[128];
    char tidStr[256];
    struct list_head head;
}stZB3DevTidNode_t;


static json* devlist_action(json *jsonVal);

static json* newDev_2gribProto(json *jsonVal, const char *mac);

static json* devStus_2gribProto(json *jsonVal, const char *mac);

static json* zoneStatus_2gribProto(json *jsonVal, const char *mac);

static json* meterEnergy_2gribProto(json *jsonVal, const char *mac);

static json* meterPower_2gribProto(json *jsonVal, const char *mac);

static json *meterCurrent_2gribProto(json *jsonVal, const char *mac);

static json *onoff_2gribProto(json *jsonVal, const char *mac);

static json *switch_2gribProto(json *jsonVal, const char *mac);

static json *smartPlug_2ubusProto(const char *vlStr, const char *mac, int ep);

static json *switch_2ubusProto(const char *vlStr, const char *mac, int ep);

static json *doorLock_2ubusProto(const char *vlStr, const char *mac, int ep);

static int protoFnStr_Parse(json *fnObj, char fn[], char tp[], char vl[]);


static stZB3DevTidNode_t gZB3TidList;

static stZB3Ctx_t gGribZB3Ctx;

static stUbusRptProto_t gUbusRptProtoCovt[]=
{
    {UBUS_ZB3_DEVADDED_ATTR,    newDev_2gribProto},             //mod.new_device_added
    {UBUS_ZB3_DEVSTUS_ATTR,     devStus_2gribProto},            //device.status
    {UBUS_ZB3_ZONE_ATTR,        zoneStatus_2gribProto},         //device.zone_status
    {UBUS_ZB3_METERENERGY_ATTR, meterEnergy_2gribProto},        //device.meter.energy
    {UBUS_ZB3_METERPOWER_ATTR,  meterPower_2gribProto},         //device.meter.power
    {UBUS_ZB3_METERCURRENT_ATTR,meterCurrent_2gribProto},       //device.meter.current
    {UBUS_ZB3_SWITCH_ATTR,      switch_2gribProto},             //device.switch.onoff
    {UBUS_ZB3_ONOFF_ATTR,       onoff_2gribProto}               //device.onoff
};

static stGribCmdProto_t gGribCmdProtoCovt2Ubus[] = {
    {"DoorLock",    doorLock_2ubusProto},
    {"SmartPlug",   smartPlug_2ubusProto},
    {"Switch",      switch_2ubusProto}
};

static stUbusDevType_t gUbusDevMaps[] = {
    {"1208", "DoorSensor",  1},
    {"1209", "PIRSensor",   1},
    {"1210", "LeakSensor",  1},
    {"1203", "DoorLock",    1},
    {"1218", "SmartPlug",   0},
    {"1212", "Switch",      0}
};

static stUbusZoneGribFn_t gUbusZoneGribFnsMaps[] = {
    {"door",    GRIB_ZB3_FNSET_DOORSTUS_VAL},
    {"leak",    GRIB_ZB3_FNSET_LEAKSTUS_VAL},
    {"pir",     GRIB_ZB3_FNSET_PIRSTUS_VAL}
};

static stZB3DevTidNode_t* addTidNode(const char devStr[], const char macStr[], const char tidStr[])
{
    stZB3DevTidNode_t  *nodePtr = NULL;
    nodePtr = malloc(sizeof(stZB3DevTidNode_t));
    strcpy(nodePtr->devStr, devStr);
    strcpy(nodePtr->macStr, macStr);
    strcpy(nodePtr->tidStr, tidStr);
    list_add_tail(&nodePtr->head, &gZB3TidList.head);

    return nodePtr;
}

static stZB3DevTidNode_t *searchTidNode(const char macStr[])
{
    stZB3DevTidNode_t *posPtr = NULL;

    list_for_each_entry(posPtr, &gZB3TidList.head, head){
       if( strcmp(posPtr->macStr, macStr) == 0){
            return posPtr;
       }
    }   
    return NULL;
}

static const char *getTidNodeDevStr(stZB3DevTidNode_t *nodePtr)
{
    return nodePtr->devStr;
}

static const char *getTidNodeMacStr(stZB3DevTidNode_t *nodePtr)
{
    return nodePtr->macStr;
}

static const char *getTidNodeTidStr(stZB3DevTidNode_t *nodePtr)
{
    return nodePtr->tidStr;
}

static int macExistInTidNode(const char macStr[])
{
    stZB3DevTidNode_t *nodPtr = NULL;

    nodPtr = searchTidNode(macStr);
    if (!nodPtr) {
        return 0;
    }
    return 1;
}

static const char *getTidbyMac(const char macStr[])
{
    stZB3DevTidNode_t *nodPtr = NULL;

    nodPtr = searchTidNode(macStr);
    if (!nodPtr) {
        return NULL;
    }
    return getTidNodeTidStr(nodPtr);
}

static const char *getDevStrbyUbusType(const char *typeStr, int *sleepy)
{
    int idx;
    for(idx = 0;idx < sizeof(gUbusDevMaps)/sizeof(gUbusDevMaps[0]);idx++) {
        if (strcmp(gUbusDevMaps[idx].ubusTypeStr,typeStr) == 0){
            *sleepy = gUbusDevMaps[idx].sleepy;
            return gUbusDevMaps[idx].devStr;
        }
    }
    LOG("Type:%s, Not Found Device String",typeStr);
    return NULL;
}

static const char *getUbustypebyGribDevStr(const char *devStr)
{
    int idx;
    for (idx = 0;idx < sizeof(gUbusDevMaps)/sizeof(gUbusDevMaps[0]);idx++) {
        if (strcmp(gUbusDevMaps[idx].devStr,devStr) == 0){
            return gUbusDevMaps[idx].ubusTypeStr;
        }
    }
    LOG("Device:%s, Not Found Ubus Type", devStr);
    return NULL;
}

static const char *getGribFnbyUbusZone(const char *zoneStr)
{
    int idx;
    for (idx = 0;idx < sizeof(gUbusZoneGribFnsMaps)/sizeof(gUbusZoneGribFnsMaps[0]);idx++) {
        if (strcmp(gUbusZoneGribFnsMaps[idx].ubusZoneName, zoneStr) == 0) {
            return gUbusZoneGribFnsMaps[idx].gribFnName;
        }
    }
    LOG("Zone:%s, Not Found Grib Fn", zoneStr);
    return NULL;
}

static json *rptUbus2Grib(json *jsonVal, const char macStr[], const char attrStr[])
{
    json *root = NULL;
    int idx;
    for(idx = 0;idx < sizeof(gUbusRptProtoCovt) / sizeof(gUbusRptProtoCovt[0]);idx++){
        if (strcmp(gUbusRptProtoCovt[idx].attrStr, attrStr) == 0 ){
            root = gUbusRptProtoCovt[idx].rptFunc(jsonVal, macStr);
            break;
        }
    }
    if (idx ==sizeof(gUbusRptProtoCovt) / sizeof(gUbusRptProtoCovt[0])){
        LOG("Unknown Grib Function(%s)", attrStr);
    }

	return root;
}

static int tidParseMacDev(const char tidStr[], char mac[], char dev[])
{
    char tmpStr[256];
    char *token;
    int ret = 1, idx = 0;

    LOG("TID:%s", tidStr);
    strcpy(tmpStr, tidStr);
    mac[0] = '\0';
    dev[0] = '\0';

    token = strtok(tmpStr, "_");
    while(token != NULL){
        if (idx == 1){
            strcpy(dev, token);
        }else if (idx == 2){
            strcpy(mac, token);
            ret = 0;
        }
        token = strtok(NULL, "_");
        idx++;
    }
    return ret;
}

static void checkDevListRpt(void *arg)
{
    if (!gGribZB3Ctx.rptDevs){
        LOG("Get ZB3.0 Dev List");
        system("dusun_ucmd.sh list");
        timer_add(checkDevListRpt, 10);
    }
}

void grib_zb3_init(void)
{
    INIT_LIST_HEAD(&gZB3TidList.head);
    timer_add(checkDevListRpt, 10);
}

json* ubus_zb3_covt_grib(const char typeStr[], const char macStr[], const char attStr[], json *jsonVal)
{
    json *root = NULL;

    if (strcmp(typeStr, UBUS_TYPECMDRT_VAL) == 0){
        LOG("Cmdtype:%s", typeStr);
    }else if (strcmp(attStr, UBUS_ZB3_DEVLIST_ATTR) == 0){
        LOG("Cmdtype:%s, Attribute:%s", typeStr, attStr);
        devlist_action(jsonVal);
    }else if (strcmp(typeStr, UBUS_TYPERP_VAL) == 0){
        LOG("Cmdtype:%s, Attribute:%s", typeStr, attStr);
        root = rptUbus2Grib(jsonVal, macStr, attStr);
    }else{
        LOG("Not Support Cmd Type:%s, Attribute:%s", typeStr, attStr);
    }

    return root;
}

json* grib_zb3_covt_ubus(json *gribFnlArray, const char *tidStr)
{
    /* Get Mac String ,Device String */
    int ret;
    char macStr[128], devStr[128];
    ret = tidParseMacDev(tidStr, macStr, devStr);
    if (ret){
        LOG("Parse Mac Device Error");
        return NULL;
    }

	json* root = NULL, *fnJsonObj = NULL;

    fnJsonObj = json_object_array_get_idx(gribFnlArray, 0);
    if (NULL == fnJsonObj) {
        LOG("Get Fn Json Obj Error");
        return NULL;
    }
    char fnStr[128], tpStr[128], vlStr[128];
    json *vlJson = NULL;
    int ep;
    ret = protoFnStr_Parse(fnJsonObj, fnStr, tpStr, vlStr);
    if (ret != 0){
        LOG("Parse Fn Error");
        return NULL;
    }
    LOG("Fn :%s", fnStr);
    ep = atoi(&fnStr[strlen(fnStr) - 1]);
    LOG("Endpoint:%d", ep);
    int idx;
    for(idx = 0;idx < sizeof(gGribCmdProtoCovt2Ubus) / sizeof(gGribCmdProtoCovt2Ubus[0]);idx++){
        if (strcmp(gGribCmdProtoCovt2Ubus[idx].devStr, devStr) == 0 ){
            root = gGribCmdProtoCovt2Ubus[idx].cmdFunc(vlStr, macStr, ep);
            break;
        }
    }

    if (idx ==sizeof(gGribCmdProtoCovt2Ubus) / sizeof(gGribCmdProtoCovt2Ubus[0])){
        LOG("Unknown Grib Function");
    }

	return root;
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

static int protoFnStr_Parse(json *fnObj, char fn[], char tp[], char vl[])
{
    const char *fnStr, *tpStr, *vlStr;

    fnStr = json_get_str(fnObj, GRIB_FN_KEY);
    tpStr = json_get_str(fnObj, GRIB_TP_KEY);
    vlStr = json_get_str(fnObj, GRIB_VL_KEY);

    if (fnStr == NULL || tpStr == NULL || vlStr == NULL){
        LOG("Parse Grib Function Error");
        return 1;
    }
    strcpy(fn, fnStr);
    strcpy(tp, tpStr);
    strcpy(vl, vlStr);
    return 0;
}

static const char *dev_topic_subscribe(const char devStr[], const char macStr[])
{
    stZB3DevTidNode_t *tidNodePtr;
    char tidStr[256], topicStr[256];

    tidNodePtr = searchTidNode(macStr);
    if (tidNodePtr){
        LOG("Alread Subscribe(%s)",getTidNodeMacStr(tidNodePtr));
        return getTidNodeTidStr(tidNodePtr);
    }
	sprintf(tidStr, "%s_%s_%s", ZB3_TAG, devStr, macStr);
	//LOG("TID:%s", tidStr);
    tidNodePtr = addTidNode(devStr, macStr, tidStr);

    sprintf(topicStr, "%s/%s", tidStr, GRIB_TOPIC_EXE);
    //LOG("Exe topic:%s", topicStr);

    extern void mqtt_subscribe(const char *);
    mqtt_subscribe(topicStr);

    return getTidNodeTidStr(tidNodePtr);
}

static int devList_element_parse(json *eleJson, char type[], char mac[])
{
    const char *typeStr, *macStr;

   typeStr = json_get_str(eleJson, ZB3_TYPE_TAG);
   macStr = json_get_str(eleJson, ZB3_MAC_TAG);
   if (NULL == typeStr || macStr == NULL){
       LOG("Parse Type Mac Error");
       return 1;
   }
   strcpy(type, typeStr);
   strcpy(mac, macStr);
   return 0;
}

static json* devlist_action(json *jsonVal)
{
    /* parse ubus json object */
    json *dlistJsonArray = NULL;
    dlistJsonArray = json_get_obj(jsonVal, ZB3_DEVLST_TAG);
    if (NULL == dlistJsonArray){
        return NULL;
    }

    const char *devStr = NULL;
    char typeStr[128], macStr[128];
    int sleepy;
    json *eleJson;
    int idx = 0, ret;
    for(idx = 0;idx < json_object_array_length(dlistJsonArray);idx++) {
        eleJson = json_object_array_get_idx(dlistJsonArray, idx);
        if (NULL == eleJson){
            continue;
        }
        ret = devList_element_parse(eleJson, typeStr, macStr);
        if (ret){
            continue;
        }
        devStr = getDevStrbyUbusType(typeStr, &sleepy);
        if (NULL == devStr){
            continue;
        }
        dev_topic_subscribe(devStr, macStr);
    }
    gGribZB3Ctx.rptDevs = 1;

    return NULL;
}

#if 0
static int ubusProto_newDev_parse(json *jsonVal, char *dev, char *mac, char *bat, char *version)
{
    int ret = 0, sleepy;
    const char *typeStr, *devStr, *macStr, *batStr, *verStr;

    typeStr = json_get_str(jsonVal, ZB3_TYPE_TAG);
    if (NULL == typeStr){
        LOG("Get Type From Json Error");
        dev[0] = '\0';
        ret = 1;
    }else{
        devStr = getDevStrbyUbusType(typeStr, &sleepy);
        if (strlen(devStr) > 0){
            strcpy(dev, devStr);
        }else{
            dev[0] = '\0';
        }
    }

    macStr = json_get_str(jsonVal, ZB3_MAC_TAG);
    if (NULL == macStr){
        LOG("Get Mac From Json Error");
        mac[0] = '\0';
        ret = 1;
    }else{
        strcpy(mac, macStr);
    }

    batStr = json_get_str(jsonVal, ZB3_BAT_TAG);
    if (NULL == batStr){
        LOG("Get Battery From Json Error");
        bat[0] = '\0';
    }else{
        if (!sleepy){
            bat[0] = '\0';
        }else{
            strcpy(bat, batStr);
        }
    }

    verStr = json_get_str(jsonVal, ZB3_VERSION_TAG);
    if (NULL == verStr){
        LOG("Get Version from Json Error");
        version[0] = '\0';
        ret = 1;
    }else{
        strcpy(version, verStr);
    }

    return ret;
}
#else
static int ubusProto_newDev_parse(json *jsonVal, char *dev, char *mac, char *bat, char *version, char *mod)
{
    int ret = 0, sleepy;
    const char *typeStr, *devStr, *macStr, *batStr, *verStr, *modStr = NULL;

    typeStr = json_get_str(jsonVal, ZB3_TYPE_TAG);
    if (NULL == typeStr){
        LOG("Get Type From Json Error");
        dev[0] = '\0';
        ret = 1;
    }else{
        devStr = getDevStrbyUbusType(typeStr, &sleepy);
        if (strlen(devStr) > 0){
            strcpy(dev, devStr);
        }else{
            dev[0] = '\0';
        }
    }

    macStr = json_get_str(jsonVal, ZB3_MAC_TAG);
    if (NULL == macStr){
        LOG("Get Mac From Json Error");
        mac[0] = '\0';
        ret = 1;
    }else{
        strcpy(mac, macStr);
    }

    batStr = json_get_str(jsonVal, ZB3_BAT_TAG);
    if (NULL == batStr){
        LOG("Get Battery From Json Error");
        bat[0] = '\0';
    }else{
        if (!sleepy){
            bat[0] = '\0';
        }else{
            strcpy(bat, batStr);
        }
    }

    modStr = json_get_str(jsonVal, ZB3_MODSTR_TAG);
    if (NULL == modStr){
        LOG("Got ModelStr From Json Error");
        mod[0] = '\0';
    }else{
        if (strcmp(devStr, "DoorLock") == 0){
            strcpy(mod, modStr);
        }else{
            mod[0] = '\0';
        }
    }

    verStr = json_get_str(jsonVal, ZB3_VERSION_TAG);
    if (NULL == verStr){
        LOG("Get Version from Json Error");
        version[0] = '\0';
        ret = 1;
    }else{
        strcpy(version, verStr);
    }

    return ret;
}

#endif

static json* gribProto_newDev_serlise(char *version, char *bat, char *mod)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_VERSIONSTR_VAL, "s",   version);       //  version
    protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_ONLINEINT_VAL, "s",    "1");           //  online
    if (strlen(bat) > 0){
        protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_BATINT_VAL, "s",    bat);          //  baterry
    }
    if (strlen(mod) > 0){
        protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_MODELSTR_VAL, "s",  mod);        // modelStr
    }
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static json* newDev_2gribProto(json *jsonVal, const char mac[])
{
    /* parse ubus json object */
    int ret = 1;
    char devStr[128], macStr[128], batStr[128], verStr[128], modStr[128];
    ret = ubusProto_newDev_parse(jsonVal, devStr, macStr, batStr, verStr, modStr);
    if (ret) {
        LOG("Ubus Proto NewDevAdd Parse Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_newDev_serlise(verStr, batStr, modStr);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* subscribe device topic */
    const char *tid;
    tid = dev_topic_subscribe(devStr, macStr);

    /* make grib root object */
    json *gribRoot = NULL;
    LOG("TID:%s", tid);
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, tid, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }

    return gribRoot;
}

static int ubusProto_status_parse(json *jsonVal, char *onlineStr, char *batStr, char *ver, char *mod)
{
    int ret = 0, sleepy = 1, online, bat;
    const char *typeStr, *devStr = NULL;

    onlineStr[0] = '\0';
    batStr[0] = '\0';
    ver[0] = '\0';
    mod[0] = '\0';
    typeStr = json_get_str(jsonVal, ZB3_TYPE_TAG);
    if (NULL == typeStr){
        LOG("Get Type From Json Error");
    }else{
        devStr = getDevStrbyUbusType(typeStr, &sleepy);
    }
    online = json_get_int(jsonVal, ZB3_ONLINE_TAG);
    if (-1 == online){
        LOG("Get Online From Json Error");
        ret++;
    }else{
        sprintf(onlineStr,"%d", online);
    }

    bat = json_get_int(jsonVal, ZB3_BAT_TAG);
    if (-1 == bat){
        LOG("Get Battery From Json Error");
        ret++;
    }else{
        if (sleepy){
            sprintf(batStr, "%d", bat);
        }
    }

    if (strcmp(devStr, "DoorLock") == 0){
        const char *verStr , *modStr = NULL;
        verStr = json_get_str(jsonVal, ZB3_VERSION_TAG);
        modStr = json_get_str(jsonVal, ZB3_MODSTR_TAG);
        if (verStr){
            strcpy(ver, verStr);
        }
        if (modStr){
            strcpy(mod, modStr);
        }
    }

    if (ret == 2){
        return 1;
    }
    return 0;
}

static json* gribProto_status_serlise(char *online, char *bat, char *ver, char *mod)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    if (strlen(online)>0){
        protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_ONLINEINT_VAL, "s",online);
    }
    if (strlen(bat) > 0){
        protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_BATINT_VAL, "s", bat);
    }
    if (strlen(ver) > 0){
        protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_VERSIONSTR_VAL, "s", ver);
    }
    if (strlen(mod) > 0){
        protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_MODELSTR_VAL, "s", mod);
    }
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static json* devStus_2gribProto(json *jsonVal, const char *mac)
{
    /* Exist */
    int ret;
    ret = macExistInTidNode(mac);
    if (ret == 0){
        LOG("Device(%s) Not Subscribe",mac);
        return NULL;
    }

    /* parse ubus json object */
    char onlineStr[128], batStr[128], verStr[128], modStr[128];
    int ep;
    ret = ubusProto_status_parse(jsonVal, onlineStr, batStr, verStr, modStr);
    if (ret) {
        LOG("Ubus Proto Zone Status Parse Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_status_serlise(onlineStr, batStr, verStr, modStr);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* make grib root object */
    json *gribRoot = NULL;
    const char *tid = NULL;

    tid = getTidbyMac(mac);
    LOG("TID:%s", tid);
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, tid, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }
    return gribRoot;
}

static int ubusProto_zoneStus_parse(json *jsonVal, char *fn, char *vl)
{
    int ret = 0;
    const char *zoneStr, *fnStr, *vlStr;

    zoneStr = json_get_str(jsonVal, ZB3_ZONE_TAG);
    if (NULL == zoneStr){
        LOG("Get Zone From Json Error");
        fn[0] = '\0';
        ret = 1;
    }else{
        fnStr = getGribFnbyUbusZone(zoneStr);
        if (fnStr){
            strcpy(fn, fnStr);
        }else{
            LOG("No Match Zone:%s", zoneStr);
            fn[0] = '\0';
            ret = 1;
        }
    }

    vlStr = json_get_str(jsonVal, ZB3_VALUE_TAG);
    if (NULL == vlStr){
        LOG("Get Mac From Json Error");
        vl[0] = '\0';
        ret = 1;
    }else{
        strcpy(vl, vlStr);
    }

    return ret;
}

static json* gribProto_zoneStus_serlise(char *fn, char *vl)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    protoFn_str_serlise(fnlJsonArray, fn, "s",vl);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static json* zoneStatus_2gribProto(json *jsonVal, const char *mac)
{
    /* Exist */
    int ret;
    ret = macExistInTidNode(mac);
    if (ret == 0){
        LOG("Device(%s) Not Subscribe",mac);
        return NULL;
    }

    /* parse ubus json object */
    char fnStr[128], vlStr[128];
    ret = ubusProto_zoneStus_parse(jsonVal, fnStr, vlStr);
    if (ret) {
        LOG("Ubus Proto Zone Status Parse Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_zoneStus_serlise(fnStr, vlStr);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* make grib root object */
    json *gribRoot = NULL;
    const char *tid = NULL;

    tid = getTidbyMac(mac);
    LOG("TID:%s", tid);
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, tid, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }
    return gribRoot;
}

static json* gribProto_energy_serlise(char *vl)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_ENERGYKWH_VAL, "s", vl);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static int ubusProto_energy_parse(json *jsonVal, char *vl)
{
    int ret = 0;
    const char *vlStr;

    vlStr = json_get_str(jsonVal, ZB3_VALUE_TAG);
    if (NULL == vlStr){
        LOG("Get Version from Json Error");
        vl[0] = '\0';
        ret = 1;
    }else{
        strcpy(vl, vlStr);
    }

    return ret;
}

static json* meterEnergy_2gribProto(json *jsonVal, const char macStr[])
{
    int ret;

    /* Exist */
    ret = macExistInTidNode(macStr);
    if (ret == 0){
        LOG("Device Not Subscribe:%s",macStr);
        return NULL;
    }

    /* parse ubus json object */
    char vlStr[128];
    ret = ubusProto_energy_parse(jsonVal, vlStr);
    if (ret) {
        LOG("Ubus Proto Value Parse Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_energy_serlise(vlStr);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* make grib root object */
    json *gribRoot = NULL;
    const char *tid = NULL;

    tid = getTidbyMac(macStr);
    LOG("TID:%s", tid);
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, tid, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }
    return gribRoot;
}

static json* gribProto_power_serlise(char *vl)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_POWERKW_VAL, "s", vl);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static int ubusProto_power_parse(json *jsonVal, char *vl)
{
    int ret = 0;
    const char *vlStr;

    vlStr = json_get_str(jsonVal, ZB3_VALUE_TAG);
    if (NULL == vlStr){
        LOG("Get Version from Json Error");
        vl[0] = '\0';
        ret = 1;
    }else{
        strcpy(vl, vlStr);
    }

    return ret;
}

static json* meterPower_2gribProto(json *jsonVal,const char mac[])
{
    int ret;

    /* Exist */
    ret = macExistInTidNode(mac);
    if (ret == 0){
        LOG("Device Not Subscribe:%s",mac);
        return NULL;
    }

    /* parse ubus json object */
    char vlStr[128];
    ret = ubusProto_power_parse(jsonVal, vlStr);
    if (ret) {
        LOG("Ubus Proto Value Parse Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_power_serlise(vlStr);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* make grib root object */
    json *gribRoot = NULL;
    const char *tid = NULL;

    tid = getTidbyMac(mac);
    LOG("TID:%s", tid);
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, tid, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }
    return gribRoot;
}

static json* gribProto_current_serlise(char *vl)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_CURRENTA_VAL, "s", vl);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static int ubusProto_current_parse(json *jsonVal, char *vl)
{
    int ret = 0;
    const char *vlStr;

    vlStr = json_get_str(jsonVal, ZB3_VALUE_TAG);
    if (NULL == vlStr){
        LOG("Get Version from Json Error");
        vl[0] = '\0';
        ret = 1;
    }else{
        strcpy(vl, vlStr);
    }

    return ret;
}

static json *meterCurrent_2gribProto(json *jsonVal, const char mac[])
{
    int ret;

    /* Exist */
    ret = macExistInTidNode(mac);
    if (ret == 0){
        LOG("Device Not Subscribe:%s",mac);
        return NULL;
    }

    /* parse ubus json object */
    char vlStr[128];
    ret = ubusProto_current_parse(jsonVal, vlStr);
    if (ret) {
        LOG("Ubus Proto Value Parse Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_current_serlise(vlStr);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* make grib root object */
    json *gribRoot = NULL;
    const char *tid = NULL;

    tid = getTidbyMac(mac);
    LOG("TID:%s", tid);
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, tid, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }
    return gribRoot;
}

static json* gribProto_singleonoff_serlise(char *vl)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    protoFn_str_serlise(fnlJsonArray, GRIB_ZB3_FNSET_ONOFF_VAL, "s", vl);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static int ubusProto_onoff_parse(json *jsonVal, char *vl, int *ep)
{
    int ret = 0;
    const char *vlStr;

    vlStr = json_get_str(jsonVal, ZB3_VALUE_TAG);
    if (NULL == vlStr){
        LOG("Get Version from Json Error");
        vl[0] = '\0';
        ret = 1;
    }else{
        strcpy(vl, vlStr);
    }
    *ep = json_get_int(jsonVal, ZB3_EP_TAG);

    return ret;
}

static json *onoff_2gribProto(json *jsonVal, const char mac[])
{
    int ret;

    /* Exist */
    ret = macExistInTidNode(mac);
    if (ret == 0){
        LOG("Device Not Subscribe:%s",mac);
        return NULL;
    }

    /* parse ubus json object */
    int ep;
    char vlStr[128];
    ret = ubusProto_onoff_parse(jsonVal, vlStr, &ep);
    if (ret) {
        LOG("Ubus Proto Value Parse Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_singleonoff_serlise(vlStr);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* make grib root object */
    json *gribRoot = NULL;
    const char *tid = NULL;

    tid = getTidbyMac(mac);
    LOG("TID:%s", tid);
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, tid, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }

    return gribRoot;
}

static json* gribProto_onoff_serlise(char *vl, int ep)
{
    /* function list */
    json *fnlJsonArray = NULL;
    fnlJsonArray = json_new_array();
    if (fnlJsonArray == NULL){
        LOG("Allocate Fnl Json Error");
        return NULL;
    }
    char fnStr[128];
    snprintf(fnStr, sizeof(fnStr), "%s%d", GRIB_ZB3_FNSET_ONOFF_VAL, ep);
    protoFn_str_serlise(fnlJsonArray, fnStr, "s", vl);
    LOG("Fnl:%s", json_to_str(fnlJsonArray));
    return fnlJsonArray;
}

static json *switch_2gribProto(json *jsonVal, const char *mac)
{
    int ret;
    /* Exist */
    ret = macExistInTidNode(mac);
    if (ret == 0){
        LOG("Device Not Subscribe:%s",mac);
        return NULL;
    }

    /* parse ubus json object */
    int ep;
    char vlStr[128];
    ret = ubusProto_onoff_parse(jsonVal, vlStr, &ep);
    if (ret) {
        LOG("Ubus Proto Value Parse Error");
        return NULL;
    }

    /* make grib fnl json array */
    json *fnlJsonArray = NULL;
    fnlJsonArray = gribProto_onoff_serlise(vlStr, ep);
    if (NULL == fnlJsonArray) {
        LOG("Serlise Fnl Json Array");
        return NULL;
    }

    /* make grib root object */
    json *gribRoot = NULL;
    const char *tid = NULL;

    tid = getTidbyMac(mac);
    LOG("TID:%s", tid);
    gribRoot = makeGribProto(GRIB_CMD_VAL_SET, tid, fnlJsonArray);
    if (NULL == gribRoot){
        LOG("Grib Protocol Error");
        json_put(fnlJsonArray);
    }

    return gribRoot;
}

static json *smartPlug_2ubusProto(const char *vlStr, const char *mac, int ep)
{
    json *cpVlJsonObj = NULL;
    int vl;

    vl = atoi(vlStr);
    LOG("Value:%d", vl);

    cpVlJsonObj = json_new();
    json_add_int(cpVlJsonObj, ZB3_VALUE_TAG, vl);
    
    /* ubus data json object*/
    json *dataJson = NULL;
    dataJson = makeUbusCmdDataJson(UBUS_ZB3_ONOFF_ATTR, mac, 1, cpVlJsonObj, GRIB_ZB3_FNSET_ONOFF_VAL);
    if (NULL == dataJson){
        LOG("Ubus Data Json Make Error");
        json_put(cpVlJsonObj);
        return NULL;
    }

    /* ubus all json object */
    json *ubusRoot = NULL;
    ubusRoot = makeUbusProto(UBUS_CLOUD_TAG, UBUS_GPSTR_VAL, mac, UBUS_TYPECMD_VAL, dataJson);
    if (NULL == ubusRoot){
        LOG("Ubus Protocol all Json Make Error");
        json_put(dataJson);
        return NULL;
    }
    return ubusRoot;
}

static json *switch_2ubusProto(const char *vlStr, const char *mac, int ep)
{
    json *cpVlJsonObj = NULL;
    int vl;

    vl = atoi(vlStr);
    LOG("Value:%d", vl);

    cpVlJsonObj = json_new();
    json_add_int(cpVlJsonObj, ZB3_VALUE_TAG, vl);
    json_add_int(cpVlJsonObj, ZB3_EP_TAG, ep);
    
    /* ubus data json object*/
    json *dataJson = NULL;
    dataJson = makeUbusCmdDataJson(UBUS_ZB3_SWITCH_ATTR, mac, ep, cpVlJsonObj, GRIB_ZB3_FNSET_ONOFF_VAL);
    if (NULL == dataJson){
        LOG("Ubus Data Json Make Error");
        json_put(cpVlJsonObj);
        return NULL;
    }

    /* ubus all json object */
    json *ubusRoot = NULL;
    ubusRoot = makeUbusProto(UBUS_CLOUD_TAG, UBUS_GPSTR_VAL, mac, UBUS_TYPECMD_VAL, dataJson);
    if (NULL == ubusRoot){
        LOG("Ubus Protocol all Json Make Error");
        json_put(dataJson);
        return NULL;
    }
    return ubusRoot;
}

static json* doorLock_2ubusProto(const char *vlStr, const char *mac, int ep)
{
    /* TODO */
    return NULL;
}


