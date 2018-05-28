/*
 * raw_dump.c
 *
 *  Created on: Feb 15, 2017
 *      Author: nadim
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file_checks.h"
#include "helper.h"
#include "raw_dump.h"
#include "retrieval.h"
#include "time.h"

#define DUMP_BUFFER_SIZE	32
// DANGEROUS
#define MAX_DUMP_STRING		400

static pthread_cond_t dump_cond;
static pthread_mutex_t dump_mtx;
static pthread_t raw_dump_thread;

unsigned char *dump_buffer;		// [MAX_FRAME_SIZE*DUMP_BUFFER_SIZE];
char *dump_mode;			// [DUMP_BUFFER_SIZE];
long long *dump_curr_time;		// [DUMP_BUFFER_SIZE];
long long *dump_diff_time;		// [DUMP_BUFFER_SIZE];
int *dump_size;				// [DUMP_BUFFER_SIZE];
char *dump_string;			// [MAX_DUMP_STRING];
char *raw_file;				// [MAX_PATH_LENGTH];
static volatile short new_pkt_idx = 0, pkt_to_dump_idx = 0;
static volatile unsigned int stop_raw_dump_run = 0;
unsigned short last_frame_idx = 0;

struct device_info {
	float device_offset;
	float device_scaling_factor;
};

void dump_to_string(struct device_info *di, short idx);

int dump_raw_to_file(const char *path) {
	if (path == NULL) {
		return EXIT_FAILURE;
	}

	if (strlen(path) >= MAX_PATH_LENGTH) {
		printf("ERROR:\t Path for raw file to long\n");
		return EXIT_FAILURE;
	}

	raw_file = malloc(sizeof(char) * MAX_PATH_LENGTH);
	if (raw_file == NULL) {
		printf("ERROR:\t Could not allocate memory in %s\n", __func__);
		return EXIT_FAILURE;
	} else {
		strcpy(raw_file, path);
		if (raw_file) {
		}
		return EXIT_SUCCESS;
	}

}

/*
 * Allocate memory for dump globals
 * return: 0 on success, 1 on failure
 */
int allocate_memory() {
	dump_buffer = calloc(sizeof(unsigned char), MAX_FRAME_SIZE * DUMP_BUFFER_SIZE);
	if (dump_buffer == NULL) {
		printf("Error:\t Could not allocate dump_buffer in %s: %s\n",
		       __func__, strerror(errno));
		return EXIT_FAILURE;
	}

	dump_mode = calloc(sizeof(char), DUMP_BUFFER_SIZE);
	if (dump_mode == NULL) {
		printf("Error:\t Could not allocate dump_mode in %s: %s\n",
		       __func__, strerror(errno));
		goto clean_dump_buffer;

	}

	dump_curr_time = calloc(sizeof(long long), DUMP_BUFFER_SIZE);
	if (dump_curr_time == NULL) {
		printf("Error:\t Could not allocate dump_curr_time in %s: %s\n",
		       __func__, strerror(errno));
		goto clean_dump_mode;
	}

	dump_diff_time = calloc(sizeof(long long), DUMP_BUFFER_SIZE);
	if (dump_diff_time == NULL) {
		printf("Error:\t Could not allocate dump_diff_time in %s: %s\n",
		       __func__, strerror(errno));
		goto clean_curr_time;
	}

	dump_size = calloc(sizeof(int), DUMP_BUFFER_SIZE);
	if (dump_size == NULL) {
		printf("Error:\t Could not allocate dump_size in %s: %s\n",
		       __func__, strerror(errno));
		goto clean_diff_time;
	}

	dump_string = calloc(sizeof(char), MAX_DUMP_STRING);
	if (dump_string == NULL) {
		printf("Error:\t Could not allocate dump_string in %s: %s\n",
		       __func__, strerror(errno));
		goto clean_dump_size;
	}

	return EXIT_SUCCESS;

clean_dump_size:
	free(dump_size);

clean_diff_time:
	free(dump_diff_time);

clean_curr_time:
	free(dump_curr_time);

clean_dump_mode:
	free(dump_mode);

clean_dump_buffer:
	free(dump_buffer);

	return EXIT_FAILURE;
}

void free_raw_memory() {
	free(dump_buffer);
	free(dump_mode);
	free(dump_curr_time);
	free(dump_diff_time);
	free(dump_size);
	free(dump_string);
}


int raw_dump_init(float device_offset, float device_scaling_factor) {
	int res = 0;
	struct device_info di;

	pthread_cond_init(&dump_cond, NULL);
	pthread_mutex_init(&dump_mtx, NULL);

	di.device_offset = device_offset;
	di.device_scaling_factor = device_scaling_factor;

	res = allocate_memory();

	if (res)
		exit(EXIT_FAILURE);

	printf("DEBUG:\t Creating Dump Thread\t[n_idx: %d, td_idx: %d]\n",
		new_pkt_idx, pkt_to_dump_idx);

	res = pthread_create(&raw_dump_thread, NULL, raw_dump_run, (void *)&di);

	return res;
}

void raw_dump_stop() {
	printf("DEBUG:\t Stopping Dump Thread\n");
	if (raw_file) {
		free(raw_file);
	}
	free_raw_memory();
	stop_raw_dump_run = 1;
	pthread_cond_signal(&dump_cond);
}

void dump_raw_packet(unsigned char* frame, int read_size, char mode) {
	struct timespec ts_curr, ts_diff;

	memset(dump_buffer + MAX_FRAME_SIZE * new_pkt_idx, 0, MAX_FRAME_SIZE);
	memcpy(&dump_buffer[MAX_FRAME_SIZE * new_pkt_idx], frame, MAX_FRAME_SIZE);
	dump_mode[new_pkt_idx] = mode;
	dump_size[new_pkt_idx] = read_size;

	/*Get current system & monotonic timestamps */
	clock_gettime(CLOCK_REALTIME, &ts_curr);
	dump_curr_time[new_pkt_idx] = ts_curr.tv_sec * 1000000 + ts_curr.tv_nsec;
	clock_gettime(CLOCK_MONOTONIC, &ts_diff);
	dump_diff_time[new_pkt_idx] = ts_diff.tv_sec * 1000000 + ts_diff.tv_nsec;

	/* Ring buffer */
	new_pkt_idx = (new_pkt_idx + 1) % DUMP_BUFFER_SIZE;

	if(!stop_raw_dump_run) {
		pthread_mutex_lock(&dump_mtx);
		pthread_cond_signal(&dump_cond);
		pthread_mutex_unlock(&dump_mtx);
	}
}

static void print_raw_to_file(const char *str) {
	FILE *fp = fopen(raw_file, "a");

	if (fp == NULL) {
		printf("ERROR:\t Could not open raw file in %s: %s\n",
		       __func__, strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(fp, "%s", str);
	fclose(fp);
}

void* raw_dump_run(void* args) {
	struct device_info *di;
	di = args;

	printf("DEBUG:\t Dump Thread has started with Thread Id: %lu\n",
	       (long unsigned int)pthread_self());
	while(!stop_raw_dump_run) {
		pthread_mutex_lock(&dump_mtx);
		pthread_cond_wait(&dump_cond, &dump_mtx);
		pthread_mutex_unlock(&dump_mtx);
		if(new_pkt_idx == pkt_to_dump_idx)
			printf("ERROR:\tdump-buffer overflow\n");
		while (new_pkt_idx != pkt_to_dump_idx) {
			dump_to_string(di, pkt_to_dump_idx);
			if (raw_file) {
				print_raw_to_file(dump_string);
			} else {
				printf("%s", dump_string);
			}
			pkt_to_dump_idx = (pkt_to_dump_idx + 1) % DUMP_BUFFER_SIZE;
		}
	}
	printf("DEBUG:\t Dump Thread has ended\n");

	return 0;
}

void dump_to_string(struct device_info *di, short idx) {
	unsigned short curr_idx;
	unsigned short curr_len;

	curr_idx = get_unsigned_short_val(&dump_buffer[idx * MAX_FRAME_SIZE + 4]);
	curr_len = get_unsigned_short_val(&dump_buffer[idx * MAX_FRAME_SIZE + 2]);

	if(dump_buffer[idx] == 0x05 && dump_size[idx] != 134) {
		printf("DUMP-WARNING:\t READ_SIZE != 134\t read_size = %d\n",
			dump_size[idx]);
	}

	if (dump_buffer[idx] == 0x05 &&
	    (last_frame_idx + 1) % 65536 != curr_idx % 65536) {
		printf("DUMP-WARNING:\t Frame Got Missing:\t last_Frame_idx: %d,"
		       "curr_Frame_idx: %d\n", last_frame_idx, curr_idx);
	}

	if(dump_buffer[idx] == 0x05 && (curr_len != 130)) {
		printf("WARNING:\t Packet with unexpected Data-Length:"
		       "\tLEN: %d\n", curr_len);
	}

	if(dump_mode[idx] == 'r') {
		sprintf(dump_string, "-> %7.4f, %7.4f,  %lld, "
			"%lld: [%03d-%03d-%d] \t",
			di->device_offset,
			di->device_scaling_factor,
			dump_curr_time[idx],
			dump_diff_time[idx],
			dump_size[idx],
			curr_len,
			curr_idx);
	} else {
		sprintf(dump_string, "<- %lld, %lld: [%03d-%03d-%d] \t",
			dump_curr_time[idx],
			dump_diff_time[idx],
			dump_size[idx],
			curr_len,
			curr_idx);
	}

	/* create blocks of measurement data */
	for (int i = 0; i < MAX_FRAME_SIZE; i++) {
		sprintf(dump_string + strlen(dump_string), "%02X",
			dump_buffer[idx * MAX_FRAME_SIZE + i]);
		if (!((i - 5) % 8) && i >= 5) {
			sprintf(dump_string + strlen(dump_string), " ");
		}
	}

	sprintf(dump_string + strlen(dump_string), "\n");
	last_frame_idx = curr_idx;
}

void raw_dump_join() {
	printf("DEBUG:\t Joining Dump Thread\n");
	pthread_join(raw_dump_thread, NULL);
	pthread_cond_destroy(&dump_cond);
	pthread_mutex_destroy(&dump_mtx);
	printf("DEBUG:\t Dump Thread Joined\n");
}
