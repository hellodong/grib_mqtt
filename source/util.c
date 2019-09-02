#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>


#include "util.h"


//#define DEVICEMAC_FILE  "/sys/class/net/eth0/address"
#define DEVICEMAC_FILE  "/sys/class/net/ens33/address"

const char * get_timestamp()
{
	static char buf[40];
	time_t now = time(NULL);

	struct tm * tmnow = localtime(&now);

	sprintf(buf, "%04d%02d%02dT%02d%02d%02d", tmnow->tm_year+1900, tmnow->tm_mon + 1, tmnow->tm_mday
		, tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);

	return buf;
}


const char * get_time_str(uint32_t ti, char * buf)
{
	time_t now = ti;

	struct tm * tmnow = localtime(&now);

	sprintf(buf, "%04d%02d%02dT%02d%02d%02d", tmnow->tm_year+1900, tmnow->tm_mon + 1, tmnow->tm_mday
		, tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);

	return buf;
}

int get_mac_address(char * mac)
{
	FILE * fp = fopen(DEVICEMAC_FILE, "r");
	fgets(mac, 20, fp);
	int len = strlen(mac);

	if (mac[len-1] == '\n')
		mac[len-1] = '\0';

	fclose(fp);
	return strlen(mac);
}

int get_mac_address_compact(char * mac)
{
	char buf[100];
	mac[0] = '\0';
	get_mac_address(buf);
	char * p = strtok(buf, ":");
	while(p) {
		strcat(mac, p);
		p = strtok(NULL, ":");
	}
	return strlen(mac);
}

char * get_string_from_file(const char * path,  char * out_buf)
{
	char buf[1024];
	buf[0] = '\0';
	out_buf[0] = '\0';
	FILE * fp = fopen(path, "r");
	if (fp == NULL)
		return out_buf;

	fgets(buf, 1024, fp);
	fclose(fp);

	int len = strlen(buf);
	while(isspace(buf[len-1]) )
	{
		buf[len-1] = '\0';
		len--;
	}
	strcpy(out_buf, buf);
	return out_buf;
}

void save_string_to_file(const char * path, char  * out_buf)
{
	FILE * fp  = fopen(path, "w");
	fputs(out_buf, fp);
	fclose(fp);
}

char * get_firmware_version(char * out_buf)
{
	char buf[1024];
	get_string_from_file("/etc/dusun_build", buf);
	strcpy(out_buf,  buf + strlen("BUILD_VERSION=") );
	return out_buf;
}

int get_file_size(const char * path)
{
	struct stat buf;
	if (stat(path, &buf) == 0 )
	{
		return buf.st_size;
	} else
		return -1;
}

int create_directory(const char * path)
{
	return mkdir(path, 0700);
}

int get_board_model(char * out_model)
{
	get_string_from_file("/tmp/sysinfo/model", out_model);
	int len = strlen(out_model);
	if (len >= 32) {
		out_model[32] = 0;
	}
	return strlen(out_model);
}

//JSON realted
json * json_new(void)
{
	return json_object_new_object();
}

json* json_new_array(void)
{
    return json_object_new_array();
}

void json_put(json * obj)
{
	json_object_put(obj);
}

const char * json_to_str(json * obj)
{
	return json_object_to_json_string(obj);
}

void json_add_str(json * root, const char * key, const char * val)
{
	json_object_object_add(root, key, json_object_new_string(val));
}

void json_add_int(json * root, const char * key, int val)
{
	json_object_object_add(root, key, json_object_new_int(val));
}

void json_add_obj(json * root, const char * key, json * obj)
{
	json_object_object_add(root, key, obj);
}

void json_array_add_obj(json *root, json* obj)
{
    json_object_array_add(root, obj);
}

void json_del_key(json *root, const char * key) {
	json_object_object_del(root, key);
}

const char * json_get_str(json * root, const char * key)
{
	const char * str = NULL;
	json * child;
	if (json_object_object_get_ex(root, key, &child)) {
		str = json_object_get_string(child);
	}
	return str;
}

const char * json_get_str_ex(json * root, const char * key, char * buf)
{
	const char * val = NULL;
	buf[0] = '\0';
	val = json_get_str(root, key);
	if (val != NULL )
		strcpy(buf, val);
	return val;
}


int json_get_int(json * root, char * key)
{
	int i = -1;
	json * child;
	if (json_object_object_get_ex(root, key, &child)) {
		i = json_object_get_int(child);
	}
	return i;
}


json * json_get_obj(json * root, const char * key)
{
	json * retObj = NULL;
	json_object_object_get_ex(root, key, &retObj);
	return retObj;
}

json * json_parse(const char * str, int len) {
	struct json_tokener* tok;
	struct json_object* obj;

	if (len < 0 )
		len = strlen(str);

	tok = json_tokener_new();
	if (!tok)
		return NULL;
	obj = json_tokener_parse_ex(tok, str, len);

    if(tok->err != json_tokener_success) {
                if (obj != NULL)
                        json_object_put(obj);
        obj = NULL;
    }
	json_tokener_free(tok);
	return obj;
}

time_t get_clock_time(void)
{
	struct timespec ts;
	//clock_gettime(CLOCK_MONOTONIC, &ts);
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec;
}

void write_sys_attribute(char * path, char * value)
{
	FILE * fp = fopen(path, "w");
    if (!fp)
        return;
    fputs(value, fp);
    fclose(fp);
}

//write value to led attribute
int  write_led_attribute(char * led, char * att, char * value)
{
    char path[100];
    char str[20];
    sprintf(path, "/sys/class/leds/%s/%s", led, att);
    sprintf(str, "%s\n", value);
    FILE * fp = fopen(path, "w");
    if (!fp)
        return -1;

    fputs(str, fp);
    fclose(fp);
    return 0;
}

void led_on(char * led)
{
	write_led_attribute(led, "trigger", "none");
	write_led_attribute(led, "brightness", "1");
}

void led_off(char * led)
{
	write_led_attribute(led, "trigger", "none");
	write_led_attribute(led, "brightness", "0");
}

void led_blink(char * led)
{
	write_led_attribute(led, "trigger", "timer");
}

void led_shot(char * led)
{
	write_led_attribute(led, "trigger", "oneshot");
	write_led_attribute(led, "shot", "1");
}


int mutex_init(pthread_mutex_t * lock)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	return pthread_mutex_init(lock, &attr);
}
