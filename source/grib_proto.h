

#ifndef _GRIB_PROTO_H_
#define _GRIB_PROTO_H_


#include <mosquitto.h>


#define GRIB_CMD_KEY_REQ        "req"
#define GRIB_CMD_KEY_RES        "res"
#define GRIB_MSGOBJ_KEY         "msg"
#define GRIB_TID_KEY            "tid"
#define GRIB_FNLARRAY_KEY       "fnl"
#define GRIB_FN_KEY             "fn"
#define GRIB_TP_KEY             "tp"

#define GRIB_CMD_VAL_REQ        "Reg"

#define GRIB_PALTFORMREG_TOPIC  "PLATFORM/Reg"


int grib_test_reqReg(struct mosquitto *mosq);


#endif

