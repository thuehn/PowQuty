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
#include "file_handling.h"
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
	printf("DEBUG:\tThreads should have stopped\n");
	stop_main = 1;
}

void stop_powqutyd() {
	handle_signal();
}

void stop_powqutyd_file_read() {
#ifdef MQTT
	stop_mosquitto();
#endif
	stop_file_read();
	printf("DEBUG:\tThreads should have stopped\n");
	stop_main = 1;
}

void handle_args (int argc, char **argv) {
	int c;
	while ((c = getopt (argc, argv, "rdf:")) != -1) {
		switch (c) {
			case 'r':
				set_raw_print(1);
				break;
			case 'd':
				set_debug(1);
				break;
			case 'f':
				set_file_read(optarg);
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
	int	conf_res = uci_config_powquty(&conf);
	printf("UCI CONFIG returned %d\n",conf_res);

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

	printf("arguments handled\n");
	if (get_input_file_state()) {
		printf("use input file\n");
//		sleep(10);
		if (!file_read_init(&conf)) {
#ifdef MQTT
			publish_device_online();
			publish_device_gps();
#endif
		}
		while (!stop_main) {
			join_file_read();
		}
		return 0;
	}
	printf("not input_file\n");
	if(!calculation_init(&conf)) {
		printf("Calculation Thread started\n");
#ifdef MQTT
		publish_device_online();
		publish_device_gps();
#endif
	} else {
#ifdef MQTT
		stop_mosquitto();
#endif
		exit(EXIT_FAILURE);
	}


	while (!stop_main){
		pthread_join(calculation_thread, NULL);


	}


	return 0;
}

