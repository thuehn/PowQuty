/*
 * config.c
 *
 *  Created on: Aug 15, 2016
 *      Author: neez
 */

#include "config.h"

static volatile int config_loaded = 0;
static config_t cfg;

int load_config(char* path) {
	int res = 0;

	// config_setting_t *setting;
	// const char *str;

	config_init(&cfg);

	if(! config_read_file(&cfg, path))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
				config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return -1;
	}

	printf("setting config loaded \n");

	config_loaded = 1;
	return res;
}

void destroy_config () {
	config_destroy(&cfg);
	config_loaded = 0;
}

struct config_t* get_cfg_ptr() {
	return &cfg;
}

int is_config_loaded() {
	return config_loaded;
}
