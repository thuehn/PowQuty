/*
 * uci_config.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef UCI_CONFIG_H_
#define UCI_CONFIG_H_

#define OFF 0
#define ON (!OFF)

#define MAX_LENGTH		32
#define MAX_LONG_LENGTH		64
#define PATH_LENGTH		512
#define MAX_WEBHOOK_LENGTH	512

struct powquty_conf {
	/* general */
	char device_tty[MAX_LENGTH];
	char dev_uuid[MAX_LENGTH];
	char dev_lat[MAX_LENGTH];
	char dev_lon[MAX_LENGTH];
	char dev_acc[MAX_LENGTH];
	char dev_alt[MAX_LENGTH];
	char powquty_path[PATH_LENGTH];
	char powquty_event_path[PATH_LENGTH];
	long max_log_size_kb;
	int  powqutyd_print;

	/* metadata */
	char meta_comment[MAX_LONG_LENGTH];
	char meta_id[MAX_LENGTH];
	char meta_operator[MAX_LONG_LENGTH];
	char meta_phase[MAX_LENGTH];
	char meta_reason[MAX_LONG_LENGTH];
	char meta_type[MAX_LONG_LENGTH];
	int use_metadata;

	/* mqtt */
	char mqtt_host[MAX_LENGTH];
	char mqtt_topic[MAX_LENGTH];
	char mqtt_uname[MAX_LENGTH];
	char mqtt_pw[MAX_LENGTH];
	int send_t5060_data;
	int send_t1012_data;

	/* slack */
	char slack_webhook[MAX_WEBHOOK_LENGTH];
	char slack_channel[MAX_LENGTH];
	char slack_user[MAX_LENGTH];
	int  slack_notification;

/*	option device_tty '/dev/ttyACM0'
		option mqtt_host 'localhost'
		option mqtt_topic 'devices/update'
		option mqtt_uname 'username'
		option mqtt_pw 'password'

		option dev_uuid 'BERTUB001'
		option dev_lat '51.156033'
		option dev_lon '10.715828'
		option dev_acc '18.986999'
		option dev_alt '175.30000'
		option powqutyd_print '1'
		option slack_webhook ''
		option slack_channel '#general'
		option slack_user 'PowQutyEvent'
		option slack_notification '0'

		option use_metadata '0'
		option meta_comment 'some comment'
		option meta_id 'id'
		option meta_operator 'operator name'
		option meta_phase '<phase number>'
		option meta_reason 'some reason'
		option meta_type 'measurement type'
		*/
};

int uci_config_powquty(struct powquty_conf*);

#endif /* CONFIG_H_ */
