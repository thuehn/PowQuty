/*
 * mqtt.c
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */
#include "config.h"
#include "mqtt.h"
#include <stdio.h>
#include <string.h>
#include <mosquitto.h>
#include <pthread.h>

static void *mosquitto_thread_main(void* param);
static pthread_t mosquitto_thread;

static const char* payload= "Test_Message";

void stop_mosquitto(){
	mosquitto_loop_stop = 1;
	pthread_join(mosquitto_thread, NULL);
}

void connect_callback(void* obj, int res)
{
	printf("connect callback, rc=%d\n", res);
}

void publish_callback(void* obj, uint16_t res) {
	printf("publish callback, rp=%d\n", res);
	publish_msg = 0;
	// TODO unlock Mutex
}

void mqtt_message_print(struct mosquitto_message* msg) {
	int i;
	printf("MQTT-Msg: \n\tM-ID: %d\tTopic: %s\tQoS: %d",
			msg->mid, msg->topic, msg->qos);
	if(msg->retain){
		printf("\tretain\n");
	} else {
		printf("\tNo-retain\n");
	}
	if(msg->payloadlen > 0) {
		printf("\t");
		for(i = 0;i<msg->payloadlen; i++){
			printf("%c",msg->payload[i]);
		}
		printf("\n");
	} else {
		printf("\t--- No Payload ---\n");
	}
}

int mqtt_init () {
	int res = 0;
	res = pthread_create(&mosquitto_thread,NULL, mosquitto_thread_main,NULL);
	return res;
}

void mqtt_publish_msg(const char* msg) {
	// TODO lock Mutex
	payload = msg;
	publish_msg = 1;
}

int mqtt_publish(struct mosquitto *mosq, const char* msg ) {
	uint32_t len = (uint32_t)strlen(msg);
	return mosquitto_publish(mosq, NULL, topic,len, (const uint8_t *)msg, 0, false);
}

static void *mosquitto_thread_main(void* param) {
	char buff[250];
	char* clientid = "MQTT_Client";
	struct mosquitto *mosq;
	int mosq_loop= 0, rc = 0, pub_res = 0;
	// char payload_msg[250] ="";

	mosquitto_lib_init();
	mosq = mosquitto_new(clientid, 0);

	if(mosq){
		rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60, true);
		if(rc != MOSQ_ERR_SUCCESS) {
			printf("Error: mosquitto_connect\n");
		}
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_publish_callback_set(mosq, publish_callback);

		while (!mosquitto_loop_stop) {
			mosq_loop = mosquitto_loop(mosq, 0);
			if (mosq_loop) {
				printf("Loop Failed: %d\t", mosq_loop);
				strerror_r(mosq_loop,buff,250);
				printf("%s\n",buff);
				// TODO
				printf("Do something! like reconnect?");
			} else {
				if(publish_msg) {
					pub_res = mqtt_publish(mosq,payload);
					if(pub_res != MOSQ_ERR_SUCCESS) {
						printf("Error: mosquitto_publish\n");
					}
				}
			}
		}

		mosquitto_disconnect(mosq);
		mosquitto_destroy(mosq);
	}

	mosquitto_lib_cleanup();
	return NULL;
}
