/*
 * config.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <libconfig.h>

static config_t cfg;

static volatile int config_loaded = 0;

int load_config(char * path);

void destroy_config ();

/*
#define mqtt_host		"localhost"
#define mqtt_topic		"devices/update"
#define dev_uuid		"BERTUB001"
#define dev_gps			"BERTUB001"
#define dev_FW_ver		"0.1"
#define dev_APP_ver		"0.1"
#define dev_HW_ver		"029"

#define device_tty		"/dev/ttyACM3"
 */

#endif /* CONFIG_H_ */
