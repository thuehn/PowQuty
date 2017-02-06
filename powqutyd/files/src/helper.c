/*
 * helper.c
 *
 *  Created on: Aug 17, 2016
 *      Author: neez
 */

#include "helper.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#define MB_TO_BYTE 1048576
#define MAX_FILE_SIZE 4096
#define TIME_STAMP 2

off_t max_filesize = MAX_FILE_SIZE;
fpos_t first_valid_line;
int is_unchecked = 1;

void print_received_buffer(unsigned char* buf, int len) {
	if(len>0) {
		int i=0;
		char c;
		printf("Received[%d] ",len);
		for (i=0;i<len;i++) {
			c= buf[i];
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

long long get_curr_time_in_milliseconds() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (long long) ( (tv.tv_sec * 1000) + (int)tv.tv_usec/1000 );
}

int get_curr_time_in_seconds() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (int) (tv.tv_sec);
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
 * returns content from comma separated line
 * @line: comma separated line, to parse
 * @entry: position in line
 * return: token if found, else NULL
 */
char * get_entry(char* line, int entry) {
	char* token;
	for (token = strtok(line, ","); token && *token;
	     token = strtok(NULL, ",\n")) {
		if (!--entry)
			return token;
	}
	return NULL;
}

/*
 * get the last line of a file
 * @file: file to get line from
 * @line_length: offset to get the start of line
 * return: complete last line of file
 */
char * get_last_line(FILE *file, ssize_t line_length) {
	char *line = NULL;
	ssize_t read;
	size_t len = 0;

	fseek(file, -line_length,SEEK_END);
	read = getline(&line, &len, file);
	if (read == -1) {
		printf("Could not get last line\n");
		exit(EXIT_FAILURE);
	}
	return line;
}

/*
 * get the number of characters in a regular line in file
 * @file: file to check for line length
 * return: returns regular line length
 */
ssize_t get_regular_line_length(FILE *file) {
	char *line = NULL;
	char *next_line = NULL;
	size_t len = 0;
	size_t next_len = 0;
	ssize_t reg_read, next_read = 0;

	reg_read = getline(&line, &len, file);
	if (reg_read == -1) {
		printf("Error in line read: Could not get length of first line\n");
		exit(EXIT_FAILURE);
	}
	fseek(file, reg_read, SEEK_SET);
	next_read = getline(&next_line, &next_len, file);

	if (next_read == reg_read) {
		fseek(file, -reg_read, SEEK_CUR);
		fgetpos(file, &first_valid_line);
		return reg_read;
	}

	if (next_read != -1) {
		reg_read = next_read;
	} else {
		printf("Error in line read: Could not get length of line\n");
		exit(EXIT_FAILURE);
	}

	fgetpos(file, &first_valid_line);
	return reg_read;
}

/*
 * checks if the log file can be rewritten from start, or has to be resumed
 * @file: file to check
 * @line_length: number of chars in regular line
 * return 0 if file write has to be resumed, 1 if the first entry is the oldest
 */
int is_outdated(FILE *file, ssize_t line_length) {
	int first_time, last_time;
	char *line =NULL;
	char *last_line = malloc((sizeof(char) * line_length) + 1);
	size_t len = 0;

	fsetpos(file, &first_valid_line);
	getline(&line, &len, file);
	first_time = atoi(get_entry(line, TIME_STAMP));

	memcpy(last_line,get_last_line(file,line_length), line_length);
	last_line[line_length] = '\0';
	last_time = atoi(get_entry(last_line, TIME_STAMP));

	if (last_time > first_time)
		return 1;

	return 0;
}

/*
 * check if a file is above a given limit
 * @file: file to check
 * @max_size: maximal size of file in MB
 * return: returns 1 if file is above the limit, else 0
 */
int has_max_size(char *powquty_path, off_t max_size) {
	struct stat st;
	off_t filesize;

	max_size *= MB_TO_BYTE;

	if (stat(powquty_path, &st) == 0) {
		filesize = st.st_size;
	} else {
		printf("Could not get filesize\n");
		exit(2);
	}

	if (filesize >= max_size)
		return 1;

	return 0;
}

void store_to_file(PQResult pqResult, char *powquty_path) {
	FILE* pf;
	ssize_t line_length;

	if (!has_max_size(powquty_path, max_filesize)) {
		pf = fopen(powquty_path,"a");
		if (pf == NULL)
			exit(EXIT_FAILURE);
	} else {
		pf = fopen(powquty_path, "r+");
		if (pf == NULL)
			exit(EXIT_FAILURE);
		line_length = get_regular_line_length(pf)
		if (is_unchecked) {
			is_unchecked = 0;
			if (is_outdated(pf,line_length)) {
				fseek(file, 0 ,SEEK_SET);
			} else {
				//TODO: get position to resume file write
			}
		} else {
			fsetpos();
		}
	}
	long long ts = get_curr_time_in_milliseconds();
	int ts_sec = get_curr_time_in_seconds();
	fprintf(pf,
			"%s,%d,%lld,3,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n",
			"DEV_UUID",
			ts_sec,
			ts,
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
