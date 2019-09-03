

#ifndef _GRIB_GATEWAY_PROTO_H_
#define _GRIB_GATEWAY_PROTO_H_


#include "util.h"


#define ATTR_GATEWAY_REBOOT             "gateway.reboot"
#define ATTR_GATEWAY_FACTORY_RESET      "gateway.facorty_reset"
#define ATTR_GATEWAY_STATUS             "gateway.status"
#define ATTR_GATEWAY_UPGRADE_FIRMWARE   "gateway.upgrade_firmware"
#define ATTR_GATEWAY_CHANGE_SEVER       "gateway.change_server"

#define GATEWAY_TAG						"gateway"



json* grib_gateway_regJson(char didStr[]);


#endif
