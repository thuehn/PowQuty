/*
 * mqtt.h
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */

#ifndef MQTT_H_
#define MQTT_H_


static volatile int mosquitto_loop_stop = 0, publish_msg=0;

void stop_mosquitto();
int mqtt_init();
void mqtt_publish_msg(const char* msg);

#endif /* MQTT_H_ */
