/*
 * main.c
 *
 *  Created on: Jun 16, 2016
 *      Author: neez
 */
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "calculation.h"
#include "config.h"
#include "file_handling.h"
#include "PQ_App.h"
#include "raw_dump.h"
#include "retrieval.h"
#include "uci_config.h"

#ifdef MQTT
#include "mqtt.h"
#endif

static volatile int stop_main = 0;

void handle_signal()
{
#ifdef MQTT
	stop_mosquitto();
#endif
	stop_calculation();
	printf("DEBUG:\t Threads should have stopped\n");
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
	printf("DEBUG:\t Threads should have stopped\n");
	stop_main = 1;
}

void handle_args (int argc, char **argv) {
	int c;
	int opt_idx = 0;

	static struct option long_options[] = {
		{"debug",	no_argument,		0,		'd'},
		{"file",	required_argument,	0,		'f'},
		{"raw",		no_argument,		0,		'r'},
		{"rawfile",	required_argument,	0,		'w'},
		{0,		0,			0,		 0 }
	};

	while ((c = getopt_long (argc, argv, "rdf:w:", long_options,
							&opt_idx)) != -1) {
		switch (c) {
			case 'r':
				set_raw_print(ON);
				break;
			case 'd':
				set_debug(ON);
				break;
			case 'f':
				read_data_from_file(optarg);
				break;
			case 'w':
				set_raw_print(ON);
				dump_raw_to_file(optarg);
				break;
			default:
				break;
		}
	}
}

int main (int argc, char *argv[]) {
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);
	get_device_information = OFF;
	struct powquty_conf conf;
	uci_config_powquty(&conf);

	printf("Starting powqutyd ...\n");
#ifdef MQTT
	if(!mqtt_init(&conf)) {
		printf("DEBUG:\t MQTT Thread started \n");
	} else {
		printf("WARN:\t Couldn't start MQTT-Thread\n");
	}
#endif
	handle_args(argc, argv);

	if (get_input_file_state()) {
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
	if(!calculation_init(&conf)) {
		printf("DEBUG:\t Calculation Thread Initialized\n");
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
		sleep(2);
	}


	return 0;
}

