
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

int parseGribProto(json *root, char cmdStr[], char didStr[], json **fArray)
{
	json *msgJson, *fJsonArray;
	const char *reqPtr,*resPtr, *didPtr;
	eGribMsg_t msgType = EGRIB_MSG_NONE;

	reqPtr = json_get_str(root, GRIB_CMD_KEY_REQ);
	if (reqPtr != NULL){
		strcpy(cmdStr, reqPtr);
		msgType = EGRIB_MSG_REQ; 
	}
	resPtr = json_get_str(root, GRIB_CMD_KEY_RES);
	if (resPtr != NULL){
		strcpy(cmdStr, resPtr);
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

