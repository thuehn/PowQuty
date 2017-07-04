/*
 * event_handling.c
 *  Created on Jun 27, 2017
 *  	Author: ikstream
 */
#include <stdio.h>
#include <stdlib.h>

#include "event_handling.h"
#include "libwebslack.h"

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

#ifdef SLACK
	if (conf->slack_notification) {
		struct team_info *ti = malloc(sizeof(struct team_info));
		if (set_webhook_url(ti, conf->slack_webhook)) {
			printf("Could not set webhook: %s\n",
				conf->slack_webhook);
			return;
		}
		if (set_channel(ti, conf->slack_channel)) {
			printf("Could not set slack channel: %s\n",
				conf->slack_channel);
			return;
		}
		if (set_username(ti, conf->slack_user)) {
			printf("Could not set username: %s\n",
				conf->slack_user);
			return;
		}
		if (set_message(ti, msg)) {
			printf("Could not set message: %s\n", msg);
			return;
		}
		if (send_message(ti)) {
			printf("Could not send message\n");
			return;
		}
	}
#endif /* Slack */
}

void handle_event(PQResult pqResult, struct powquty_conf *conf) {
	int i;

	for (i = 0; i < pqResult.nmbPqEvents; i++)
		send_event(pqResult.pqEvents[i], conf);
}
