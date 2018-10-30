/*
 * config.c
 *
 *  Created on: Aug 15, 2016
 *      Author: neez
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <uci.h>

#include "uci_config.h"

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
static int config_option_is_valid(const char *option_value,
				  unsigned int option_length) {
	int ret = 0;

	if ((option_value == NULL) || (strlen(option_value) == 0)) {
		ret = -1;
	}
	if (strlen(option_value) >= option_length - 1) {
		printf("WARN: %s max length is %u\n", option_value,
						      option_length);
		ret = 1;
	}
	return ret;
}

/*
 * set default parameters for powquty
 *
 * @param conf: powquty_conf struct to store defaults in
 */
static void init_default_values(struct powquty_conf *conf) {
	/* general configuration */
	char default_powquty_path[PATH_LENGTH] = "/tmp/powquty.log";
	char default_event_path[PATH_LENGTH] = "/tmp/powquty_event.log"
	char default_device_tty[MAX_LENGTH] = "/dev/WeSense0";
	char default_dev_uuid[MAX_LENGTH] = "BERTUB001";
	char default_dev_lat[MAX_LENGTH] = "55.0083525";
	char default_dev_lon[MAX_LENGTH] = "82.935732";
	char default_dev_acc[MAX_LENGTH] = "18.234567";
	char default_dev_alt[MAX_LENGTH] = "0";
	long default_max_log_size_kb = 4096;

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

	/* general */
	strcpy(conf->device_tty, default_device_tty);
	strcpy(conf->dev_uuid, default_dev_uuid);
	strcpy(conf->dev_lat, default_dev_lat);
	strcpy(conf->dev_lon, default_dev_lon);
	strcpy(conf->dev_acc, default_dev_acc);
	strcpy(conf->dev_alt, default_dev_alt);
	strcpy(conf->powquty_path, default_powquty_path);
	strcpy(conf->powquty_event_path, default_event_path);

	conf->powqutyd_print = ON;
	conf->max_log_size_kb = default_max_log_size_kb;
	conf->send_t5060_data = ON;
	conf->send_t1012_data = OFF;

	/* mqtt */
	strcpy(conf->mqtt_host, default_mqtt_host);
	strcpy(conf->mqtt_topic, default_mqtt_topic);

	/* slack */
	conf->slack_notification = OFF;
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
}

/*
 * set powquty option
 *
 * @param dest: storage location in powquty_conf_struct
 * @param str: string to set
 * @param length: maximal allowed length of string
 *
 * @return: 1 if string is to long, 0 else
 */
unsigned int set_powquty_option(char *dest, const char *str,
		       unsigned int length) {
	unsigned int ret = 0;
	int valid = config_option_is_valid(str, length);
	if ( valid  == 0) {
		strcpy(dest, str);
	} else if (valid > 0) {
		ret = 1;
	}
	return ret;
}

/*
 * print current configuration
 *
 * @param conf: powquty configuratoin struct
 */
static void print_powquty_conf(struct powquty_conf *conf) {
	/* general */
	printf("dev_uuid:\t\t%s\n", conf->dev_uuid);
	printf("dev_lat:\t\t%s\n", conf->dev_lat);
	printf("dev_lon:\t\t%s\n", conf->dev_lon);
	printf("dev_acc:\t\t%s\n", conf->dev_acc);
	printf("dev_alt:\t\t%s\n", conf->dev_alt);
	printf("device_tty:\t\t%s\n", conf->device_tty);
	printf("powquty_path:\t\t%s\n", conf->powquty_path);
	printf("powquty_event_path:\t\t%s\n", conf->powquty_event_path)
	printf("powqutyd_print:\t\t%d\n", conf->powqutyd_print);
	printf("max_log_size_kb:\t\t%ld\n", conf->max_log_size_kb);

	/* metadate */
	printf("use_metadata:\t\t%d\n", conf->use_metadata);
	printf("meta_comment:\t\t%s\n", conf->meta_comment);
	printf("meta_id:\t\t%s\n", conf->meta_id);
	printf("meta_operator:\t\t%s\n", conf->meta_operator);
	printf("meta_phase:\t\t%s\n", conf->meta_phase);
	printf("meta_reason:\t\t%s\n", conf->meta_reason);
	printf("meta_type:\t\t%s\n", conf->meta_type);

	/* mqtt */
	printf("mqtt_host:\t\t%s\n", conf->mqtt_host);
	printf("mqtt_topic:\t\t%s\n", conf->mqtt_topic);
	printf("mqtt_uname:\t\t%s\n", conf->mqtt_uname);
	printf("mqtt_pw:\t\t%s\n", conf->mqtt_pw);
	printf("send_t5060_data:\t\t%d\n", conf->send_t5060_data);
	printf("send_t1012_data:\t\t%d\n", conf->send_t1012_data);

	/* slack */
	printf("slack_notification:\t\t%d\n", conf->slack_notification);
	printf("slack_webhook:\t\t%s\n", conf->slack_webhook);
	printf("slack_channel:\t\t%s\n", conf->slack_channel);
	printf("slack_user:\t\t%s\n", conf->slack_user);
}

/*
 * read in uci config
 *
 * @param conf: powquty configuration struct
 */
int uci_config_powquty(struct powquty_conf* conf) {
	struct uci_context* uci;
	struct uci_package* p;
	struct uci_element* e;
	unsigned int failed = 0;
	const char* str;

	init_default_values(conf);

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
			str = uci_lookup_option_string(uci, s, "dev_uuid");
			failed += set_powquty_option(conf->dev_uuid, str,
						    MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "dev_lat");
			failed += set_powquty_option(conf->dev_lat, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "dev_lon");
			failed += set_powquty_option(conf->dev_lon, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "dev_acc");
			failed += set_powquty_option(conf->dev_acc, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "dev_alt");
			failed += set_powquty_option(conf->dev_alt, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "device_tty");
			failed += set_powquty_option(conf->device_tty, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "powquty_path");
			failed += set_powquty_option(conf->powquty_path, str,
						     PATH_LENGTH);
			str = uci_lookup_option_string(uci, s,
						       "powquty_event_path");
			failed += set_powquty_option(conf->powquty_event_path,
						     str, PATH_LENGTH);
			conf->powqutyd_print = uci_lookup_option_int(uci, s,
					"powqutyd_print");
			conf->max_log_size_kb = uci_lookup_option_long(uci, s,
					"max_log_size_kb");

			/* metadata */
			conf->use_metadata= uci_lookup_option_int(uci, s,
					"use_metadata");
			str = uci_lookup_option_string(uci, s, "meta_comment");
			failed += set_powquty_option(conf->meta_comment, str,
						     MAX_LONG_LENGTH);
			str = uci_lookup_option_string(uci, s, "meta_id");
			failed += set_powquty_option(conf->meta_id, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "meta_operator");
			failed += set_powquty_option(conf->meta_operator, str,
						     MAX_LONG_LENGTH);
			str = uci_lookup_option_string(uci, s, "meta_phase");
			failed += set_powquty_option(conf->meta_phase, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "meta_reason");
			failed += set_powquty_option(conf->meta_reason, str,
						     MAX_LONG_LENGTH);
			str = uci_lookup_option_string(uci, s, "meta_type");
			failed += set_powquty_option(conf->meta_type, str,
						     MAX_LONG_LENGTH);

			/* mqtt */
			str = uci_lookup_option_string(uci, s, "mqtt_host");
			failed += set_powquty_option(conf->mqtt_host, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "mqtt_topic");
			failed += set_powquty_option(conf->mqtt_topic, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "mqtt_uname");
			failed += set_powquty_option(conf->mqtt_uname, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "mqtt_pw");
			failed += set_powquty_option(conf->mqtt_pw, str,
						     MAX_LENGTH);
			conf->send_t5060_data = uci_lookup_option_int(uci, s,
					"send_t5060_data");
			conf->send_t1012_data = uci_lookup_option_int(uci, s,
					"send_t1012_data");

			/* slack */
			conf->slack_notification = uci_lookup_option_int(uci, s,
				"slack_notification");
			str = uci_lookup_option_string(uci, s, "slack_webhook");
			failed += set_powquty_option(conf->slack_webhook, str,
						     MAX_WEBHOOK_LENGTH);
			str = uci_lookup_option_string(uci, s, "slack_channel");
			failed += set_powquty_option(conf->slack_channel, str,
						     MAX_LENGTH);
			str = uci_lookup_option_string(uci, s, "slack_user");
			failed += set_powquty_option(conf->slack_user, str,
						     MAX_LENGTH);
		}
	}
	print_powquty_conf(conf);
	if (failed > 0) {
		printf("WARN: could not set %u options\n", failed);
	}

	uci_unload(uci, p);
	uci_free_context(uci);
	return 0;
}
