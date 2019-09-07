

#ifndef _GRIB_ZB3_PROTO_H_
#define _GRIB_ZB3_PROTO_H_


#define GATEWAY_STATUS_VERSION          "version"
#define GATEWAY_STATUS_MOD              "model"
#define GATEWAY_STATUS_FAC              "factory"
#define GATEWAY_STATUS_CTIME            "current_time"
#define GATEWAY_STATUS_UPTIME           "uptime"  
#define GATEWAY_STATUS_UPLINKT          "uplinkType"  
#define GATEWAY_STATUS_VAL              "status"


#define GRIB_ZB3_FNEXESET_ADD_VAL       "Add"
#define GRIB_ZB3_FNSET_VERSIONSTR_VAL   "Version"
#define GRIB_ZB3_FNSET_MODELSTR_VAL     "Model"
#define GRIB_ZB3_FNSET_ONLINEINT_VAL    "Online"
#define GRIB_ZB3_FNSET_BATINT_VAL       "Battery"


json* grib_zb3_covt_ubus(json *gribFnlArray);

json* ubus_zb3_covt_grib(const char typeStr[], const char attStr[], json *jsonVal);


#endif
