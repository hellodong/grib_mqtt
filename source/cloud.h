#ifndef CLOUD_H
#define CLOUD_H
#include <stdbool.h>
#include <stdint.h>


bool cloud_init(const char * mac, const char * server, int port);
bool cloud_run();
bool cloud_forReg();
uint32_t mqtt_pubed_time(void);
uint8_t mqtt_loopThAlive(void);
void networkCheck(void *arg);
int waitForCurTime(uint32_t waitTime);

#endif
