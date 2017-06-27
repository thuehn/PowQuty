/*
 * event_handling.c
 *  Created on Jun 27, 2017
 *  	Author: ikstream
 */
#include <stdio.h>
#include <stdlib.h>

#include "event_handling.h"

#define MAX_MSG_LENGTH 1024
#define MAX_EVENT_LENGTH 64

void send_event(PQEvent pqe, struct powquty_conf *conf) {
	char *msg = malloc(sizeof(char) * MAX_MSG_LENGTH);
	char *event = malloc(sizeof(char) * MAX_EVENT_LENGTH);

	switch (pqe.type) {
		case (int)PQ_EVENT_TYPE_DIP:
			snprintf(event, MAX_EVENT_LENGTH, "Voltage dip >= 10%%");
			break;
		case (int)PQ_EVENT_TYPE_SWELL:
			snprintf(event, MAX_EVENT_LENGTH, "Voltage above 110%%");
			break;
		case (int)PQ_EVENT_TYPE_INTERRUPT:
			snprintf(event, MAX_EVENT_LENGTH, "Voltage dip < 10%%");
			break;
		case (int)PQ_EVENT_TYPE_HARMONIC:
			snprintf(event, MAX_EVENT_LENGTH, "Harmonic off more"
				 "than 5%% of the time");
			break;
		default:
			break;
	}

	/* prepare msg to send */
	snprintf(msg, MAX_MSG_LENGTH, "%s", event);
}

void handle_event(PQResult pqResult, struct powquty_conf *conf) {
	int i;

	for (i = 0; i < pqResult.nmbPqEvents; i++)
		send_event(pqResult.pqEvents[i], conf);
}
