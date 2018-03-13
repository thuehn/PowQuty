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

/*
 * check option value from uci config
 * @param option_value: name of the option to check
 * @param option_length: max length of option value
 * @return 1 on to long option value, -1 if unset, 0 else
 */
static int check_option_value(const char * option_value, unsigned int option_length) {
	if ((option_value == NULL) || (strlen(option_value) == 0)) {
		return -1;
	}
	if (strlen(option_value) >= option_length - 1) {
		printf("WARN: %s max length is %u\n", option_value,
						      option_length);
		return 1;
	}
	return 0;
}


int uci_config_powquty(struct powquty_conf* conf) {
	struct uci_context* uci;
	struct uci_package* p;
	struct uci_element* e;
	const char* str;

	/* general configuration */
	char default_powquty_path[PATH_LENGTH] = "/tmp/powquty.log";
	char default_event_path[PATH_LENGTH] = "/tmp/powquty_event.log";
	char default_device_tty[MAX_LENGTH] = "/dev/ttyACM0";
	char default_dev_uuid[MAX_LENGTH] = "BERTUB001";
	char default_dev_lat[MAX_LENGTH] = "55.0083525";
	char default_dev_lon[MAX_LENGTH] = "82.935732";
	char default_dev_acc[MAX_LENGTH] = "18.234567";
	char default_dev_alt[MAX_LENGTH] = "0";
	long default_max_log_size_kb = 4096;
	int default_powqutyd_print = ON;

	/* metadata block */
	char default_meta_comment[MAX_LONG_LENGTH] = "";
	char default_meta_id[MAX_LENGTH] = "";
	char default_meta_operator[MAX_LONG_LENGTH] = "";
	char default_meta_phase[MAX_LENGTH] = "";
	char default_meta_reason[MAX_LONG_LENGTH] = "";
	char default_meta_type[MAX_LONG_LENGTH] = "";
	int default_meta_use = OFF;

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
			strcpy(conf->dev_acc, default_dev_acc);
			strcpy(conf->dev_alt, default_dev_alt);
			strcpy(conf->powquty_path, default_powquty_path);
			strcpy(conf->powquty_event_path, default_event_path);

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

			/* metadata */
			conf->use_metadata = default_meta_use;
			strcpy(conf->meta_comment, default_meta_comment);
			strcpy(conf->meta_id, default_meta_id);
			strcpy(conf->meta_operator, default_meta_operator);
			strcpy(conf->meta_phase, default_meta_phase);
			strcpy(conf->meta_reason, default_meta_reason);
			strcpy(conf->meta_type, default_meta_type);

			/* general */
			/* uuid */
			str = uci_lookup_option_string(uci, s, "dev_uuid");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->dev_uuid, str);
				printf("looking up dev_uuid: currently ==> %s\n",
					conf->dev_uuid);
			}

			/* latitude */
			str = uci_lookup_option_string(uci, s, "dev_lat");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->dev_lat, str);
				printf("looking up dev_lat: currently ==> %s\n",
					conf->dev_lat);
			}

			/* longitude */
			str = uci_lookup_option_string(uci, s, "dev_lon");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->dev_lon, str);
				printf("looking up dev_lon: currently ==> %s\n",
					conf->dev_lon);
			}

			/* gps accuracy */
			str = uci_lookup_option_string(uci, s, "dev_acc");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->dev_acc, str);
				printf("looking up dev_acc: currently ==> %s\n",
					conf->dev_acc);
			}

			/* altitude */
			str = uci_lookup_option_string(uci, s, "dev_alt");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->dev_alt, str);
				printf("looking up dev_alt: currently ==> %s\n",
					conf->dev_alt);
			}

			/* device tty */
			str = uci_lookup_option_string(uci, s, "device_tty");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->device_tty, str);
				printf("looking up device_tty: currently ==> %s\n",
					conf->device_tty);
			}

			/* logfile path */
			str = uci_lookup_option_string(uci, s, "powquty_path");
			if (!check_option_value(str, PATH_LENGTH)) {
				strcpy(conf->powquty_path, str);
				printf("looking up powquty_path: currently ==> %s\n",
					conf->powquty_path);
			}

			/* event log file */
			str = uci_lookup_option_string(uci, s,
						       "powquty_event_path");
			if (!check_option_value(str, PATH_LENGTH)) {
				strcpy(conf->powquty_event_path, str);
				printf("looking up powquty_event_path: currently ==>"
					"%s\n", conf->powquty_event_path);
			}

			/* print_print */
			conf->powqutyd_print = uci_lookup_option_int(uci, s,
					"powqutyd_print");

			/* max logfile size */
			conf->max_log_size_kb = uci_lookup_option_long(uci, s,
					"max_log_size_kb");

			/* metadata */
			/* use metadata */
			conf->use_metadata= uci_lookup_option_int(uci, s,
					"use_metadata");

			/* comment */
			str = uci_lookup_option_string(uci, s, "meta_comment");
			if (!check_option_value(str, MAX_LONG_LENGTH)) {
				strcpy(conf->meta_comment, str);
				printf("looking up meta_comment: currently ==> %s\n",
					conf->meta_comment);
			}

			/* id */
			str = uci_lookup_option_string(uci, s, "meta_id");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->meta_id, str);
				printf("looking up meta_id: currently ==> %s\n",
					conf->meta_id);
			}

			/* operator name */
			str = uci_lookup_option_string(uci, s, "meta_operator");
			if (!check_option_value(str, MAX_LONG_LENGTH)) {
				strcpy(conf->meta_operator, str);
				printf("looking up meta_operator: currently ==> %s\n",
					conf->meta_operator);
			}

			/* pahse */
			str = uci_lookup_option_string(uci, s, "meta_phase");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->meta_phase, str);
				printf("looking up meta_phase: currently ==> %s\n",
					conf->meta_phase);
			}

			/* reason */
			str = uci_lookup_option_string(uci, s, "meta_reason");
			if (!check_option_value(str, MAX_LONG_LENGTH)) {
				printf("in if\n");
				strcpy(conf->meta_reason, str);
				printf("looking up meta_reason: currently ==> %s\n",
					conf->meta_reason);
			}

			/* type */
			str = uci_lookup_option_string(uci, s, "meta_type");
			if (!check_option_value(str, MAX_LONG_LENGTH)) {
				strcpy(conf->meta_type, str);
				printf("looking up meta_type: currently ==> %s\n",
					conf->meta_type);
			}

			/* mqtt */
			/* mqtt_host */
			str = uci_lookup_option_string(uci, s, "mqtt_host");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->mqtt_host, str);
				printf("looking up mqtt_host: currently ==> %s\n",
					conf->mqtt_host);
			}

			/* mqtt_topic */
			str = uci_lookup_option_string(uci, s, "mqtt_topic");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->mqtt_topic, str);
				printf("looking up mqtt_topic: currently ==> %s\n",
					conf->mqtt_topic);
			}

			/* mqtt username */
			str = uci_lookup_option_string(uci, s, "mqtt_uname");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->mqtt_uname, str);
				printf("looking up mqtt_uname: currently ==> %s\n",
					conf->mqtt_uname);
			}

			/* mqtt password */
			str = uci_lookup_option_string(uci, s, "mqtt_pw");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->mqtt_pw, str);
				printf("looking up mqtt_pw: currently ==> %s\n",
					conf->mqtt_pw);
			}

			/* slack */
			/* powquty slack */
			conf->slack_notification = uci_lookup_option_int(uci, s,
				"slack_notification");

			/* webhook */
			str = uci_lookup_option_string(uci, s, "slack_webhook");
			if (!check_option_value(str, MAX_WEBHOOK_LENGTH)) {
				strcpy(conf->slack_webhook, str);
				printf("looking up slack_webhook: currently ==> %s\n",
					conf->slack_webhook);
			}

			/* slack channel */
			str = uci_lookup_option_string(uci, s, "slack_channel");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->slack_channel, str);
				printf("looking up slack_channel: currently ==> %s\n",
					conf->slack_channel);
			}

			/* slack user */
			str = uci_lookup_option_string(uci, s, "slack_user");
			if (!check_option_value(str, MAX_LENGTH)) {
				strcpy(conf->slack_user,str);
				printf("looking up slack_user: currently ==> %s\n",
					conf->slack_user);
			}
		}
	}
	uci_unload(uci, p);
	uci_free_context(uci);
	return 0;
}
