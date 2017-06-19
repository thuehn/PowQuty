/*
 * uci_config.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef UCI_CONFIG_H_
#define UCI_CONFIG_H_

#define MAX_STR_LEN	31

struct powquty_conf {

	char device_tty[32];
	char mqtt_host[32];
	char mqtt_topic[32];
	char mqtt_uname[32];
	char mqtt_pw[32];
	char dev_uuid[32];
	char dev_lat[32];
	char dev_lon[32];
	char powquty_path[32];
	int powqutyd_print;
	long max_log_size_kb;

/*	option device_tty '/dev/ttyACM0'
		option mqtt_host 'localhost'
		option mqtt_topic 'devices/update'
		option dev_uuid 'BERTUB001'
		option dev_lat '51.156033'
		option dev_lon '10.715828'
		option powqutyd_print '1'
		*/
};

int uci_config_powquty(struct powquty_conf*);

#endif /* CONFIG_H_ */
