/*
 * config.c
 *
 *  Created on: Aug 15, 2016
 *      Author: neez
 */

#include <stdlib.h>
#include <string.h>
#include <uci.h>
#include "uci_config.h"

#define OFF 0
#define ON (!OFF)

/** analogous to uci_lookup_option_string from uci.h, returns -1 when not found */
static int uci_lookup_option_int(struct uci_context *uci, struct uci_section *s,
		const char *name) {
	const char* str = uci_lookup_option_string(uci, s, name);
	return str == NULL ? -1 : atoi(str);
}

static long uci_lookup_option_long(struct uci_context *uci,
				   struct uci_section *s,
				   const char *name) {
	const char *str = uci_lookup_option_string(uci, s, name);
	return str == NULL ? -1 : atol(str);
}

int uci_config_powquty(struct powquty_conf* conf) {
	struct uci_context* uci;
	struct uci_package* p;
	struct uci_element* e;
	const char* str;

	/* general configuration */
	char default_device_tty[MAX_LENGTH] = "/dev/ttyACM0";
	char default_powquty_path[MAX_LENGTH] = "/tmp/powquty.log";
	char default_dev_uuid[MAX_LENGTH] = "BERTUB001";
	char default_dev_lat[MAX_LENGTH] = "55.0083525";
	char default_dev_lon[MAX_LENGTH] = "82.935732";
	int default_powqutyd_print = ON;
	long default_max_log_size_kb = 4096;

	/* mqtt config */
	char default_mqtt_host[MAX_LENGTH] = "localhost";
	char default_mqtt_topic[MAX_LENGTH] = "devices/update";

	/* Slack configuration */
	char default_webhook[MAX_LENGTH] = "";
	char default_slack_channel[MAX_LENGTH] = "#general";
	char default_slack_user[MAX_LENGTH] = "PowQutyEvent";
	int default_slack = OFF;

	uci = uci_alloc_context();
	if (uci == NULL)
		return 0;

	if (uci_load(uci, "powquty", &p)) {
		uci_free_context(uci);
		return 0;
	}

	uci_foreach_element(&p->sections, e)
	{
		struct uci_section *s = uci_to_section(e);
		if (strcmp(s->type, "powquty") == 0) {
			/* general */
			strcpy(conf->device_tty, default_device_tty);
			strcpy(conf->dev_uuid, default_dev_uuid);
			strcpy(conf->dev_lat, default_dev_lat);
			strcpy(conf->dev_lon, default_dev_lon);
			strcpy(conf->powquty_path, default_powquty_path);

			conf->powqutyd_print = default_powqutyd_print;
			conf->max_log_size_kb = default_max_log_size_kb;

			/* mqtt */
			strcpy(conf->mqtt_host, default_mqtt_host);
			strcpy(conf->mqtt_topic, default_mqtt_topic);

			/* slack */
			conf->slack_notification = default_slack;
			strcpy(conf->slack_webhook, default_webhook);
			strcpy(conf->slack_channel, default_slack_channel);
			strcpy(conf->slack_user, default_slack_user);

			/* general */
			/* uuid */
			str = uci_lookup_option_string(uci, s, "dev_uuid");
			if (str == NULL)
				continue;
			if (strlen(str) > MAX_LENGTH) {
				continue;
			}
			strcpy(conf->dev_uuid, str);
			printf("looking up dev_uuid: currently ==> %s\n",
				conf->dev_uuid);

			/* latitude */
			str = uci_lookup_option_string(uci, s, "dev_lat");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1) {
				continue;
			}
			strcpy(conf->dev_lat, str);
			printf("looking up dev_lat: currently ==> %s\n",
				conf->dev_lat);

			/* longitude */
			str = uci_lookup_option_string(uci, s, "dev_lon");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1) {
				continue;
			}
			strcpy(conf->dev_lon, str);
			printf("looking up dev_lon: currently ==> %s\n",
				conf->dev_lon);

			/* device tty */
			str = uci_lookup_option_string(uci, s, "device_tty");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1) {
				continue;
			}
			strcpy(conf->device_tty, str);
			printf("looking up device_tty currently ==> %s\n",
				conf->device_tty);

			/* logfile path */
			str = uci_lookup_option_string(uci, s, "powquty_path");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1) {
				continue;
			}
			strcpy(conf->powquty_path, str);
			printf("looking up powquty_path currently ==> %s\n",
				conf->powquty_path);

			/* print_print */
			conf->powqutyd_print = uci_lookup_option_int(uci, s,
					"powqutyd_print");

			/* max logfile size */
			conf->max_log_size_kb = uci_lookup_option_long(uci, s,
					"max_log_size_kb");

			/* mqtt */
			/* mqtt_host */
			str = uci_lookup_option_string(uci, s, "mqtt_host");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1) {
				continue;
			}
			strcpy(conf->mqtt_host, str);
			printf("looking up mqtt_host: currently ==> %s\n",
				conf->mqtt_host);

			/* mqtt_topic */
			str = uci_lookup_option_string(uci, s, "mqtt_topic");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1) {
				continue;
			}
			strcpy(conf->mqtt_topic, str);
			printf("looking up mqtt_topic: currently ==> %s\n",
				conf->mqtt_topic);

			/* mqtt username */
			str = uci_lookup_option_string(uci, s, "mqtt_uname");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1) {
				continue;
			}
			strcpy(conf->mqtt_uname, str);
			printf("looking up mqtt_uname: currently ==> %s\n",
				conf->mqtt_uname);

			/* mqtt password */
			str = uci_lookup_option_string(uci, s, "mqtt_pw");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1) {
				continue;
			}
			strcpy(conf->mqtt_pw, str);
			printf("looking up mqtt_pw: currently ==> %s\n",
				conf->mqtt_pw);

			/* slack */
			/* powquty slack */
			conf->slack_notification = uci_lookup_option_int(uci, s,
				"slack_notification");

			/* webhook */
			str = uci_lookup_option_string(uci, s, "slack_webhook");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1)
				continue;
			strcpy(conf->slack_webhook, str);
			printf("looking up slack_webhook: currently ==> %s\n",
				conf->slack_webhook);

			/* slack channel */
			str = uci_lookup_option_string(uci, s, "slack_channel");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1)
				continue;
			strcpy(conf->slack_channel, str);
			printf("looking up slack_channel: currently ==> %s\n",
				conf->slack_channel);

			/* slack user */
			str = uci_lookup_option_string(uci, s, "slack_user");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_LENGTH - 1)
				continue;
			strcpy(conf->slack_user,str);
			printf("looking up slack_user: currently ==> %s\n",
				conf->slack_user);
		}
	}
	uci_unload(uci, p);
	uci_free_context(uci);
	return 0;
}
