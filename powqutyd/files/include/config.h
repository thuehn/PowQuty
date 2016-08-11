/*
 * config.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef CONFIG_H_
#define CONFIG_H_


#define mqtt_port 1883

#define mqtt_host		"localhost"
#define mqtt_topic		"devices/update"
#define dev_uuid		"BERTUB001"
#define dev_gps			"BERTUB001"
#define dev_FW_ver		"0.1"
#define dev_APP_ver		"0.1"
#define dev_HW_ver		"029"

#define device_tty		"/dev/ttyACM3"

/*
static const char* mqtt_host = "localhost";
static const char* mqtt_topic = "devices/update";
static const char* dev_uuid = "BERTUB001";
static const char* dev_gps = "BERTUB001";
static const char* dev_FW_ver = "0.1";
static const char* dev_APP_ver = "0.1";
static const char* dev_HW_ver = "029";

static const char* device_tty = "/dev/ttyACM3";
*/
#endif /* CONFIG_H_ */
