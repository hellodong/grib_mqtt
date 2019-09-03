

#ifndef _GRIB_PROTO_COMMON_H_
#define _GRIB_PROTO_COMMON_H_


#include "util.h"


#define GRIB_CMD_KEY_REQ        "req"
#define GRIB_CMD_KEY_RES        "res"
#define GRIB_MSGOBJ_KEY         "msg"
#define GRIB_TID_KEY            "tid"
#define GRIB_FNLARRAY_KEY       "fnl"
#define GRIB_FN_KEY             "fn"
#define GRIB_TP_KEY             "tp"

#define GRIB_CMD_VAL_REQ        "Reg"

#define GRIB_PALTFORMREG_TOPIC  "PLATFORM/Reg"

#define GRIB_TAG				"grib"

#define GRIB_TOPIC_REG			"_Reg"
#define GRIB_TOPIC_EXE			"_Exe"
#define GRIB_TOPIC_DEL			"_Del"

typedef enum 
{
	EGRIB_MSG_NONE= 0x00,
	EGRIB_MSG_RES = 0x01,
	EGRIB_MSG_REQ = 0x02,
}eGribMsg_t;


json* makeGribProto(const char *cmdStr, const char *didStr, json *fArray);

int parseGribProto(json *root, char cmdStr[], char didStr[], json **fArray);


#endif
