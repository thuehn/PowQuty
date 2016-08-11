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


pPQInstance pPQInst = NULL;
PQConfig pqConfig;
PQInfo pqInfo;
PQResult pqResult;

static pthread_t calculation_thread;
static void *calculation_thread_run(void* param);
static volatile unsigned int stop_calculation_run = 0, data_ready=0, buffer_idx=0;

int calculation_init() {
	int res=0;
	PQ_ERROR err = PQ_NO_ERROR;

	if(!retrieval_init(device_tty)) {
		printf("Retrieval Thread started \n");
	} else {
		printf("couldn't start Retrieval-Thread\n");
		return -1;
	}

	pqConfig.sampleRate = 10240;
	pqConfig.HW_offset = get_hw_offset();
	pqConfig.HW_scale = get_hw_scaling();

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
			printf("calculating @ idx: %d\n", buffer_idx);
			data_ready=0;
		}
	}
	return NULL;
}

void do_calculation(unsigned int stored_frame_idx) {
	data_ready = 1;
	buffer_idx = stored_frame_idx;
}

void stop_calculation() {
	stop_retrieval();
	// stop calculation
	stop_calculation_run=1;
    pthread_join(calculation_thread, NULL);
	// destroy PQInstance
    destroyPowerQuality(&pPQInst);
}

