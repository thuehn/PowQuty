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
#include "calculation.h"

static volatile int stop_main = 0;

void handle_signal()
{
	stop_mosquitto();
	stop_calculation();
	stop_main = 1;
	publish_device_offline();
}


int main (int argc, char *argv[]) {
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	PQ_ERROR err = PQ_NO_ERROR;
	printf("powqutyd ...\n");
	if(!mqtt_init()) {
		printf("MQTT Thread started \n");
	} else {
		printf("couldn't start MQTT-Thread\n");
		// return -1;
	}


	if(!calculation_init()) {
		printf("Calculation Thread started\n");
		publish_device_online();
	}

	while (!stop_main){

	}

	return err;
}

