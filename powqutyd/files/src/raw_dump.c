/*
 * raw_dump.c
 *
 *  Created on: Feb 15, 2017
 *      Author: nadim
 */
#include "raw_dump.h"
#include <pthread.h>
#include "retrieval.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"
#include "time.h"

#define DUMP_BUFFER_SIZE	32
// DANGEROUS
#define MAX_DUMP_STRING		400

static pthread_cond_t dump_cond;
static pthread_mutex_t dump_mtx;
static pthread_t raw_dump_thread;

unsigned char * dump_buffer;		// [MAX_FRAME_SIZE*DUMP_BUFFER_SIZE];
char *dump_mode;			// [DUMP_BUFFER_SIZE];
long long *dump_curr_time;		// [DUMP_BUFFER_SIZE];
long long *dump_diff_time;		// [DUMP_BUFFER_SIZE];
int *dump_size;				// [DUMP_BUFFER_SIZE];
char *dump_string;			// [MAX_DUMP_STRING];
static volatile short new_pkt_idx=0, pkt_to_dump_idx=0;
static volatile unsigned int stop_raw_dump_run = 0;
unsigned short last_frame_idx =0;

void dump_to_string(short idx);

int raw_dump_init() {
	int res = 0;
	pthread_cond_init(&dump_cond, NULL);
	pthread_mutex_init(&dump_mtx,NULL);
	dump_buffer= calloc(sizeof (unsigned char),MAX_FRAME_SIZE*DUMP_BUFFER_SIZE);
	dump_mode= calloc(sizeof(char), DUMP_BUFFER_SIZE);
	dump_curr_time= calloc(sizeof(long long), DUMP_BUFFER_SIZE);
	dump_diff_time= calloc(sizeof(long long), DUMP_BUFFER_SIZE);
	dump_size= calloc(sizeof(int), DUMP_BUFFER_SIZE);
	dump_string = calloc(sizeof(char),MAX_DUMP_STRING);
	//memset(dump_buffer,0,MAX_FRAME_SIZE*DUMP_BUFFER_SIZE);
	memset(dump_mode,0,DUMP_BUFFER_SIZE);
	memset(dump_curr_time,0,DUMP_BUFFER_SIZE);
	memset(dump_diff_time,0,DUMP_BUFFER_SIZE);
	memset(dump_size,0,DUMP_BUFFER_SIZE);
	memset(dump_string,0,DUMP_BUFFER_SIZE);
	printf("DEBUG:\tCreating Dump Thread\t[n_idx: %d, td_idx: %d] \n",new_pkt_idx,pkt_to_dump_idx);
	res= pthread_create(&raw_dump_thread, NULL, raw_dump_run, NULL);
	return res;
}


void raw_dump_stop() {
	printf("DEBUG:\tStopping Dump Thread \n");
	stop_raw_dump_run =1;
	pthread_cond_signal(&dump_cond);
}

void dump_raw_packet(unsigned char* frame, int read_size, char mode) {
	struct timespec ts_curr, ts_diff;

	// TODO printf("DEBUG:\t ended\n");
	memset(dump_buffer+MAX_FRAME_SIZE*new_pkt_idx, 0, MAX_FRAME_SIZE);
	memcpy(&dump_buffer[MAX_FRAME_SIZE*new_pkt_idx],
			frame, MAX_FRAME_SIZE);
	dump_mode[new_pkt_idx] = mode;
	dump_size[new_pkt_idx] = read_size;

	clock_gettime(CLOCK_REALTIME, &ts_curr);
	clock_gettime(CLOCK_MONOTONIC, &ts_diff);
	dump_curr_time[new_pkt_idx] = ts_curr.tv_sec * 1000000 + ts_curr.tv_nsec;
	dump_diff_time[new_pkt_idx] = ts_diff.tv_sec * 1000000 + ts_diff.tv_nsec;

	new_pkt_idx = (new_pkt_idx+1)%DUMP_BUFFER_SIZE;
	if(!stop_raw_dump_run) {
		pthread_mutex_lock(&dump_mtx);
		pthread_cond_signal(&dump_cond);
		pthread_mutex_unlock(&dump_mtx);
	}
}

void* raw_dump_run(void* args) {
	printf("DEBUG:\tDump Thread has started with Thread Id: %lu\n",
		(long unsigned int)pthread_self());
	while(!stop_raw_dump_run) {
		pthread_mutex_lock(&dump_mtx);
		pthread_cond_wait(&dump_cond,&dump_mtx);
		pthread_mutex_unlock(&dump_mtx);
		if(new_pkt_idx == pkt_to_dump_idx) printf("ERROR:\tdump-buffer overflow\n");
		while (new_pkt_idx != pkt_to_dump_idx) {
			dump_to_string(pkt_to_dump_idx);
			printf("%s",dump_string);
			pkt_to_dump_idx = (pkt_to_dump_idx+1)%DUMP_BUFFER_SIZE;
		}
	}
	printf("DEBUG:\tDump Thread has ended\n");
	return 0;
}

void dump_to_string(short idx) {
	int flag =0;
	unsigned short curr_idx =get_unsigned_short_val(&dump_buffer[idx*MAX_FRAME_SIZE+4]);
			//(unsigned short) (dump_buffer[idx+5] <<8 |dump_buffer[idx+4]);

	unsigned short curr_len = get_unsigned_short_val(&dump_buffer[idx*MAX_FRAME_SIZE+2]);
			// (unsigned short) (dump_buffer[idx+3] << 8 |dump_buffer[idx+2]);
	if(dump_buffer[idx] == 0x05 && dump_size[idx] !=134) {
		printf("DUMP-WARNING - READ_SIZE != 134\t read_size=%d\n", dump_size[idx]);
		flag =1;
	}

	if (dump_buffer[idx] == 0x05 && (last_frame_idx+1)%65536 != curr_idx%65536) {
		printf("DUMP-WARNING - Frame Got Missing:\tlast_Frame_idx: %d,\tcurr_Frame_idx: %d\n", last_frame_idx, curr_idx);
		flag=1;
	}

	if(dump_buffer[idx] == 0x05 && (130 != curr_len)) {
		printf("WARNING - Packet with unexpected Data-Length: \tLEN: %d\n", curr_len);
		flag=1;
	}
	if(dump_mode[idx] == 'r') {
		sprintf(dump_string, "-> %lld, %lld: [%03d-%03d-%d] \t",
			dump_curr_time[idx],
			dump_diff_time[idx],
			dump_size[idx],
			curr_len,
			curr_idx);
	}
	else {
		sprintf(dump_string, "<- %lld, %lld: [%03d-%03d-%d] \t",
			dump_curr_time[idx],
			dump_diff_time[idx],
			dump_size[idx],
			curr_len,
			curr_idx);
	}
	//for (int i = 0; i<dump_size[idx]; i++) {
	for (int i = 0; i<MAX_FRAME_SIZE; i++) {
		sprintf(dump_string +strlen(dump_string), "%02X", dump_buffer[idx*MAX_FRAME_SIZE + i]);
		if (!((i-5)%8)&& i>=5) {
			sprintf(dump_string +strlen(dump_string), " ");
		}
	}
	sprintf(dump_string + strlen(dump_string), "\n");
	last_frame_idx=curr_idx;
	if(flag) {
		printf("%s", dump_string);
	}
}

void raw_dump_join() {
	printf("DEBUG:\tJoining Dump Thread \n");
	pthread_join(raw_dump_thread, NULL);
	pthread_cond_destroy(&dump_cond);
	pthread_mutex_destroy(&dump_mtx);
	printf("DEBUG:\tDump Thread Joined \n");
}
