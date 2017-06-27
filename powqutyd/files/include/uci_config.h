/*
 * uci_config.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef UCI_CONFIG_H_
#define UCI_CONFIG_H_

#define MAX_LENGTH	32

struct powquty_conf {

	char device_tty[MAX_LENGTH];
	char mqtt_host[MAX_LENGTH];
	char mqtt_topic[MAX_LENGTH];
	char mqtt_uname[MAX_LENGTH];
	char mqtt_pw[MAX_LENGTH];
	char dev_uuid[MAX_LENGTH];
	char dev_lat[MAX_LENGTH];
	char dev_lon[MAX_LENGTH];
	char powquty_path[MAX_LENGTH];
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
