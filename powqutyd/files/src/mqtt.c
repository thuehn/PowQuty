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
#include "retrieval.h"

void publish_callback(struct mosquitto *mosq, void* obj, int res);
void mqtt_publish_payload();
static void *mosquitto_thread_main(void* param);
static pthread_t mosquitto_thread;

static char payload[MAX_MQTT_MSG_LEN];

struct mosquitto *mosq;

void stop_mosquitto(){
	mosquitto_thread_stop = 1;
	pthread_join(mosquitto_thread, NULL);
}

void connect_callback(struct mosquitto *mosq, void* obj, int res)
{
	// printf("connect callback, rc=%d\n", res);
}

void publish_callback(struct mosquitto *mosq, void* obj, int res) {
	printf("publish callback, rp=%d\n", res);
	publish_msg = 0;
	// TODO unlock Mutex
}

/*
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
*/

int mqtt_init () {
	int res = 0;
	res = pthread_create(&mosquitto_thread,NULL, mosquitto_thread_main,NULL);
	return res;
}

void publish_device_offline() {
	payload[0] = '\0';
	long long ts = get_curr_time_in_milliseconds();
	sprintf(payload,"%s,%lld,0",dev_uuid,ts);
	mqtt_publish_payload();
}

void publish_device_online() {
	payload[0] = '\0';
	long long ts = get_curr_time_in_milliseconds();
	sprintf(payload,
			"%s,%lld,1,%s,%s,%s",
			dev_uuid,
			ts,
			dev_FW_ver,
			dev_APP_ver,
			dev_HW_ver);
	mqtt_publish_payload();
}

void publish_measurements(PQResult pqResult) {
	printf("publish_measurements: \n");
	payload[0] = '\0';
	long long ts = get_curr_time_in_milliseconds();
	sprintf(payload,
			"%s,%lld,3,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f",
			dev_uuid,
			ts,
			pqResult.PowerVoltageEff_5060T,
			pqResult.PowerFrequency5060T,
			pqResult.Harmonics[0],
			pqResult.Harmonics[1],
			pqResult.Harmonics[2],
			pqResult.Harmonics[3],
			pqResult.Harmonics[4],
			pqResult.Harmonics[5],
			pqResult.Harmonics[6] );
	mqtt_publish_payload();
}
void mqtt_publish_payload() {
	printf("%s\n",payload);
	// TODO lock Mutex
	publish_msg = 1;
}

void mqtt_publish_msg(const char* msg) {
	// TODO lock Mutex
	payload[0] = '\0';
	sprintf(payload,"%s",msg);
	publish_msg = 1;
}

int mqtt_publish(struct mosquitto *mosq, const char* msg ) {
	int res;
	unsigned int len = (unsigned int)strlen(msg);
	res =  mosquitto_publish(mosq, NULL, mqtt_topic,len, (const unsigned char *)msg, 0, false);
	return res;
}

static void *mosquitto_thread_main(void* param) {
	char buff[250];
	char* clientid = "MQTT_Client";

	int mosq_loop= 0, rc = 0, pub_res = 0;
	// char payload_msg[250] ="";

	mosquitto_lib_init();
	mosq = mosquitto_new(clientid, true, 0);

	if(mosq){
		rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
		if(rc != MOSQ_ERR_SUCCESS) {
			printf("Error: mosquitto_connect\n");
		}
		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_publish_callback_set(mosq, publish_callback);

		while (!mosquitto_thread_stop) {
			mosq_loop = mosquitto_loop(mosq, 0, 1);
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
