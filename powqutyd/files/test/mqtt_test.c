/*
 * mqtt_test.c
 *
 *  Created on: Jun 16, 2016
 *      Author: neez
 */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "mqtt.h"

static volatile int stop_main = 0;

void handle_signal()
{
	stop_mosquitto();
	stop_main = 1;
}


int main (int argc, char *argv[]) {
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	char c, msg[200];
	printf("powqutyd ...\n");
	if(!mqtt_init()) {
		printf("MQTT started x=)\n");
	} else {
		printf("couldn't start MQTT-Thread");
	}

	while(!stop_main) {
		printf("Type m for sending a message, or x for exit\n\tyour choice: ");
		scanf("%c",&c);
		switch (c) {
			case 'm':
				printf("\nPlease Type your Message: \n\t: ");
				scanf("%s",msg);
				printf("\nSending Message .....  \n");
				mqtt_publish_msg(msg);
				printf("Sending Message .....  Done\n");
				break;
			case 'x':
				handle_signal();
				break;
			default:
				break;
		}

	}
	return 0;
}

