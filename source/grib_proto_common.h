

#ifndef _GRIB_PROTO_COMMON_H_
#define _GRIB_PROTO_COMMON_H_


#include <dusun/dusun.h>


#define GRIB_CMD_KEY_REQ        "req"
#define GRIB_CMD_KEY_RES        "res"
#define GRIB_MSGOBJ_KEY         "msg"
#define GRIB_TID_KEY            "tid"
#define GRIB_FNLARRAY_KEY       "fnl"
#define GRIB_FN_KEY             "fn"
#define GRIB_TP_KEY             "tp"
#define GRIB_VL_KEY             "vl"

#define GRIB_CMD_VAL_REQ        "Reg"
#define GRIB_CMD_VAL_SET        "Set"

#define GRIB_PALTFORMREG_TOPIC  "PLATFORM/Reg"
#define GRIB_PLATFORSET_TOPIC   "PLATFORM/Set"

#define GRIB_TAG				"GRIB"
#define ZB3_TAG                 "ZB3"

#define GRIB_TOPIC_REG			"_Reg"
#define GRIB_TOPIC_EXE			"_Exe"

#define UBUS_FROM_TAG           "from"
#define UBUS_TO_TAG             "to"
#define UBUS_TYPE_TAG           "type"
#define UBUS_MACSTR_TAG         "mac"
#define UBUS_DATAOBJ_TAG        "data"
#define UBUS_IDSTR_TAG          "id"
#define UBUS_COMMAND_TAG        "command"
#define UBUS_CODE_TAG           "code"
#define UBUS_ARGOBJ_TAG         "arguments"
#define UBUS_ATTRSTR_TAG        "attribute"
#define UBUS_VLOBJ_TAG          "value"
#define UBUS_EPINT_TAG          "ep"

#define UBUS_TYPERP_VAL         "reportAttribute"
#define UBUS_TYPESET_VAL        "setAttribute"
#define UBUS_TYPECMDRT_VAL      "cmdResult"
#define UBUS_TYPECMD_VAL        "cmd"

#define UBUS_CLOUD_TAG          "CLOUD"
#define UBUS_DEVSTR_VAL         "GATEWAY"
#define UBUS_GPSTR_VAL          "GREENPOWER"


typedef enum 
{
	EGRIB_MSG_NONE= 0x00,
	EGRIB_MSG_RES = 0x01,
	EGRIB_MSG_REQ = 0x02,
}eGribMsg_t;


json* makeGribProto(const char *cmdStr, const char *didStr, json *fArray);

int parseGribProto(json *root, char cmdStr[], char didStr[], json **fArray);

int parseUbusProto(json *root, char dev[], char type[], int *ep, char mac[], char att[], json **value);

json* makeUbusProto(const char from[], const char to[], const char mac[],  const char cmdType[], json *dataPtr);

json *makeUbusCmdDataJson(const char attr[], const char mac[], int ep, json *vlJson, const char id[]);


#endif

