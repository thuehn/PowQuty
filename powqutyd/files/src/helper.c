/*
 * helper.c
 *
 *  Created on: Aug 17, 2016
 *      Author: neez
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "file_checks.h"
#include "helper.h"

int file_is_unchecked = 1;
long cur_offset;

void print_received_buffer(unsigned char* buf, int len) {
	int i;
	char c;

	if(len>0) {
		printf("Received[%d] ",len);
		for (i = 0; i < len; i++) {
			c = buf[i];
			printf("%x ", (unsigned char) c);
		}
		printf("\n");
	}
}

float get_float_val(unsigned char* buf) {
	float res=0.0;
	//* ((unsigned char *)&x+0 )= buf[3];
	unsigned int bin = buf[3]<<24|buf[2]<<16|buf[1]<<8|buf[0];
	// printf("Uint: \t");
	// print_received_buffer( (unsigned char *)&bin, 4);
	// unsigned char reversed[] = {buf[3], buf[2], buf[1], buf[0]};
	// printf("Reversed: \t");
	// print_received_buffer(reversed,4);
	memcpy(&res, &bin,sizeof(float));
	// printf("Result: \t");
	// print_received_buffer( (unsigned char *)&res, 4);
	return res;
}

unsigned short get_unsigned_short_val(unsigned char* buf) {
	unsigned char c0= buf[0], c1= buf[1];
	return (unsigned short) (c1<<8 | c0);
}

short get_short_val(unsigned char* buf) {
	unsigned char c0= buf[0], c1= buf[1];
	return (short) (c1<<8 | c0);
}

void print_PQ_Error(PQ_ERROR err) {
	switch (err) {
		case PQ_MEM_ERROR:
			printf("Memory allocation failed.\n");
			break;
		case PQ_INVALED_CONFIG_ERROR:
			printf("Error during creation / configuration of library\n");
			break;
		case PQ_HANDLE_ERROR:
			printf("Invalid handle given to functions. \n");
			break;
		case PQ_PROCESSING_ERROR:
			printf("Error during processing.\n");
			break;
		default:
			printf("Unknown Error: %d\n",err);
			break;
	}
}

/*
 * For a correct file write all line must have the same number of characters.
 */
void store_to_file(PQResult pqResult, struct powquty_conf *config) {
	FILE* pf;
	struct powquty_conf *conf = config;
	ssize_t char_count;
	long lower_bound, upper_bound;
	struct timespec ts_curr, ts_diff;

	if (!has_max_size(conf->powquty_path, (off_t)conf->max_log_size_kb)) {
		pf = fopen(config->powquty_path,"a");
		if (pf == NULL)
			exit(EXIT_FAILURE);
	} else {
		pf = fopen(config->powquty_path, "r+");
		char_count = get_character_count_per_line(pf);
		fseek(pf, -char_count, SEEK_END);
		lower_bound = ftell(pf);
		if (file_is_unchecked) {
			file_is_unchecked = 0;

			if (is_outdated(pf,char_count)) {
				fseek(pf, 0, SEEK_SET);
				cur_offset = 0;
			} else {
				fseek(pf, 0, SEEK_SET);
				upper_bound = ftell(pf);
				set_position(pf,upper_bound,lower_bound,
					     char_count);
				cur_offset = ftell(pf);
				if (cur_offset == lower_bound)
					file_is_unchecked = 1;
			}
		} else {
			cur_offset += (long)get_character_count_per_line(pf);
			fseek(pf, cur_offset, SEEK_SET);
			if (cur_offset == lower_bound)
				file_is_unchecked = 1;
		}
	}

	clock_gettime(CLOCK_REALTIME, &ts_curr);
	clock_gettime(CLOCK_MONOTONIC, &ts_diff);
	fprintf(pf,
			"%s,%lld.%.9ld,%lld.%.9ld,3,%010.6f,%09.6f,%09.6f,"
			"%09.6f,%09.6f,%09.6f,%09.6f,%09.6f,%09.6f\n",
			"DEV_UUID",
			(long long)ts_curr.tv_sec, ts_curr.tv_nsec,
			(long long)ts_diff.tv_sec, ts_diff.tv_nsec,
			pqResult.PowerVoltageEff_5060T,
			pqResult.PowerFrequency5060T,
			pqResult.Harmonics[0],
			pqResult.Harmonics[1],
			pqResult.Harmonics[2],
			pqResult.Harmonics[3],
			pqResult.Harmonics[4],
			pqResult.Harmonics[5],
			pqResult.Harmonics[6] );
	fclose(pf);
}
