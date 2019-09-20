

#ifndef _GRIB_H_
#define _GRIB_H_


#include <mosquitto.h>
#include "dusun/dusun.h"


int grib_reqReg(struct mosquitto *mosq);

void grib_start_init(void);

json* grib_plat_parse(const char dataStr[]);

json* ubus_conv2_grib(json *ori, char topic[]);

int grib_test_reqReg(struct mosquitto *mosq);


#endif

