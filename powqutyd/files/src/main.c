/*
 * main.c
 *
 *  Created on: Jun 16, 2016
 *      Author: neez
 */
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "PQ_App.h"
#ifdef MQTT
#include "mqtt.h"
#endif
#include "calculation.h"
#include "retrieval.h"
#include "config.h"
#include "uci_config.h"

static volatile int stop_main = 0;

void handle_signal()
{
#ifdef MQTT
	stop_mosquitto();
#endif
	stop_calculation();
	printf("DEBUG:\tThreads should have stopped \n");
	stop_main = 1;
}

void stop_powqutyd() {
	handle_signal();
}

void handle_args (int argc, char **argv) {
	int c;
	while ((c = getopt (argc, argv, "rd")) != -1) {
		switch (c) {
			case 'r':
				set_raw_print(1);
				break;
			case 'd':
				set_debug(1);
				break;
			default:
				break;
		}
	}
}

int main (int argc, char *argv[]) {
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);
	// char* config_file = "/etc/powqutyd/powqutyd.cfg";

	/*if(load_config(config_file)){
		printf("Error: could not load some config from %s\n", config_file);
		// return -1;
	}*/

	struct powquty_conf conf;
	uci_config_powquty(&conf);
	//printf("UCI CONFIG FTW!!!");

	// PQ_ERROR err = PQ_NO_ERROR;

	printf("Starting powqutyd ...\n");
#ifdef MQTT
	if(!mqtt_init(&conf)) {
		printf("MQTT Thread started \n");
	} else {
		printf("couldn't start MQTT-Thread\n");
		// return -1;
	}
#endif
	handle_args(argc, argv);

	if(!calculation_init(&conf)) {
		printf("Calculation Thread started\n");
#ifdef MQTT
		publish_device_online();
		publish_device_gps();
#endif
	} else {
		stop_mosquitto();
		exit(EXIT_FAILURE);
	}


	while (!stop_main){
		join_calculation();

	}


	return 0;
}

