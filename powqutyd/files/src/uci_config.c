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

	char default_device_tty[32] = "/dev/ttyACM0";
	char default_powquty_path[32] = "/tmp/powquty.log";
	char default_mqtt_host[32] = "localhost";
	char default_mqtt_topic[32] = "devices/update";
	char default_dev_uuid[32] = "BERTUB001";
	int default_powqutyd_print = 1;
	long default_max_log_size_kb = 4096;

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
			strcpy(conf->device_tty, default_device_tty);
			strcpy(conf->dev_uuid, default_dev_uuid);
			strcpy(conf->mqtt_host, default_mqtt_host);
			strcpy(conf->mqtt_topic, default_mqtt_topic);
			strcpy(conf->powquty_path, default_powquty_path);

			conf->powqutyd_print = default_powqutyd_print;
			conf->max_log_size_kb = default_max_log_size_kb;

			str = uci_lookup_option_string(uci, s, "dev_uuid");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_STR_LEN) {
				continue;
			}
			strcpy(conf->dev_uuid, str);
			printf("looking up dev_uuid: currently ==> %s\n", conf->dev_uuid);


			str = uci_lookup_option_string(uci, s, "mqtt_host");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_STR_LEN) {
				continue;
			}
			strcpy(conf->mqtt_host, str);
			printf("looking up mqtt_host: currently ==> %s\n", conf->mqtt_host);


			str = uci_lookup_option_string(uci, s, "mqtt_topic");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_STR_LEN) {
				continue;
			}
			strcpy(conf->mqtt_topic, str);
			printf("looking up mqtt_topic: currently ==> %s\n", conf->mqtt_topic);

			str = uci_lookup_option_string(uci, s, "device_tty");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_STR_LEN) {
				continue;
			}
			strcpy(conf->device_tty, str);
			printf("looking up device_tty currently ==> %s\n", conf->device_tty);

			str = uci_lookup_option_string(uci, s, "powquty_path");
			if (str == NULL)
				continue;
			if (strlen(str) >= MAX_STR_LEN) {
				continue;
			}
			strcpy(conf->powquty_path, str);
			printf("looking up powquty_path currently ==> %s\n", conf->powquty_path);

			conf->powqutyd_print = uci_lookup_option_int(uci, s,
					"powqutyd_print");

			conf->max_log_size_kb = uci_lookup_option_long(uci, s,
					"max_log_size_kb");

		}
	}

	uci_unload(uci, p);
	uci_free_context(uci);
	return 0;
}
