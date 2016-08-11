/*
 * main.c
 *
 *  Created on: Jun 16, 2016
 *      Author: neez
 */
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "PQ_App.h"
#include "mqtt.h"
#include "retrieval.h"
#include "config.h"

static volatile int stop_main = 0;

void handle_signal()
{
	stop_mosquitto();
	stop_retrieval();
	stop_main = 1;
}


int main (int argc, char *argv[]) {
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	PQ_ERROR err = PQ_NO_ERROR;
	printf("powqutyd ...\n");
	if(!mqtt_init()) {
		printf("MQTT started \n");
	} else {
		printf("couldn't start MQTT-Thread\n");
		// return -1;
	}

	if(!retrieval_init(device_tty)) {
		printf("Retrieval Thread started \n");
	} else {
		printf("couldn't start Retrieval-Thread\n");
		// return -1;
	}

	while (!stop_main){

	}

	return err;
}

