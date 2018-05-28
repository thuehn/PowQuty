/*
 * file_handling.c
 *
 *  created on Aug 13, 2017
 *  	Author: ikstream
 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "event_handling.h"
#include "file_handling.h"
#include "helper.h"
#include "main.h"
#include "mqtt.h"
#include "retrieval.h"

#define FILE_READ_OFFSET 0.f
#define FILE_READ_SCALE 1.f
#define SAMPLE_FREQUENCY 10240

short *block_buffer;
static long long *timestamp_buffer;
float *fr_in;
struct powquty_conf* config;

static pthread_t file_read_thread;
void *file_read_thread_run(void *param);
static volatile unsigned int stop_file_read_run = 0;

PQConfig frpqConfig;
PQ_ERROR frerr = PQ_NO_ERROR;
pPQInstance frpPQInst = NULL;
PQInfo frpqInfo;
PQResult frpqResult;

static char *input_file = NULL;
long long timestamp = 0;
unsigned int counter = -1;

int read_data_from_file(const char *path)
{
	if (path == NULL)
		return EXIT_FAILURE;

	if (strlen(path) >= MAX_PATH_LENGTH)
		return EXIT_FAILURE;

	input_file = malloc(sizeof(char) * MAX_PATH_LENGTH);
	if (input_file == NULL) {
		printf("ERROR:\t\t error allocating memory in %s\n", __func__);
		return EXIT_FAILURE;
	} else {
		strcpy(input_file, path);
		return EXIT_SUCCESS;
	}
}

int get_input_file_state()
{
	if (input_file)
		return 1;
	else
		return 0;
}

void join_file_read()
{
	printf("DEBUG:\tJoining file read thread\n");
	pthread_join(file_read_thread, NULL);
	printf("DEBUG:\tFile read thread joined\n");
}

void stop_file_read()
{
	printf("DEBUG:\tStopping file read thread\n");
	if (input_file) {
		free(input_file);
	}
	stop_file_read_run = 1;
	destroyPowerQuality(&frpPQInst);
}

/*
 * set timestamp buffer to new time and return index
 * @return: index of time stamp buffer
 */
int set_time_stamp()
{
	int i;

	for (i = 0; i < FRAMES_PER_BLOCK; i++) {
		counter++;
		if (!(counter % 4)) {
			timestamp += 7;
			timestamp_buffer[counter] = timestamp;
			continue;
		}

		timestamp += 6;
		timestamp_buffer[counter] = timestamp;
		if (((counter + 1) % 160) == 0) {
			printf("max size\n");
			counter = -1;
		}
	}
	return counter;
}


int file_read_init(struct powquty_conf *conf)
{
	int res;

	block_buffer = calloc(sizeof(short),BLOCK_BUFFER_SIZE);
	timestamp_buffer = calloc(sizeof(long long), TS_BUFFER_SIZE);
	fr_in = calloc(sizeof(float), SAMPLES_PER_BLOCK);
	config = conf;
	memset(block_buffer, 0, BLOCK_BUFFER_SIZE * sizeof(short));
	memset(timestamp_buffer, 0, TS_BUFFER_SIZE * sizeof(long long));
	memset(fr_in, 0, SAMPLES_PER_BLOCK * sizeof(float));
	frpqConfig.sampleRate = SAMPLE_FREQUENCY;
	frpqConfig.HW_offset = FILE_READ_OFFSET;
	frpqConfig.HW_scale = FILE_READ_SCALE;

	frerr = createPowerQuality(&frpqConfig, &frpPQInst, &frpqInfo);
	printf("frerr: %d\n", frerr);
	if(frerr == PQ_NO_ERROR) {
		// start file_read thread
		res = pthread_create(&file_read_thread, NULL,
				     file_read_thread_run, NULL);
	} else {
		// TODO see what happened
		printf("ERROR:\t\terror creating PQ_Instance, errno: %d\n", frerr);
		return -1;
	}
	return res;
}

void *file_read_thread_run(void *param)
{
	FILE *file = fopen(input_file, "r");
	int offset = 0;
	int i = 0;

	printf("DEBUG:\tFile read thread has started\n");

	if (file == NULL) {
		printf("ERROR:\tCould not open file %s: %s\n", input_file,
			strerror(errno));
		return NULL;
	}

	while (!stop_file_read_run) {
		sleep(1);
		if (!feof(file)) {
			fread(fr_in, sizeof(float), MAX_FRAMESIZE, file);
		} else {
			printf("DEBUG:\tReached end of file\n");
			break;
		}

		if (ferror(file)) {
			printf("ERROR:\tAn error occurred during file "
				"read\n");
			break;
		}

		set_time_stamp();

		frerr = applyPowerQuality(
				frpPQInst,
				fr_in,
				&frpqResult,
				NULL,
				timestamp_buffer + offset,
				FRAMES_PER_BLOCK);

		offset = i * 32;
		if (i == 4)
			i = 0;
		else
			i++;

		/* exit processing on error */
		if(frerr != PQ_NO_ERROR) {
			printf("TODO:\t\tError applying PQ-Lib\n\t\t\t");
			print_PQ_Error(frerr);
			stop_powqutyd_file_read();
			break;
		}

		/* EN50160 event detected */
		if (frpqResult.nmbPqEvents > 0)
			handle_event(frpqResult, config);

		if(frpqResult.HarmonicsExist) {
			store_to_file(frpqResult, config);
#ifdef MQTT
			publish_measurements(frpqResult);
#endif
		}
	}
	fclose(file);
	printf("DEBUG:\tFile read thread has ended\n");

	stop_powqutyd_file_read();

	return NULL;
}

