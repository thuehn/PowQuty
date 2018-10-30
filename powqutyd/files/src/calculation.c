/*
 * calculation.c
 *
 *  Created on: Aug 11, 2016
 *      Author: neez
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "calculation.h"
#include "config.h"
#include "helper.h"
#include "main.h"

#ifdef MQTT
#include "mqtt.h"
#endif

#include "retrieval.h"

const char* device_tty;

#define SAMPLE_FREQUENCY 10240

static pthread_t calculation_thread;

static pthread_cond_t calc_cond;
static pthread_mutex_t calc_mtx;

pPQInstance pPQInst = NULL;
PQConfig pqConfig;
PQInfo pqInfo;
PQResult pqResult;
PQ_ERROR err = PQ_NO_ERROR;

static void *calculation_thread_run(void* param);

/*
 * It is the index of the first frame of the block.
 * The following assumptions should be met:
 * ===> idx % 32 = 0 !
 * ===> idx <= (TS_BUFFER_SIZE -32)
 */
static volatile unsigned int buffer_data_start_idx=0;
static volatile unsigned int stop_calculation_run = 0;
static volatile int data_ready = 0;
float hw_offset = 0.0, hw_scale = 0.0;

// short block_buffer[BLOCK_BUFFER_SIZE];
//long long timestamp_buffer[TS_BUFFER_SIZE];
//float in[SAMPLES_PER_BLOCK];
short *block_buffer;
long long *timestamp_buffer;
float *in;

struct powquty_conf* config;

struct en50160_event *event;

void load_data_to_in();
void print_from_buffer();
void print_in_signal();
void print_from_ts_buffer();
void print_results();

int calculation_load_from_config()
{
	int res = 0;

	device_tty = "/dev/ttyACM3";
	device_tty = config->device_tty;
	printf("DEBUG:\t Looking up device_tty!!!! currently ==> %s\n", device_tty);

	return res;
}

int calculation_init(struct powquty_conf* conf)
{
	int res=0;

	block_buffer = calloc(sizeof(short),BLOCK_BUFFER_SIZE);
	timestamp_buffer = calloc(sizeof(long long),TS_BUFFER_SIZE);
	in = calloc(sizeof(float),SAMPLES_PER_BLOCK);
	config = conf;

	if (is_config_loaded())
		res= calculation_load_from_config();

	/* start RETRIVAL thread*/
	if (!retrieval_init(config->device_tty)) {
		printf("DEBUG:\t Retrieval Thread started \n");
	} else {
		printf("ERROR:\t couldn't start Retrieval-Thread\n");
		return -1;
	}

	pqConfig.sampleRate = SAMPLE_FREQUENCY;
	pqConfig.HW_offset = get_hw_offset();
	pqConfig.HW_scale = get_hw_scaling();

	err = createPowerQuality(&pqConfig, &pPQInst, &pqInfo);

	pthread_cond_init(&calc_cond, NULL);
	pthread_mutex_init(&calc_mtx,NULL);

	/* start CALCULATION thread */
	if (err == PQ_NO_ERROR) {
		res = pthread_create(&calculation_thread,NULL, calculation_thread_run,NULL);
	} else {
		// TODO see what happend
		printf("ERROR:\t error creating PQ_Instance, errno: %d\n", err);
		stop_retrieval();
		return -1;
	}

	return res;
}

static void *calculation_thread_run(void* param)
{
	unsigned char ret = 0;
	printf("DEBUG:\t Calculation Thread has started\n");

	while (!stop_calculation_run) {
		pthread_mutex_lock(&calc_mtx);
		while (data_ready <= 0)
			pthread_cond_wait(&calc_cond,&calc_mtx);

		if (data_ready == 1) {
			load_data_to_in();
			data_ready--;
			pthread_mutex_unlock(&calc_mtx);
		} else if (data_ready <= NUMBER_OF_BLOCKS_IN_BUFFER) {
			// printf("WARNING:\t\t\t Processing is slower then Data received!\t %d Blocks late \n", data_ready-1);
			load_data_to_in();
                        data_ready--;
			pthread_mutex_unlock(&calc_mtx);
		} else {
			printf("ERROR:\t BLOCK_BUFFER_OVERFLOW! %d Blocks late\n\tProcessing is much slower then Data arriving\n", data_ready-1);
			pthread_mutex_unlock(&calc_mtx);
			exit(EXIT_FAILURE);
		}

		// apply the PQ_lib
		err = applyPowerQuality(
				pPQInst,
				in,
				&pqResult,
				NULL,
				timestamp_buffer+buffer_data_start_idx,
				FRAMES_PER_BLOCK);

		/* exit processing on error */
		if (err != PQ_NO_ERROR) {
			/* TODO: handle PQ-Lib Errors */
			printf("ERROR:\tapplying PQ-Lib\n");
			print_PQ_Error(err);
			stop_powqutyd();
			break;
		}

		/* EN50160 event detected */
		if (pqResult.nmbPqEvents > 0) {
			ret = handle_event(pqResult, config);
		}

		if (ret) {
			stop_calculation_run = 1;
			break;
		}

		if (pqResult.HarmonicsExist) {
			store_to_file(pqResult, config);
#ifdef MQTT
			publish_measurements(pqResult);
#endif
		}
	}
	printf("DEBUG:\t Calculation Thread has ended\n");

	return NULL;
}

void do_calculation(unsigned int stored_frame_idx)
{
	/*
	 * buffer_data_start_idx is the index of the first frame of the block.
	 * The following assumptions should be met:
	 * ===> idx % 32 = 0 !
	 * ===> idx <= (TS_BUFFER_SIZE -32)
	 */
	if (stored_frame_idx % 32) {
		printf("ERROR:\t Error in retrieval: Stored frame idx given for"
		       "calculation is not a multiple of FRAMES_PER_BLOCK\n");
		return;
	}

	pthread_mutex_lock(&calc_mtx);
	data_ready++;

	/*
	if (stored_frame_idx < 32) {
		buffer_data_start_idx = (stored_frame_idx + 128);
	} else  {
		buffer_data_start_idx = (stored_frame_idx - 32);
	}
	*/
	buffer_data_start_idx = stored_frame_idx - FRAMES_PER_BLOCK;
	buffer_data_start_idx %= TS_BUFFER_SIZE;

	pthread_cond_signal(&calc_cond);
	pthread_mutex_unlock(&calc_mtx);
}

void stop_calculation()
{
	printf("DEBUG:\t Stopping Calculation Thread\n");
	stop_retrieval();
	// stop calculation
	stop_calculation_run = 1;
	pthread_cond_signal(&calc_cond);
	//join_calculation();		// TODO: solve segfault on sigint

	// destroy PQInstance
	destroyPowerQuality(&pPQInst);
}

void join_calculation()
{
	printf("DEBUG:\t Joining Calculation Thread\n");
	join_retrieval();
	pthread_join(calculation_thread, NULL);
	pthread_cond_destroy(&calc_cond);
	pthread_mutex_destroy(&calc_mtx);
	printf("DEBUG:\t Calculation Thread Joined \n");
}

void store_data(unsigned char * buf, unsigned int stored_frame_idx, long long ts)
{
	int i = 0;
	int buffer_idx = stored_frame_idx*SAMPLES_PER_FRAME;

	timestamp_buffer[stored_frame_idx] = ts;
	buffer_idx%=BLOCK_BUFFER_SIZE;
	while (i<SAMPLES_PER_FRAME) {
		block_buffer[buffer_idx + i] = get_short_val(buf + 2 * i);
		i++;
	}
}


void print_from_buffer()
{
	unsigned int i;

	for (i = buffer_data_start_idx*SAMPLES_PER_FRAME;
	     i < (buffer_data_start_idx * SAMPLES_PER_FRAME+ SAMPLES_PER_BLOCK); i++) {
		printf("%d ", block_buffer[i]);
	}
	printf("\n");
}

void print_in_signal() {
	int i = 0;

	for (i = 0;i < SAMPLES_PER_BLOCK; i++) {
		printf("%f ", in[i]);
	}
	printf("\n");
}

void print_from_ts_buffer() {
	unsigned int i;

	for (i = buffer_data_start_idx;
	     i < (buffer_data_start_idx + FRAMES_PER_BLOCK); i++) {
		printf("%lld ", timestamp_buffer[i]);
	}
	printf("\n");
}

void load_data_to_in()
{
	int buffer_idx=buffer_data_start_idx*SAMPLES_PER_FRAME;
	int in_idx = 0;
	float scale = 1.0f;

	for (in_idx = 0; in_idx < SAMPLES_PER_BLOCK; in_idx++) {
		in[in_idx] = scale * block_buffer[buffer_idx + in_idx];
	}
}

void print_results()
{
	printf("Results: \n");
	if (pqResult.PowerVoltageEff5060TExist) {
		printf("PowerVoltageEff_5060T: %f\n", pqResult.PowerVoltageEff_5060T);
	}
	if (pqResult.PowerFrequency5060TExist) {
		printf("PowerFrequency5060T: %f\n", pqResult.PowerFrequency5060T);
	}
	if (pqResult.HarmonicsExist) {
		printf("Harmonics: %f %f %f %f %f %f %f\n",
				pqResult.Harmonics[0],
				pqResult.Harmonics[1],
				pqResult.Harmonics[2],
				pqResult.Harmonics[3],
				pqResult.Harmonics[4],
				pqResult.Harmonics[5],
				pqResult.Harmonics[6] );
	}
	if (pqResult.PowerFrequency1012TExist[0]) {
		printf("PowerFrequency1012T [0]: %f\n", pqResult.PowerFrequency1012T[0]);
	}
	if (pqResult.PowerFrequency1012TExist[1]) {
		printf("PowerFrequency1012T [1]: %f\n", pqResult.PowerFrequency1012T[1]);
	}
	if (pqResult.PowerVoltage1012TExist[0]) {
		printf("PowerVoltage1012T [0]: %f\n", pqResult.PowerVoltageEff_1012T[0]);
	}
	if (pqResult.PowerVoltage1012TExist[1]) {
		printf("PowerVoltageEff_1012T [1]: %f\n", pqResult.PowerVoltageEff_1012T[1]);
	}

}
