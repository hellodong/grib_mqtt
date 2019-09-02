#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include "cloud.h"


int main(int argc, char * argv[])
{
	char gw_mac[20];
	char * server = NULL;
	int port = 0;
	int cmd = 0;


    if (argc >= 3 ) {
        int ch;
        while((ch = getopt(argc,argv,"h:p:C")) != -1){
            switch(ch){
                case 'p':
                    port = atoi(optarg);
                    break;
                case 'h':
                    server = optarg;
                    break;
                case 'C':
                    cmd = 1;
                    break;
                default:
                    break;
            }
        }
    }

	if (port == 0 || server == NULL){
		server = "interface.jiutengkj.com";
		port = 1883;
	}

    waitForCurTime(50);

	cloud_init(gw_mac, server, port);
	cloud_run();
	while(1) {
		sleep(1);
	}
	return 0;
}
