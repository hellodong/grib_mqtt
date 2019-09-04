#ifndef CLOUD_H
#define CLOUD_H
#include <stdbool.h>
#include <stdint.h>


bool cloud_init(const char * mac, const char * server, int port);
bool cloud_run();
int cloud_send_json(json * json);
void cloud_dump();
bool cloud_forReg();
uint32_t mqtt_pubed_time(void);
uint8_t mqtt_loopThAlive(void);
void networkCheck(void *arg);
int waitForCurTime(uint32_t waitTime);
void nxp_dump();
#endif
