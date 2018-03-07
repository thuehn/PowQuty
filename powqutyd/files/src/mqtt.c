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
#include <errno.h>

static const char* mqtt_host = "localhost";
static const char* mqtt_topic = "devices/update";
static const char* mqtt_uname = "username";
static const char* mqtt_pw = "password";
static const char* dev_uuid = "BERTUB001";
static const char* dev_lat = "55.0083525";
static const char* dev_lon = "82.935732";
//static const char* dev_gps = "BERTUB001";
// static const char* dev_FW_ver = "0.1";
// static const char* dev_APP_ver = "0.1";
// static const char* dev_HW_ver = "029";
static int powqutyd_print = 0;

struct powquty_conf* config;
void publish_callback(struct mosquitto *mosq, void* obj, int res);
void mqtt_publish_payload();
static void *mosquitto_thread_main(void* param);
static pthread_t mosquitto_thread;

static char payload[MAX_MQTT_MSG_LEN];
static char metadata[512];

struct mosquitto *mosq;

void mosq_str_err(int mosq_errno) {
	switch (mosq_errno) {
	case MOSQ_ERR_NOMEM:
		printf("MOSQ_ERR_NOMEM:\tout of memory condition occurred.");
		break;
	case MOSQ_ERR_PROTOCOL:
		printf("MOSQ_ERR_PROTOCOL:\tthere is a protocol error communicating with the broker.");
		break;
	case MOSQ_ERR_INVAL:
		printf("MOSQ_ERR_INVAL:\tInput parameters were invalid.");
		break;
	case MOSQ_ERR_NO_CONN:
		printf("MOSQ_ERR_NO_CONN:\tthe client isn't connected to a broker.");
		break;
	case MOSQ_ERR_CONN_REFUSED:
		printf("MOSQ_ERR_CONN_REFUSED:\t.");
		break;
	case MOSQ_ERR_NOT_FOUND:
		printf("MOSQ_ERR_NOT_FOUND:\t.");
		break;
	case MOSQ_ERR_CONN_LOST:
		printf("MOSQ_ERR_CONN_LOST:\tthe connection to the broker was lost.");
		break;
	case MOSQ_ERR_TLS:
		printf("MOSQ_ERR_TLS:\t.");
		break;
	case MOSQ_ERR_PAYLOAD_SIZE:
		printf("MOSQ_ERR_PAYLOAD_SIZE:\t.");
		break;
	case MOSQ_ERR_NOT_SUPPORTED:
		printf("MOSQ_ERR_NOT_SUPPORTED:\t.");
		break;
	case MOSQ_ERR_AUTH:
		printf("MOSQ_ERR_AUTH:\t.");
		break;
	case MOSQ_ERR_ACL_DENIED:
		printf("MOSQ_ERR_ACL_DENIED:\t.");
		break;
	case MOSQ_ERR_UNKNOWN:
		printf("MOSQ_ERR_UNKNOWN:\t.");
		break;
	case MOSQ_ERR_ERRNO:
		printf("MOSQ_ERR_ERRNO:\tSystem call returned an error.");
		printf("\t\t-> ERROR:\t %s\n", strerror(errno));
		break;
	default:
			break;
	}
}

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

/* construct metadata json string
 * @param config: configuration struct
 */
void compose_metadata(struct powquty_conf* conf) {
	metadata[0] = '\0';
	sprintf(metadata,
			"\"comment\": \"%s\", "
			"\"id\": \"%s\", "
			"\"operator\": \"%s\", "
			"\"phase\": \"%s\", "
			"\"reason\": \"%s\", "
			"\"type\": \"%s\"",
			conf->meta_comment,
			conf->meta_id,
			conf->meta_operator,
			conf->meta_phase,
			conf->meta_reason,
			conf->meta_type);
}

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

	if (config->use_metadata) {
		compose_metadata(config);
	} else {
		sprintf(metadata, "");
	}

	powqutyd_print = config->powqutyd_print;

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
	/*
	// UUID,TIMESTAMP,2,LATITUDE, LONGITUDE,ACCURANCY,PROVIDER,NETFREQ
	payload[0] = '\0';
	struct timeval tv;
	gettimeofday(&tv,NULL);
	sprintf(payload,"%s,%lu.%lu,2,%s,%s,0,2,50",dev_uuid,tv.tv_sec, (long int)tv.tv_usec/100, dev_lat, dev_lon);
	mqtt_publish_payload();
	*/
}

void publish_device_offline() {
	/*
	payload[0] = '\0';
	struct timeval tv;
	gettimeofday(&tv,NULL);
	sprintf(payload,"%s,%lu.%lu,0",dev_uuid,tv.tv_sec, (long int)tv.tv_usec/100);
	mqtt_publish_payload();
	*/
}

void publish_device_online() {
	/*
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
	*/
}

void publish_measurements(PQResult pqResult) {
	// printf("publish_measurements: \n");
	payload[0] = '\0';
	//long long ts = get_curr_time_in_milliseconds();
	struct timeval tv;
	gettimeofday(&tv, NULL);

	time_t nowtime;
	struct tm *nowtm;
	char tmbuf[64];
	nowtime = tv.tv_sec;
	nowtm = gmtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);

	//long ts_sec = get_curr_time_in_seconds();
	sprintf(payload,
			//"%s,%ld,%lld,3,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f",
			// "%s,%lu.%lu,3,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f",
			"{\"id\":\"%s\", "
			"\"utc\":\"%s.%lu\", "
			"\"pkg\":\"0\", "
			"\"lat\":%s, "
			"\"lng\":%s, "
			"\"acc\":0.0, "
			"\"metadata\": {%s}, "
			"\"t5060\": "
			"{ \"u\":%.6f, "
			"\"f\":%.6f, "
			"\"h3\":%.6f, "
			"\"h5\":%.6f, "
			"\"h7\":%.6f, "
			"\"h9\":%.6f, "
			"\"h11\":%.6f, "
			"\"h13\":%.6f, "
			"\"h15\":%.6f } }",
			dev_uuid,
			tmbuf, (long int)tv.tv_usec/1000,
			dev_lat,
			dev_lon,
			metadata,
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
	printf("DEBUG:\tMQTT Thread has started \t mqtt_host:%s\n",mqtt_host);
	char buff[250];
	char* clientid = (char *) dev_uuid;
	int mosq_loop= 0, rc = 0, pub_res = 0;
	// char payload_msg[250] ="";

	mosquitto_lib_init();
	mosq = mosquitto_new(clientid, true, 0);

	if(mosq){
		mosquitto_username_pw_set (mosq, mqtt_uname, mqtt_pw);
		//mosquitto_threaded_set(mosq, true); ==> setting it to true seem to hinder device_offline msg.
		rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
		if(rc != MOSQ_ERR_SUCCESS) {
			printf("Error: mosquitto_connect while first connecting to host:\t%s,at port\t%d\n",mqtt_host,mqtt_port);
			printf("\t\t\t");
			mosq_str_err(rc);
			printf("\n");
		}

		mosquitto_connect_callback_set(mosq, connect_callback);
		mosquitto_publish_callback_set(mosq, publish_callback);
		while (!mosquitto_thread_stop) {
			mosq_loop = mosquitto_loop(mosq, -1, 1);
			if (mosq_loop) {
				printf("WARNING:\tLoop Failed: %d\t", mosq_loop);
				mosq_str_err(mosq_loop);
				//strerror_r(mosq_loop,buff,250);
				printf("%s\n",buff);
				printf("\t\t\t--> reconnecting to host:\t%s,at port\t%d\n",mqtt_host,mqtt_port);
				rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
				if(rc != MOSQ_ERR_SUCCESS) {
					printf("\t\t\t");
					mosq_str_err(rc);
					printf(" Error: mosquitto_connect\n");
				}
				//break;
			} else {
				// printf("DEBUG:\tMQTT-Loop: %d\t\n", mosq_loop);
				if(publish_msg) {
					pub_res = mqtt_publish(mosq, payload,
							mqtt_topic);
					if (pub_res != MOSQ_ERR_SUCCESS) {
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
#endif /* MQTT */
