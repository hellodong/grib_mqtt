
#include <stdlib.h>
#include <string.h>

#include "grib_proto_common.h"


json* makeGribProto(const char *cmdStr, const char *didStr, json *fArray)
{
   json *rootPtr, *msgPtr, *tidPtr, *fnlArrayPtr;

   /* root json */
   rootPtr = json_new();
   if (NULL == rootPtr){
       LOG("allocat root obj json Error");
       return NULL;
   }
   json_add_str(rootPtr, GRIB_CMD_KEY_REQ, cmdStr);

   /* msg json */
   msgPtr = json_new();
   if (NULL == msgPtr) {
       LOG("allocat msg obj json Error");
        json_put(rootPtr);
        return NULL;
   }
   json_add_str(msgPtr, GRIB_TID_KEY, didStr);
   json_add_obj(msgPtr, GRIB_FNLARRAY_KEY, fArray);

   json_add_obj(rootPtr, GRIB_MSGOBJ_KEY,msgPtr);
    
   return rootPtr;
}

int parseGribProto(json *root, char resStr[], char didStr[], json **fArray)
{
	json *msgJson, *fJsonArray;
	const char *reqPtr,*resPtr, *didPtr;
	eGribMsg_t msgType = EGRIB_MSG_NONE;

	resPtr = json_get_str(root, GRIB_CMD_KEY_RES);
	if (resPtr != NULL){
		strcpy(resStr, resPtr);
		msgType = EGRIB_MSG_RES;
	}

	msgJson = json_get_obj(root, GRIB_MSGOBJ_KEY);
	if (msgType == EGRIB_MSG_NONE || msgJson == NULL){
		LOG("Parse Error1");
		return EGRIB_MSG_NONE;
	}

	didPtr = json_get_str(msgJson, GRIB_TID_KEY);
	fJsonArray = json_get_obj(msgJson, GRIB_FNLARRAY_KEY);
	if (NULL == didPtr || fJsonArray == NULL){
		LOG("Parse Error2");		
		return EGRIB_MSG_NONE;
	}
	strcpy(didStr, didPtr);
	*fArray = fJsonArray;

	return msgType;
}

int parseUbusProto(json *root, char dev[], char type[], int *ep, char att[], json **value)
{
    const char *devStr = NULL, *typeStr = NULL;
    json *dataJson, *argJson;

    dev[0] = '\0';
    type[0] = '\0';
    att[0] = '\0';
    /* root */
    devStr = json_get_str(root, UBUS_FROM_TAG);         //from
    typeStr = json_get_str(root, UBUS_TYPE_TAG);        //type
    dataJson = json_get_obj(root, UBUS_DATAOBJ_TAG);    //data object

    if (devStr == NULL || typeStr == NULL || dataJson == NULL){
        LOG("Ubus Parse Error\r\n");
        return 1;
    }
    *ep = json_get_int(root, UBUS_EPINT_TAG);
    strcpy(dev, devStr);
    strcpy(type, typeStr);
    LOG("From:%s, Type:%s, EP:%d", devStr, typeStr, *ep);

    /* data */
    const char *attrStr;
    if (strcmp(typeStr, UBUS_TYPERP_VAL) == 0){          //reportAttribute
       *value = json_get_obj(dataJson, UBUS_VLOBJ_TAG); 
       if (*value == NULL){
           LOG("Parse Ubus value object Error");
           return 2;
       }
       attrStr = json_get_str(dataJson, UBUS_ATTRSTR_TAG);
       if (NULL == attrStr){
           LOG("Parse Ubus Attr Value Error");
           return 3;
       }
       strcpy(att, attrStr);
    }else if (strcmp(typeStr, UBUS_TYPECMDRT_VAL) == 0){ //cmdReport
        *value = dataJson;
    }

    return 0;
}

json* makeUbusProto(const char from[], const char to[], const char mac[],  const char cmdType[], json *dataPtr)
{
    json *root = NULL;

    root = json_new();
    if (NULL == root){
        LOG("allocate root json Error");
        return NULL;
    }
    json_add_str(root, UBUS_TO_TAG, to);
    json_add_str(root, UBUS_FROM_TAG, from);
    json_add_str(root, UBUS_MACSTR_TAG,  mac);
    json_add_str(root, UBUS_TYPE_TAG, cmdType);
    json_add_obj(root, UBUS_DATAOBJ_TAG, dataPtr);

    return root;
}

json *makeUbusCmdDataJson(const char attr[], const char mac[], int ep, json *vlJson, const char id[])
{
    json *argJson;
    /* arguments json object */
    argJson = json_new();
    if (argJson == NULL){
        LOG("Allocate new Json Error");
        json_put(vlJson);
        return NULL;
    }
    json_add_str(argJson, UBUS_ATTRSTR_TAG,attr);
    json_add_str(argJson, UBUS_MACSTR_TAG, mac);
    json_add_int(argJson, UBUS_EPINT_TAG, ep);
    json_add_obj(argJson, UBUS_VLOBJ_TAG, vlJson);

    /* data json object */
    json *dataJson;
    dataJson = json_new();
    if (NULL == dataJson){
        LOG("Allocate new Json Error");
        json_put(argJson);
        return NULL;
    }
    json_add_str(dataJson, UBUS_IDSTR_TAG, id);
    json_add_str(dataJson, UBUS_COMMAND_TAG, UBUS_TYPESET_VAL); 
    json_add_obj(dataJson, UBUS_ARGOBJ_TAG, argJson);
    return dataJson;
} 
