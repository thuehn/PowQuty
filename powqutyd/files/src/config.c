/*
 * config.c
 *
 *  Created on: Aug 15, 2016
 *      Author: neez
 */

#include "config.h"

int load_config(char* path) {
	int res = 0;

	// config_setting_t *setting;
	// const char *str;

	config_init(&cfg);

	if(! config_read_file(&cfg, "example.cfg"))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return -1;
	}

	config_loaded = 1;
	return res;
}

void destroy_config () {
	config_destroy(&cfg);
	config_loaded = 0;
}
