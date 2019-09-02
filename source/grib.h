

#ifndef _GRIB_H_
#define _GRIB_H_


#include <mosquitto.h>


int grib_gateway_reqReg(struct mosquitto *mosq);

int grib_gateway_statusSet(struct mosquitto *mosq);

int grib_test_reqReg(struct mosquitto *mosq);


#endif

