/*
 * calculation.c
 *
 *  Created on: Aug 11, 2016
 *      Author: neez
 */

#include "calculation.h"
#include "retrieval.h"
#include "PQ_App.h"
#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "retrieval.h"
#include <string.h>


pPQInstance pPQInst = NULL;
PQConfig pqConfig;
PQInfo pqInfo;
PQResult pqResult;

static pthread_t calculation_thread;
static void *calculation_thread_run(void* param);
static volatile unsigned int stop_calculation_run = 0, data_ready=0, buffer_data_start_idx=0;
float hw_offset= 0.0, hw_scale=0.0;

short block_buffer[BLOCK_BUFFER_SIZE];
long long timestamp_buffer[TS_BUFFER_SIZE];
float in[SAMPLES_PER_BLOCK];

print_from_buffer(unsigned int idx);

int calculation_init() {
	int res=0;
	PQ_ERROR err = PQ_NO_ERROR;

	if(!retrieval_init(device_tty)) {
		printf("Retrieval Thread started \n");
	} else {
		printf("couldn't start Retrieval-Thread\n");
		return -1;
	}
	memset(&block_buffer, 0, BLOCK_BUFFER_SIZE * sizeof(short));
	memset(&timestamp_buffer, 0, TS_BUFFER_SIZE * sizeof(long long));
    memset(&in, 0, SAMPLES_PER_BLOCK * sizeof(float));

	pqConfig.sampleRate = 10240;
	hw_offset = get_hw_offset();
	pqConfig.HW_offset = hw_offset;
	hw_scale = get_hw_scaling();
	pqConfig.HW_scale = hw_scale;

	err = createPowerQuality(&pqConfig, &pPQInst, &pqInfo);

	if(err == PQ_NO_ERROR) {
		// start calculation thread
		res = pthread_create(&calculation_thread,NULL, calculation_thread_run,NULL);

	} else {
		// TODO see what happend
		printf("error creating PQ_Instance, errno: %d\n", err);
		stop_retrieval();
		return -1;
	}

	return res;
}

static void *calculation_thread_run(void* param) {
	while(!stop_calculation_run) {
		if(data_ready) {
			// do the calculation
			printf("calculating @ idx: %d\n", buffer_data_start_idx );
			print_from_buffer(buffer_data_start_idx);

			data_ready=0;
		}
	}
	return NULL;
}

void do_calculation(unsigned int stored_frame_idx) {
	// printf("stored Frame: %d\t", stored_frame_idx);
	data_ready = 1;
	if (stored_frame_idx<32) {
		buffer_data_start_idx = (stored_frame_idx + 128);
	} else  {
		buffer_data_start_idx = (stored_frame_idx - 32);
	}
	buffer_data_start_idx %=TS_BUFFER_SIZE;
}

void stop_calculation() {
	stop_retrieval();
	// stop calculation
	stop_calculation_run=1;
    pthread_join(calculation_thread, NULL);
	// destroy PQInstance
    destroyPowerQuality(&pPQInst);
}

void store_data(unsigned char * buf, unsigned int stored_frame_idx, long long ts){
	timestamp_buffer[stored_frame_idx] = ts;
	int i = 0;
	int buffer_idx = stored_frame_idx*SAMPLES_PER_FRAME;
	buffer_idx%=BLOCK_BUFFER_SIZE;
		while(i<SAMPLES_PER_FRAME) {
			block_buffer[buffer_idx + i] = get_short_val(buf+2*i);
			i++;
		}
}

print_from_buffer(unsigned int idx) {
	int i;
	for (i=idx*SAMPLES_PER_FRAME; i<(idx*SAMPLES_PER_FRAME+ SAMPLES_PER_BLOCK); i++) {
		printf("%d ", block_buffer[i]);
	}
	printf("\n");
}
