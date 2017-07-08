/*
 * mqtt.c
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */
#ifdef MQTT
#include "config.h"
#include "mqtt.h"
#include <stdio.h>
#include <string.h>
#include <mosquitto.h>
#include <pthread.h>
#include "helper.h"
#include "uci_config.h"
#include <sys/time.h>
#include <stdlib.h>

static const char* mqtt_host = "localhost";
static const char* mqtt_topic = "devices/update";
static const char* mqtt_uname = "username";
static const char* mqtt_pw = "password";
static const char* dev_uuid = "BERTUB001";
static const char* dev_lat = "55.0083525";
static const char* dev_lon = "82.935732";
//static const char* dev_gps = "BERTUB001";
static const char* dev_FW_ver = "0.1";
static const char* dev_APP_ver = "0.1";
static const char* dev_HW_ver = "029";
static const char* mqtt_event_host = "localhost";
static const char* mqtt_event_topic = "PowQutyEvent";
static int powqutyd_print = 0;
static int mqtt_event_flag = 0;

struct powquty_conf* config;
void publish_callback(struct mosquitto *mosq, void* obj, int res);
void mqtt_publish_payload();
static void *mosquitto_thread_main(void* param);
static pthread_t mosquitto_thread;

static char payload[MAX_MQTT_MSG_LEN];
static char event_payload[MAX_MQTT_MSG_LEN];

struct mosquitto *mosq;
struct mosquitto *event_mosq;

void stop_mosquitto(){
	publish_device_offline();
	mosquitto_thread_stop = 1;
	printf("DEBUG:\tJoining MQTT Thread\n");
	pthread_join(mosquitto_thread, NULL);
}

void connect_callback(struct mosquitto *mosq, void* obj, int res)
{
	// printf("connect callback, rc=%d\n", res);
}

void publish_callback(struct mosquitto *mosq, void* obj, int res) {
	//printf("publish callback, rp=%d\n", res);
	publish_msg = 0;
	publish_powquty_event = 0;
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
int mqtt_load_from_config() {
	int res= 0;

	// printf("looking up mqtt_host: currently ==> %s\n", mqtt_host);
	/*if(!config_lookup_string(get_cfg_ptr(), "mqtt_host", &mqtt_host)) {
		res= -1;
	}*/
	// printf("looking up mqtt_host: currently ==> %s\n", mqtt_host);

	mqtt_host = config->mqtt_host;


	/*if(!config_lookup_string(get_cfg_ptr(), "mqtt_topic", &mqtt_topic)) {
		res= -1;
	}*/

	mqtt_topic = config->mqtt_topic;

	mqtt_uname = config->mqtt_uname;
	mqtt_pw = config->mqtt_pw;

/*
	if(!config_lookup_string(get_cfg_ptr(), "dev_uuid", &dev_uuid)) {
		res= -1;
	}*/

	dev_uuid = config->dev_uuid;
	printf("dev_uuid: %s\n", dev_uuid);

	dev_lat = config->dev_lat;
	dev_lon = config->dev_lon;

	/*if(!config_lookup_string(get_cfg_ptr(), "dev_gps", &dev_gps)) {
		res= -1;
	}

	if(!config_lookup_string(get_cfg_ptr(), "dev_FW_ver", &dev_FW_ver)) {
		res= -1;
	}

	if(!config_lookup_string(get_cfg_ptr(), "dev_APP_ver", &dev_APP_ver)) {
		res= -1;
	}

	if(!config_lookup_string(get_cfg_ptr(), "dev_HW_ver", &dev_HW_ver)) {
		res= -1;
	}

	if(!config_lookup_int(get_cfg_ptr(), "powqutyd_print", &powqutyd_print)) {
		res= -1;
	}*/

	/* event handling variables */
	mqtt_event_host = config->mqtt_event_host;
	mqtt_event_topic = config->mqtt_event_topic;
	mqtt_event_flag = config->mqtt_event_flag;

	powqutyd_print = config->powqutyd_print;
	printf("powqutyd_print: %d\n", powqutyd_print);

	return res;
}

int mqtt_init (struct powquty_conf* conf) {
	int res = 0;
	config = conf;
	//if(is_config_loaded()) {
	res = mqtt_load_from_config();
	//}
	int vers [] = {0,0,0};
	int ret = mosquitto_lib_version(&vers[0], &vers[1], &vers[2]);
	printf("MQTT_LIB_VERSION: \tRet: %d\tMaj: %d\tMin: %d\tRev: %d\n",ret, vers[0], vers[1], vers[2]);
	printf("DEBUG:\tCreating MQTT Thread\n");
	res = pthread_create(&mosquitto_thread,NULL, mosquitto_thread_main,NULL);
	return res;
}

void publish_device_gps() {
	// UUID,TIMESTAMP,2,LATITUDE, LONGITUDE,ACCURANCY,PROVIDER,NETFREQ
	payload[0] = '\0';
	struct timeval tv;
	gettimeofday(&tv,NULL);
	sprintf(payload,"%s,%lu.%lu,2,%s,%s,0,2,50",dev_uuid,tv.tv_sec, (long int)tv.tv_usec/100, dev_lat, dev_lon);
	mqtt_publish_payload();
}

void publish_device_offline() {
	payload[0] = '\0';
	struct timeval tv;
	gettimeofday(&tv,NULL);
	sprintf(payload,"%s,%lu.%lu,0",dev_uuid,tv.tv_sec, (long int)tv.tv_usec/100);
	mqtt_publish_payload();
}

void publish_device_online() {
	payload[0] = '\0';
	struct timeval tv;
	gettimeofday(&tv,NULL);
	sprintf(payload,
			"%s,%lu.%lu,1,%s,%s,%s",
			dev_uuid,
			tv.tv_sec, (long int)tv.tv_usec/100,
			dev_FW_ver,
			dev_APP_ver,
			dev_HW_ver);
	mqtt_publish_payload();
}

void mqtt_publish_event() {
	if (powqutyd_print)
		printf("%s\n", event_payload);

	if (mqtt_event_flag)
		publish_powquty_event = 1;
}

void publish_event(const char *event) {
	event_payload[0] = '\0';
	struct timeval tv;
	gettimeofday(&tv, NULL);
	snprintf(event_payload, MAX_MQTT_MSG_LEN, "%s,%s,%lu,%lu",
			dev_uuid,
			event,
			tv.tv_sec,
			(long int)tv.tv_usec/100
		);
	mqtt_publish_event();
}

void publish_measurements(PQResult pqResult) {
	// printf("publish_measurements: \n");
	payload[0] = '\0';
	//long long ts = get_curr_time_in_milliseconds();
	struct timeval tv;
	gettimeofday(&tv, NULL);
	//long ts_sec = get_curr_time_in_seconds();
	sprintf(payload,
			//"%s,%ld,%lld,3,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f",
			"%s,%lu.%lu,3,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f",
			dev_uuid,
			tv.tv_sec, (long int)tv.tv_usec/100,
			//ts,
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
	if(powqutyd_print) {
		printf("%s\n",payload);
	}
	// TODO lock Mutex
	publish_msg = 1;
}

void mqtt_publish_msg(const char* msg) {
	// TODO lock Mutex
	payload[0] = '\0';
	sprintf(payload,"%s",msg);
	publish_msg = 1;
}

int mqtt_publish(struct mosquitto *m, const char *msg, const char *topic) {
	int res;
	unsigned int len = (unsigned int)strlen(msg);
	res =  mosquitto_publish(m, NULL, topic, len, (const unsigned char *)msg, 0, true);
	return res;
}

static void *mosquitto_thread_main(void* param) {
	printf("DEBUG:\tMQTT Thread has started\n");
	char buff[250];
	char* clientid = (char *) dev_uuid;
	char *event_id = malloc(strlen(clientid) + strlen("_event") + 1);
	strcat(event_id, clientid);
	strcat(event_id, "_event");

	int mosq_loop= 0, rc = 0, pub_res = 0;
	// char payload_msg[250] ="";

	mosquitto_lib_init();
	mosq = mosquitto_new(clientid, true, 0);
	if (mqtt_event_flag)
		event_mosq = mosquitto_new(event_id, true, 0);

	if(mosq){
		mosquitto_username_pw_set (mosq, mqtt_uname, mqtt_pw);
		//mosquitto_threaded_set(mosq, true); ==> setting it to true seem to hinder device_offline msg.
		rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
		if(rc != MOSQ_ERR_SUCCESS) {
			printf("Error: mosquitto_connect\n");
		}

		if (mqtt_event_flag) {
			rc = mosquitto_connect(event_mosq, mqtt_event_host, mqtt_port, 15);
			if (rc != MOSQ_ERR_SUCCESS)
				printf("Error: mosquitto_connect event\n");

			mosquitto_connect_callback_set(event_mosq, connect_callback);
			mosquitto_connect_callback_set(event_mosq, publish_callback);
		}

		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_publish_callback_set(mosq, publish_callback);

		while (!mosquitto_thread_stop) {
			mosq_loop = mosquitto_loop(mosq, -1, 1);
			if (mosq_loop) {
				printf("WARNING:\tLoop Failed: %d\t", mosq_loop);
				strerror_r(mosq_loop,buff,250);
				printf("%s\n",buff);
				//printf("Do something! like reconnect?\n");
				rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
				if(rc != MOSQ_ERR_SUCCESS) {
					printf("Error: mosquitto_connect\n");
				}

				if (mqtt_event_flag) {
					rc = mosquitto_connect(event_mosq,
						mqtt_event_host, mqtt_port, 60);
					if (rc != MOSQ_ERR_SUCCESS)
					printf("Error: mosquitto_connect event\n");
				}
				//break;
			} else {
				if(publish_msg) {
					pub_res = mqtt_publish(mosq, payload,
							mqtt_topic);
					if (pub_res != MOSQ_ERR_SUCCESS) {
						printf("Error: mosquitto_publish\n");
					}
				}
				if (publish_powquty_event) {
					pub_res = mqtt_publish(event_mosq,
							       event_payload,
							       mqtt_event_topic);
					if (pub_res != MOSQ_ERR_SUCCESS)
						printf("Error: publishing event\n");
					publish_powquty_event = 0;
				}
			}
		}
		mosquitto_disconnect(mosq);
		mosquitto_destroy(mosq);
		if (mqtt_event_flag) {
			mosquitto_disconnect(event_mosq);
			mosquitto_destroy(event_mosq);
		}
	}

	mosquitto_lib_cleanup();
	return NULL;
}
#endif /* MQTT */
