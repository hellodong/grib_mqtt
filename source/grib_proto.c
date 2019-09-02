

#include <stdlib.h>
#include <string.h>
#include "util.h"

#include "grib_proto.h"


json* makeProto(const char *cmdStr, const char *didStr, json *fArray)
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

    root = makeProto(GRIB_CMD_VAL_REQ, "grib_multitap_00000001", fnlArrayPtr);
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
    //free((void *)jsontoStr); 
    json_put(root);
    //root = NULL;
    LOG("Free");
    return 0;
}

