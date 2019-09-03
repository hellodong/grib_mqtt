

#ifndef _GRIB_H_
#define _GRIB_H_


#include <mosquitto.h>


int grib_reqReg(struct mosquitto *mosq);

int grib_plat_parse(const char dataStr[]);

int grib_test_reqReg(struct mosquitto *mosq);


#endif

