/*
 * event_handling.c
 *  Created on Jun 27, 2017
 *  	Author: ikstream
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "event_handling.h"
#include "libwebslack.h"
#include "mqtt.h"

#define MAX_MSG_LENGTH		1024
#define MAX_EVENT_LENGTH	64
#define MAX_TIME_LENGTH		64
#define MAX_HOSTNAME_LENGTH 255

void send_event(PQEvent pqe, struct powquty_conf *conf) {
	FILE *file;
	time_t timer;
	struct tm *tmi;
	struct timeval tv;
	int volt_event = 0, harm_event = 0;

	char *msg = malloc(sizeof(char) * MAX_MSG_LENGTH);
	if (msg == NULL) {
		printf("Could not allocate memory for msg in %s\n", __func__);
		return;
	}

	char *event = malloc(sizeof(char) * MAX_EVENT_LENGTH);
	if (event == NULL) {
		printf("Could not allocate memory for event in %s\n", __func__);
		return;
	}

	char *hostname = malloc(sizeof(char) * (MAX_HOSTNAME_LENGTH + 1));
	if (hostname == NULL) {
		printf("Could not allocate memory for hostname in %s\n"
			, __func__);
		return;
	}

	/* set hostname */
	if (gethostname(hostname, MAX_HOSTNAME_LENGTH + 1)) {
		printf("Could not get hostname: %s\n", strerror(errno));
		return;
	}

	char *local_time = malloc(sizeof(char) * MAX_TIME_LENGTH);
	if (local_time == NULL) {
		printf("Could not allocate memory for time in %s\n", __func__);
		return;
	}

	/* set event type */
	switch (pqe.type) {
		case (int)PQ_EVENT_TYPE_DIP:
			snprintf(event, MAX_EVENT_LENGTH, "DIP");
			volt_event = 1;
			break;
		case (int)PQ_EVENT_TYPE_SWELL:
			snprintf(event, MAX_EVENT_LENGTH, "SWELL");
			volt_event = 1;
			break;
		case (int)PQ_EVENT_TYPE_INTERRUPT:
			snprintf(event, MAX_EVENT_LENGTH, "INTERRUPT");
			volt_event = 1;
			break;
		case (int)PQ_EVENT_TYPE_HARMONIC:
			snprintf(event, MAX_EVENT_LENGTH, "HARMONIC");
			harm_event = 1;
			break;
		default:
			break;
	}

	/* set local time */
	time(&timer);
	if (timer == -1) {
		printf("Could not get time since epoch in %s\n", __func__);
		if (errno)
			printf("error is %s\n", strerror(errno));
		return;
	}

	tmi = localtime(&timer);
	if (tmi == NULL) {
		printf("Error occurred in %s: %s\n", __func__, strerror(errno));
		return;
	}

	strftime(local_time, MAX_TIME_LENGTH, "%Y-%m-%d %H:%M:%S", tmi);

	/* prepare msg to send */
	snprintf(msg, MAX_MSG_LENGTH, "%s started: %s", event, local_time);

	/* get time stamp */
	gettimeofday(&tv, NULL);

	/* write event to logfile */
	file = fopen(conf->powquty_event_path, "a+");
	if (file == NULL) {
		printf("Could not open event log: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(file, "%s,%s,%s,%s,%s,%lu,%lu,%llu,%d",
			hostname,
			conf->dev_uuid,
			event,
			conf->dev_lat,
			conf->dev_lon,
			tv.tv_sec,
			(long int)tv.tv_usec/100,
			pqe.startTime,
			pqe.length);
	if (volt_event)
		fprintf(file, ",%9.6f,-\n", pqe.minMax);
	else if (harm_event)
		fprintf(file, ",%d,%d\n", pqe.harmonic_number,
					  pqe.fail_percentage);
	else
		fprintf(file, "\n");

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

#ifdef MQTT
	publish_event(event);
#endif

	/* free allocated stuff and close file*/
	free(msg);
	free(event);
	free(local_time);
	free(hostname);
	fclose(file);
}

void handle_event(PQResult pqResult, struct powquty_conf *conf) {
	int i;

	for (i = 0; i < pqResult.nmbPqEvents; i++)
		send_event(pqResult.pqEvents[i], conf);
}
