/*
 *  file_checks.c
 *
 *   Create on May 02, 2018
 *   	Author: ikstream
 */

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file_checks.h"

#define KB_TO_BYTE 1024
#define TIME_STAMP_POSITION 2

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
	free(last_line);

	if (last_time > first_time)
		return 1;

	return 0;
}

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

