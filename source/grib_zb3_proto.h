

#ifndef _GRIB_ZB3_PROTO_H_
#define _GRIB_ZB3_PROTO_H_


#define UBUS_ZB3_DEVLIST_ATTR           "mod.device_list"
#define UBUS_ZB3_DEVADDED_ATTR          "mod.new_device_added"
#define UBUS_ZB3_DEVSTUS_ATTR           "device.status"
#define UBUS_ZB3_ZONE_ATTR              "device.zone_status"
#define UBUS_ZB3_METERENERGY_ATTR       "device.meter.energy"
#define UBUS_ZB3_METERPOWER_ATTR        "device.meter.power"
#define UBUS_ZB3_METERCURRENT_ATTR      "device.meter.current"
#define UBUS_ZB3_METERVOLTAGE_ATTR      "device.meter.voltage"
#define UBUS_ZB3_ONOFF_ATTR             "device.onoff"
#define UBUS_ZB3_SWITCH_ATTR            "device.switch.onoff"
#define UBUS_ZB3_LOCK_PASSWDDEL_ATTR    "device.lock.del_password"

#define ZB3_DEVLST_TAG        "device_list"
#define ZB3_TYPE_TAG          "type"
#define ZB3_MAC_TAG           "mac"
#define ZB3_BAT_TAG           "battery"
#define ZB3_ONLINE_TAG        "online"
#define ZB3_VERSION_TAG       "version"
#define ZB3_MODSTR_TAG        "ModelStr"
#define ZB3_VALUE_TAG         "value"
#define ZB3_EP_TAG            "ep"
#define ZB3_ZONE_TAG          "zone"

#define GRIB_ZB3_FNSET_VERSIONSTR_VAL   "Version"
#define GRIB_ZB3_FNSET_MODELSTR_VAL     "Model"
#define GRIB_ZB3_FNSET_ONLINEINT_VAL    "Online"
#define GRIB_ZB3_FNSET_BATINT_VAL       "Battery"
#define GRIB_ZB3_FNSET_LEAKSTUS_VAL     "LeakStatus"
#define GRIB_ZB3_FNSET_DOORSTUS_VAL     "DoorStatus"
#define GRIB_ZB3_FNSET_PIRSTUS_VAL      "PIRStatus"
#define GRIB_ZB3_FNSET_ENERGYKWH_VAL    "Energy_Kwh"
#define GRIB_ZB3_FNSET_POWERKW_VAL      "Power_Kw"
#define GRIB_ZB3_FNSET_CURRENTA_VAL     "Current_A"
#define GRIB_ZB3_FNSET_ONOFF_VAL        "Onoff"
#define GRIB_ZB3_FNSET_SWITCH_VAL       "Switch"
#define GRIB_ZB3_FNSET_PASSWDADD_VAL    "PasswdAdd"
#define GRIB_ZB3_FNSET_PASSWDDEL_VAL    "PasswdDel"


void grib_zb3_init(void);

json* grib_zb3_covt_ubus(json *gribFnlArray, const char *tidStr);

json* ubus_zb3_covt_grib(const char typeStr[], const char macStr[], const char attStr[], json *jsonVal);


#endif
