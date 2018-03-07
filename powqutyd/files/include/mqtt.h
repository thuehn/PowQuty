/*
 * mqtt.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef MQTT_H_
#define MQTT_H_

#include "PQ_App.h"

#define MAX_MQTT_MSG_LEN	1024
#define mqtt_port		1883

static volatile int mosquitto_thread_stop = 0, publish_msg = 0;

void stop_mosquitto();
int mqtt_init();
void mqtt_publish_msg(const char* msg);


void publish_device_offline();
void publish_device_online();
void publish_device_gps();
void publish_measurements(PQResult pqResult) ;

#endif /* MQTT_H_ */
