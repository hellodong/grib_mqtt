#ifndef UTIL_H
#define UTIL_H

#include <json-c/json.h>
#include <pthread.h>
#include <stdio.h>


#define LOG(fmt,arg...)		printf("%s() #%d "fmt"\n", __FUNCTION__, __LINE__, ##arg)


typedef  json_object json;

const char * get_timestamp();

time_t get_clock_time(void);

const char * get_time_str(uint32_t ti, char * buf);

int mac2string(char *mac, char *str);

int get_mac_address(char * mac);

///get mac address with ":"
int get_mac_address_compact(char * mac);

char * itoa(int i,  char *buf);

char * get_string_from_file(const char * path,  char * out_buf);

void save_string_to_file(const char * path, char  * out_buf);

int create_directory(const char * path);

char * get_firmware_version(char * out_buf);

int get_board_model(char * out_model);

int download_file(const char * url, const char * path);

int get_file_size(const char * path);

void write_sys_attribute(char * path, char * value);

//json related
json * json_new(void);

json* json_new_array(void);

json * json_parse(const char * str, int len);

void  json_put(json * obj);

const char * json_to_str(json * obj);

void json_add_str(json * root, const char * key, const char * val);

void json_add_int(json * root, const char * key, int val);

void json_add_obj(json * root, const char * key, json * obj);

void json_array_add_obj(json *root, json* obj);

void json_del_key(json *root, const char * key);

const char * json_get_str(json * root, const char * key);

const char * json_get_str_ex(json * root, const char * key, char * buf);

int json_get_int(json * root, char * key);

json * json_get_obj(json * root, const char * key);


void led_on(char * led);

void led_off(char * led);

void led_shot(char * led);

void led_blink(char * led);

int mutex_init(pthread_mutex_t * lock);
#endif
