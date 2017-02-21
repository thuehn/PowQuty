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
#include <unistd.h>
#include "config.h"

#define KB_TO_BYTE 1024
#define TIME_STAMP_POSITION 2

int file_is_unchecked = 1;
long cur_offset;

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

long get_curr_time_in_seconds() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
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
 * get entry in csv line
 * @line: comma separated line, to parse
 * @entry: position in line
 * return: token if found, else NULL
 */
char * get_entry_from_line_position(char* line, int entry) {
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
 * @char_count: offset(number of characters) to get the start of line
 * return: complete last line of file
 */
char * get_last_line(FILE *file, ssize_t char_count) {
	char *line = NULL;
	ssize_t read;
	size_t len = 0;

	fseek(file, -char_count,SEEK_END);
	read = getline(&line, &len, file);
	if (read == -1) {
		printf("Could not get last line\n");
		exit(EXIT_FAILURE);
	}
	return line;
}

/*
 * get the number of characters in a regular line in file
 * this assumes, that all lines have the same character count
 * @file: file to check for line length
 * return: returns character count of first line
 */
ssize_t get_character_count_per_line(FILE *file) {
	char *line = NULL;
	size_t len = 0;
	ssize_t char_count;

	fseek(file, 0, SEEK_SET);
	char_count = getline(&line, &len, file);
	if (char_count == -1) {
		printf("Error in line read: Could not get number of characters"
			" in first line\n");
		exit(EXIT_FAILURE);
	}
	fseek(file, 0, SEEK_SET);

	return char_count;
}

/*
 * checks if the log file can be rewritten from start, or has to be resumed
 * @file: file to check
 * @char_count: number of chars in a line
 * return 0 if file write has to be resumed, 1 if the first entry is the oldest
 */
int is_outdated(FILE *file, ssize_t char_count) {
	long first_time, last_time;
	char *line =NULL;
	char *last_line = malloc((sizeof(char) * char_count) + 1);
	size_t len = 0;

	fseek(file, 0, SEEK_SET);
	getline(&line, &len, file);
	first_time = atol(get_entry_from_line_position(line,
						       TIME_STAMP_POSITION));

	memcpy(last_line,get_last_line(file,char_count), char_count);
	last_line[char_count] = '\0';
	last_time = atol(get_entry_from_line_position(last_line,
						      TIME_STAMP_POSITION));

	if (last_time > first_time)
		return 1;

	return 0;
}

/*
 * check if a file is above a given limit
 * @file: file to check
 * @max_size: maximal size of file in kB
 * return: returns 1 if file is above the limit, else 0
 */
int has_max_size(char *powquty_path, off_t max_size) {
	struct stat st;
	off_t filesize;

	max_size *= KB_TO_BYTE;

	if (access(powquty_path, F_OK ))
		return 0;

	if (stat(powquty_path, &st) == 0) {
		filesize = st.st_size;
	} else {
		printf("Could not get filesize\n");
		exit(2);
	}

	if (filesize > max_size)
		return 1;

	return 0;
}

/*
 * calculate real number of a line(starting with 1)
 * @offset: offset of line in file
 * @char_count number of characters in a line
 * return the line number
 */
long get_line_number(long offset, ssize_t char_count) {
	long line_number = (offset / char_count) + 1;
	return line_number;
}

/*
 * get an Entry(timestamp) from a line
 * the position of the line has to be set before calling this function
 * @file file to read from
 * return timestam as integer
 */
int get_line_entry(FILE *file) {
	char *line = NULL;
	size_t len = 0;
	long val;

	getline(&line, &len, file);
	val = atol(get_entry_from_line_position(line, TIME_STAMP_POSITION));
	return val;
}

/*
 * set the position to resume write operations after powqutyd stopped
 * uses logarithmic search to get the latest timestamp
 * @file: file to write to
 * @u_bound upper boundary for interval
 * @l_bound lower boundary for latest timestamp search
 * @char_count: length of line to calculate offset
 */
void set_position(FILE *file, long u_bound, long l_bound, ssize_t char_count) {
	long u_offset, m_offset, l_offset;
	long u_line_number, m_line_number, l_line_number;
	int u_val, m_val, l_val;

	fseek(file, u_bound, SEEK_SET);
	u_offset = ftell(file);
	u_val = get_line_entry(file);
	u_line_number = get_line_number(u_offset, char_count);

	fseek(file, l_bound, SEEK_SET);
	l_offset = ftell(file);
	l_val = get_line_entry(file);
	l_line_number = get_line_number(l_offset, char_count);

	m_line_number = ((l_line_number + u_line_number) / 2);
	m_offset = ((m_line_number * char_count) - char_count);
	fseek(file, m_offset, SEEK_SET);
	m_val = get_line_entry(file);

	if ((m_val > l_val) && (m_val > u_val))
		set_position(file, m_offset, l_offset, char_count);
	if ((m_val < l_val) && (m_val < u_val))
		set_position(file, u_offset, m_offset, char_count);
	if ((m_val == u_val) || (m_val == l_val))
		fseek(file, m_offset + char_count, SEEK_SET);
}

/*
 * For a correct file write all line must have the same number of characters.
 */
void store_to_file(PQResult pqResult, struct powquty_conf *config) {
	FILE* pf;
	struct powquty_conf *conf = config;
	ssize_t char_count;
	long lower_bound, upper_bound;

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
	long long ts = get_curr_time_in_milliseconds();
	long ts_sec = get_curr_time_in_seconds();
	fprintf(pf,
			"%s,%ld,%lld,3,%010.6f,%09.6f,%09.6f,%09.6f,%09.6f,%09.6f,"
			"%09.6f,%09.6f,%09.6f\n",
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
