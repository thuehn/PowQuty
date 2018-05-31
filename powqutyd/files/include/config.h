/*
 * config.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <libconfig.h>

struct config_t* get_cfg_ptr();

int load_config(char * path);

void destroy_config ();

int is_config_loaded();

#endif /* CONFIG_H_ */
