#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <mosquitto.h>
#include <dusun/dusun.h>
#include <errno.h>

#include "cloud.h"
#include "mqgwHnd.h"
#include "timer.h"
#include "system.h"
#include "grib.h"
#include "grib_gateway_proto.h"


#define NETWORKFAILED   "/tmp/.network_check_failed"
#define CONNECTED_PATA  "/tmp/.mq_connected"

#define MQTTUSRNAME     "dhkswl"
#define MQTTPASSWD      "@65$@"


extern bool report_gateway_status();
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


static void buildConnectedFlag(void)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "touch /tmp/.mq_connected");
    system(cmd);
}

static void destroyConnectedFlag(void)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "rm /tmp/.mq_connected");
    system(cmd);
}


static void corrctTimeRptStatus(void *arg)
{
    int retFunc, ret = 1;
    int *retPtr = (int *)arg;
    retFunc = waitForCurTime(1);
    if (0 == retFunc){
        report_gateway_status();
        ret = 0;
    }else{
        LOG("No Correct Time.");
        timer_add(corrctTimeRptStatus, 10);
    }
    if (retPtr){
        *retPtr = ret;
    }
}

static void mqtt_on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	LOG("mqtt_on_connect rc=%d", rc);
	if (rc == 0 ) {
		LOG("mqtt_on_connected");
		cb.connected = true;
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
        grib_gateway_exeTopic(topic_x);;
		mosquitto_subscribe(cb.mqtt, NULL, topic_x, 1);
	    cb.gateway_status_reported = true;
        report_gateway_status();
#endif
	    led_off("errled");
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

    destroyConnectedFlag();
	led_on("errled");
}

static void mqtt_on_publish(struct mosquitto *mosq, void *obj,
							int mid)
{
	LOG("mqtt msg pubed mid=%d", mid);
    cb.pubedTime = get_clock_time();
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
	LOG("MQTT(%d): %s", len, data);
    json *packet = NULL;
    packet = grib_plat_parse(data);
    proto_handle_packet(packet);
    json_put(packet);
#endif
}


#if 0
static int mqtt_publish(const uint8_t * data, int len)
{
	int mid;
	lock();
	mosquitto_publish(cb.mqtt, &mid, "t", len, data, 1, 0);
	unlock();
	LOG("mqtt_pub mid=%d", mid);
	return mid;
}
#else
static int mqtt_publish(const char topic[], const uint8_t * data, int len)
{
	int mid;
	lock();
	mosquitto_publish(cb.mqtt, &mid, topic, len, data, 1, 0);
	unlock();
	LOG("mqtt_pub mid=%d", mid);
	return mid;
}
#endif

static int mqtt_publish_json(json * root)
{
    json *gribJson;
	const char * content = json_to_str(root);

#if 0
	LOG("send message: %s", content);
	int mid = mqtt_publish((const uint8_t *)content, strlen(content));
	return mid;
#else
    char topic[128];
	LOG("Conv MSG: %s", content);
    gribJson = ubus_conv2_grib(root, topic);
    if (NULL == gribJson){
        LOG("grib Json Error");
        return 0;
    }
    content = json_to_str(gribJson);
	LOG("send message(%s): %s",topic, content);
    int mid = mqtt_publish(topic, (const uint8_t *)content, strlen(content));
    json_put(gribJson);
	return mid;
#endif
}

int cloud_send_json(json * root)
{
	return mqtt_publish_json(root);
}

static bool connect_mqtt()
{
    int retFunc;

	int keepalive = system_get_mqtt_keepalive();
	LOG("mqtt keep alive is %d\n", keepalive);

	char mqtt_name[128];
	char mqtt_pass[128];
	if (system_get_mqtt_username_password(mqtt_name, mqtt_pass) != 0) {
	    sprintf(mqtt_name, "%s", MQTTUSRNAME);
        sprintf(mqtt_pass, "%s", MQTTPASSWD);
		system_set_mqtt_username_password(mqtt_name, mqtt_pass);
	} 
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
       cb.threadId = mosquitto_loop_threadId(cb.mqtt);
       LOG("Mosquitto Loop thread:%ld", cb.threadId);
    }
    return !!retFunc;
}

bool cloud_init(const char * mac, const char * server, int port)
{
    memset(&cb, 0, sizeof(cb));
    mutex_init(&cb.lock);
    get_mac_address(cb.gateway_mac);
    //sprintf(topic_x, "%s", "PLATFORM/_Exe");
    //LOG("subcrible to [%s]", topic_x);
    strcpy(cb.server, server);
    cb.port = port;
    cb.inited = true;
    mosquitto_lib_init();
    cb.mqtt = mosquitto_new(cb.gateway_mac, true, NULL);

    led_on("errled");
    return true;
}

bool cloud_run()
{
    return connect_mqtt();
}

bool cloud_forReg()
{
    bool ret = false;
    if (true == cb.connected) {
		if (proto_gw_is_registered() == false) {
			proto_gw_send_register_request();
        }else{
            ret = true;
        }
    }
    return ret;
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

/*
 * brif: 查看网络是否通
 * para: arg -- 传入参数值
 * retn: None
 * see :
 * note: 网络会断开，尤其是2G/3G连接时网络会自动断开，产生虚连接
 */
void networkCheck(void *arg)
{
    if(false == cb.connected && access(NETWORKFAILED, F_OK)) {
        char dnsStr[64] = "8.8.8.8";
        char cmdStr[128];
        int retFunc;

        LOG("ping %s", dnsStr);
        snprintf(cmdStr, sizeof(cmdStr), "ping -W 3 -w 3 %s | grep ttl", dnsStr);
        retFunc = system(cmdStr);
        if (0 != retFunc){
            LOG("ping failed");
            if (++cb.netWorkFailedCount >= 3) {
                LOG("Network Failed");
                snprintf(cmdStr, sizeof(cmdStr), "touch %s", NETWORKFAILED);
                system(cmdStr);
                cb.netWorkFailedCount = 0;
            }
        }else{
            LOG("ping OK");
            cb.netWorkFailedCount = 0;
        }
    }else if (true == cb.connected) {
        cb.netWorkFailedCount = 0;
    }
    timer_add(networkCheck, 60);
}

int waitForCurTime(uint32_t waitTime)
{
    uint32_t nowTime;
    uint32_t over = 1532571500;
    waitTime = waitTime < 1 ? 1:waitTime;

    LOG("Start Waiting...");
    for(;waitTime > 0; waitTime--) {
        nowTime = get_clock_time();
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

void cloud_dump()
{
	char buf[4096];
	char * p = buf;
	lock();
	p += sprintf(p, "###CLOUD DUMP:\n");
	p += sprintf(p, "inited: %d\n", cb.inited);
	p += sprintf(p, "gw_mac: %s\n", cb.gateway_mac);
	p += sprintf(p, "port: %d\n", cb.port);
	p += sprintf(p, "connected: %d\n", cb.connected);
	unlock();
	printf("%s", buf);
}

