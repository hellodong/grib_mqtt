#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <mosquitto.h>
#include <errno.h>
#include <pthread.h>

#include "cloud.h"
#include "util.h"
#include "grib.h"


#define MQTTUSRNAME     "dhkswl"
#define MQTTPASSWD      "@65$@"


typedef struct _CLOUD_CONTROL_BLOCK
{
	bool inited;
    uint32_t pubedTime;
	char gateway_mac[20];
	char server[200];
	int  port;
	bool connected;
    uint16_t netWorkFailedCount;
	bool gateway_status_reported;
	struct mosquitto * mqtt;
	pthread_mutex_t lock;
    pthread_t threadId;
} cloud_cb;

static cloud_cb cb;

static void lock()
{
	pthread_mutex_lock(&cb.lock);
}

static void unlock()
{
	pthread_mutex_unlock(&cb.lock);
}
static char topic_x[128];


static void mqtt_on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	LOG("mqtt_on_connect rc=%d", rc);
	if (rc == 0 ) {
		LOG("mqtt_on_connected");
		cb.connected = true;
		mosquitto_subscribe(cb.mqtt, NULL, topic_x, 1);
#if 0
        if (proto_gw_is_registered() == true) {
            int ret;
			cb.gateway_status_reported = true;
            corrctTimeRptStatus((void *)&ret);
            if (ret){
                report_gateway_status();
            }
        }
#else
		grib_reqReg(cb.mqtt);
        //grib_test_reqReg(cb.mqtt);
#endif
	} else if (rc == 4  || rc == 2) {
		LOG("username/password error, exit");
		exit(1);
	} else {
		cb.connected = false;
		cb.gateway_status_reported = false;
	}
}

static void mqtt_on_disconnect(struct mosquitto *mosq, void *obj,
								int rc)
{
	LOG("mqtt_on_disconnect %d, errno:%s", rc, strerror(errno));
	cb.connected = false;
	cb.gateway_status_reported = false;

}

static void mqtt_on_publish(struct mosquitto *mosq, void *obj,
							int mid)
{
	LOG("mqtt msg pubed mid=%d", mid);
    cb.pubedTime = time(NULL);
}

static void mqtt_on_message(struct mosquitto *mosq, void *obj,
						const struct mosquitto_message * message)
{
	int len = message->payloadlen;
	char * data = message->payload;
#if 0
	data[len-1] = '\0';

	LOG("MQTT: %s", data);
	json * packet = json_parse(data, -1);
    if (mq_setUtc(packet) == 0){
	    proto_handle_packet(packet);
    }else{
	    LOG("Set UTC Done.");
    }
	json_put(packet);

	if (!cb.gateway_status_reported && proto_gw_is_registered()) {
		cb.gateway_status_reported = true;
        corrctTimeRptStatus(NULL);
	}
#else
	LOG("MQTT(%d): %s",len, data);
	grib_plat_parse(data);
#endif
}

static int mqtt_publish(const uint8_t * data, int len)
{
	int mid;
	lock();
	mosquitto_publish(cb.mqtt, &mid, "t", len, data, 1, 0);
	unlock();
	LOG("mqtt_pub mid=%d", mid);
	return mid;
}

static int mqtt_publish_json(json * json)
{
	const char * content = json_to_str(json);
	LOG("send message: %s", content);
	int mid = mqtt_publish((const uint8_t *)content, strlen(content));
	return mid;
}

int cloud_send_json(json * json)
{
	return mqtt_publish_json(json);
}

static bool connect_mqtt()
{
    int retFunc;

	int keepalive = 120;
	LOG("mqtt keep alive is %d\n", keepalive);

	char mqtt_name[128];
	char mqtt_pass[128];
	sprintf(mqtt_name, "%s", MQTTUSRNAME);
    sprintf(mqtt_pass, "%s", MQTTPASSWD);
    LOG("username : %s, passwd:%s", mqtt_name, mqtt_pass);
#if 0
	mosquitto_username_pw_set(cb.mqtt, username, username);
#else
	mosquitto_username_pw_set(cb.mqtt, mqtt_name, mqtt_pass);
#endif
  
	mosquitto_reconnect_delay_set(cb.mqtt, 5,300, false);
	mosquitto_connect_callback_set(cb.mqtt, mqtt_on_connect);
	mosquitto_disconnect_callback_set(cb.mqtt, mqtt_on_disconnect);
	mosquitto_publish_callback_set(cb.mqtt, mqtt_on_publish);
	mosquitto_message_callback_set(cb.mqtt, mqtt_on_message);
	mosquitto_connect_async(cb.mqtt, cb.server, cb.port, keepalive);
	retFunc = mosquitto_loop_start(cb.mqtt);
    if (!retFunc) {
       LOG("Mosquitto Loop thread:%d", retFunc);
    }
    return !!retFunc;
}

bool cloud_init(const char * mac, const char * server, int port)
{
    memset(&cb, 0, sizeof(cb));
    mutex_init(&cb.lock);

	get_mac_address(cb.gateway_mac);
    sprintf(topic_x, "%s", "PLATFORM/_Exe");
    LOG("subcrible to [%s]", topic_x);
    strcpy(cb.server, server);
    cb.port = port;
    cb.inited = true;
    mosquitto_lib_init();
    cb.mqtt =  mosquitto_new(cb.gateway_mac, true, NULL);

    return true;
}

bool cloud_run()
{
    return connect_mqtt();
}


uint32_t mqtt_pubed_time(void)
{
    return cb.pubedTime;
}

uint8_t mqtt_loopThAlive(void)
{
    int kill_rc;
    uint8_t ret = 1;

    if (!cb.threadId) {
        //printf("No thread id\n");
        LOG("No thread id");
        return ret;
    }

    kill_rc = pthread_kill(cb.threadId,0);
    if(kill_rc == ESRCH){
        //printf("the specified thread did not exists or already quit\n");
        LOG("the specified thread did not exists or already quit");
        ret = 0; 
    }else if(kill_rc == EINVAL){
        //printf("signal is invalid\n");
        LOG("signal is invalid");
    }else {
        //printf("the specified thread is alive\n");
        //LOG("the specified thread is alive");
        ;
    }
    return ret;
}


int waitForCurTime(uint32_t waitTime)
{
    uint32_t nowTime;
    uint32_t over = 1532571500;
    waitTime = waitTime < 1 ? 1:waitTime;

    LOG("Start Waiting...");
    for(;waitTime > 0; waitTime--) {
        nowTime = time(NULL);
        if (nowTime > over) {
            LOG("Over Time");
            return 0;
        }
        LOG("Waitting... now:%d", nowTime);
        sleep(1);
    }
    LOG("Waiting Time Failed");
    return 1;
}

