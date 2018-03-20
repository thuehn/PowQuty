/*
 * mqtt.c
 *
 *  Created on: Jul 12, 2016
 *      Author: neez
 */
#ifdef MQTT
#include <errno.h>
#include <limits.h>
#include <mosquitto.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "config.h"
#include "helper.h"
#include "mqtt.h"
#include "uci_config.h"

#define STATIC_DATA_LENGTH 208
#define META_DATA_LENGTH 398
#define T5060_DATA_LENGTH 184
#define T1012_DATA_LENGTH 70

static const char* mqtt_host = "localhost";
static const char* mqtt_topic = "devices/update";
static const char* mqtt_uname = "username";
static const char* mqtt_pw = "password";
static const char* dev_uuid = "BERTUB001";
//static const char* dev_gps = "BERTUB001";
// static const char* dev_FW_ver = "0.1";
// static const char* dev_APP_ver = "0.1";
// static const char* dev_HW_ver = "029";
static int powqutyd_print = 0;

static void (*t5060_composer)(PQResult*);
static void (*t1012_composer)(PQResult*);

void publish_callback(struct mosquitto *mosq, void* obj, int res);
void mqtt_publish_payload();
static void *mosquitto_thread_main(void* param);
static pthread_t mosquitto_thread;

static char payload[MAX_MQTT_MSG_LEN];
static char metadata[META_DATA_LENGTH];
static char static_data[STATIC_DATA_LENGTH];
static char t5060_data[T5060_DATA_LENGTH];
static char t1012_data[T1012_DATA_LENGTH];

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

static void composing_error(const char* function, const char* data, int size) {
	printf("Error: composing %s failed in %s: %s block exceeds available"
	       "space by %d character\n",data, function, data, size);
	exit(EXIT_FAILURE);
}

/*
 * construct static data json string
 * @param config: configuration struct
 */
static void compose_staticdata(struct powquty_conf* conf) {
	int ret;
	static_data[0] = '\0';
	ret = snprintf(static_data, STATIC_DATA_LENGTH,
		"\"acc\":%s, "
		"\"alt\":%s, "
		"\"id\":\"%s\", "
		"\"lat\":%s, "
		"\"lng\":%s,",
		conf->dev_acc,
		conf->dev_alt,
		conf->dev_uuid,
		conf->dev_lat,
		conf->dev_lon);

	if (ret > STATIC_DATA_LENGTH) {
		composing_error(__func__, "static data", ret - STATIC_DATA_LENGTH);
	}
}

/*
 * construct metadata json string
 * @param config: configuration struct
 */
static void compose_metadata(struct powquty_conf* conf) {
	int ret;
	metadata[0] = '\0';
	ret = snprintf(metadata, META_DATA_LENGTH,
		" \"metadata\": {"
		"\"comment\": \"%s\", "
		"\"id\": \"%s\", "
		"\"operator\": \"%s\", "
		"\"phase\": \"%s\", "
		"\"reason\": \"%s\", "
		"\"type\": \"%s\""
		"},",
		conf->meta_comment,
		conf->meta_id,
		conf->meta_operator,
		conf->meta_phase,
		conf->meta_reason,
		conf->meta_type);

	if (ret > META_DATA_LENGTH) {
		composing_error(__func__, "meta data", ret - META_DATA_LENGTH);
	}
}

/*
 * construct t5060 data json string
 * @param pqResult: PQResult struct containing frequency, harmonics and voltage
 * 		    data
 */
static inline volatile void compose_t5060_data(const PQResult* pqResult) {
	t5060_data[0] = '\0';
	sprintf(t5060_data,
		" \"t5060\": "
		"{ \"f\":%10.6f, "
		"\"u\":%11.6f, "
		"\"h3\":%10.6f, "
		"\"h5\":%10.6f, "
		"\"h7\":%10.6f, "
		"\"h9\":%10.6f, "
		"\"h11\":%10.6f, "
		"\"h13\":%10.6f, "
		"\"h15\":%10.6f },",
		pqResult->PowerFrequency5060T,
		pqResult->PowerVoltageEff_5060T,
		pqResult->Harmonics[0],
		pqResult->Harmonics[1],
		pqResult->Harmonics[2],
		pqResult->Harmonics[3],
		pqResult->Harmonics[4],
		pqResult->Harmonics[5],
		pqResult->Harmonics[6]);
}

/*
 * compose empty json string for t5060 data
 * @param pqResult: PQResult struct containing frequency, harmonics and voltage
 */
static inline void compose_empty_t5060_data(const PQResult* pqResult) {
	t5060_data[0]='\0';
}

/*
 * construct t1012 data json string
 * @param pqResult: PQResult struct containing frequency, harmonics and voltage
 * 		    data
 */
static inline volatile void compose_t1012_data(const PQResult* pqResult) {
	t1012_data[0] = '\0';
	sprintf(t1012_data,
		" \"t1012\": {"
		"\"f\": %10.6f/%10.6f, "
		"\"u\": %11.6f/%11.6f},",
		pqResult->PowerFrequency1012T[0],
		pqResult->PowerFrequency1012T[1],
		pqResult->PowerVoltageEff_1012T[0],
		pqResult->PowerVoltageEff_1012T[1]);
}

/*
 * compose empty json string for t1012 data
 * @param pqResult: PQResult struct containing frequency, harmonics and voltage
 */
static inline void compose_empty_t1012_data(const PQResult* pqResult) {
	t1012_data[0] = '\0';
}

int mqtt_load_from_config(struct powquty_conf* config) {
	int res= 0;

	compose_staticdata(config);

	// printf("looking up mqtt_host: currently ==> %s\n", mqtt_host);
	/*if(!config_lookup_string(get_cfg_ptr(), "mqtt_host", &mqtt_host)) {
		res= -1;
	}*/
	// printf("looking up mqtt_host: currently ==> %s\n", mqtt_host);

	mqtt_host = config->mqtt_host;


	/*if(!config_lookup_string(get_cfg_ptr(), "mqtt_topic", &mqtt_topic)) {
		res= -1;
	}*/
/*  */
	mqtt_topic = config->mqtt_topic;

	mqtt_uname = config->mqtt_uname;
	mqtt_pw = config->mqtt_pw;

/*
	if(!config_lookup_string(get_cfg_ptr(), "dev_uuid", &dev_uuid)) {
		res= -1;
	}*/

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
		metadata[0] = '\0';
	}

	powqutyd_print = config->powqutyd_print;

	return res;
}

int mqtt_init (struct powquty_conf* conf) {
	int res = 0;
	int vers [] = {0,0,0};
	int ret = mosquitto_lib_version(&vers[0], &vers[1], &vers[2]);
	res = mqtt_load_from_config(conf);
	printf("send_t1012_data: %d, send_t5060_data: %d\n",
			conf->send_t1012_data,
			conf->send_t5060_data);

	/* select composer function for t5060 data */
	if (conf->send_t5060_data == 1) {
		t5060_composer = (void (*))&compose_t5060_data;
	} else {
		t5060_composer = (void (*))&compose_empty_t5060_data;
	}

	/* select composer function for t1012 data */
	if (conf->send_t1012_data == 1) {
		t1012_composer = (void (*))&compose_t1012_data;
	} else {
		t1012_composer = (void (*))&compose_empty_t1012_data;
	}

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
	static unsigned long long pkg_count = 0;

	struct timeval tv;
	struct tm *nowtm;
	time_t nowtime;
	char tmbuf[64];
	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = gmtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);

	t5060_composer(&pqResult);
	t1012_composer(&pqResult);
	payload[0] = '\0';
	sprintf(payload,
			//"%s,%ld,%lld,3,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f",
			// "%s,%lu.%lu,3,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f",
			"{"
			"%s"			//static data
			"%s"			//metadata (optional object)
			"\"pkg\":\"%llu\","	//pkg count
			"%s"			//t5060 data
			"%s"			//t1012 data
			"\"utc\":\"%s.%lu\" "
			"}",
			static_data,
			metadata,
			pkg_count,
			t5060_data,
			t1012_data,
			tmbuf,
			(long int)tv.tv_usec/1000);
	mqtt_publish_payload();

	if (pkg_count < ULLONG_MAX) {
		pkg_count++;
	} else {
		pkg_count = 0;
	}
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
		rc = mosquitto_connect(mosq, mqtt_host, MQTT_PORT, 60);
		if(rc != MOSQ_ERR_SUCCESS) {
			printf("Error: mosquitto_connect while first connecting "
				"to host:\t%s,at port\t%d\n",mqtt_host, MQTT_PORT);
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
				printf("\t\t\t--> reconnecting to host:\t%s,at port\t%d\n",
					mqtt_host,MQTT_PORT);
				rc = mosquitto_connect(mosq, mqtt_host, MQTT_PORT, 60);
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
