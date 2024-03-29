

#ifndef _GRIB_GATEWAY_PROTO_H_
#define _GRIB_GATEWAY_PROTO_H_


#define ATTR_GATEWAY_REBOOT             "gateway.reboot"
#define ATTR_GATEWAY_FACTORY_RESET      "gateway.facorty_reset"
#define ATTR_GATEWAY_STATUS             "gateway.status"
#define ATTR_GATEWAY_UPGRADE_FIRMWARE   "gateway.upgrade_firmware"
#define ATTR_GATEWAY_CHANGE_SEVER       "gateway.change_server"
#define ATTR_GATEWAY_REMOTE_SHELL       "gateway.remote_shell"

#define GATEWAY_TAG                     "GATEWAY"

#define GATEWAY_STATUS_VERSION          "version"
#define GATEWAY_STATUS_MOD              "model"
#define GATEWAY_STATUS_FAC              "factory"
#define GATEWAY_STATUS_CTIME            "current_time"
#define GATEWAY_STATUS_UPTIME           "uptime"  
#define GATEWAY_STATUS_UPLINKT          "uplinkType"  
#define GATEWAY_STATUS_VAL              "status"

#define GRIB_GATEWAY_FNSET_VERSION_VAL   "Version"
#define GRIB_GATEWAY_FNSET_MODEL_VAL     "Model"
#define GATEWAY_FNEXE_FWUPGD_VAL         "FirmwareUpgrade"
#define GATEWAY_FNEXE_REBOOT_VAL         "Reboot"
#define GATEWAY_FNEXE_REMOTESH_VAL       "RemoteSh"


int grib_gateway_exeTopic(char topic[]);

int grib_gateway_tidCmp(const char tidStr[]);

json* grib_gateway_covt_ubus(json *gribFnlArray);

json* ubus_gateway_covt_grib(const char typeStr[], const char attStr[], json *jsonVal);


#endif
