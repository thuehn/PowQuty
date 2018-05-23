#ifndef _FILE_CHECKS_H_
#define _FILE_CHECKS_H_

#include <stdio.h>

#define MAX_PATH_LENGTH 512

/*
 * checks if the log file can be rewritten from start, or has to be resumed
 * @file: file to check
 * @char_count: number of chars in a line
 * return 0 if file write has to be resumed, 1 if the first entry is the oldest
 */
int is_outdated(FILE *file, ssize_t char_count);

/*
 * check if a file is above a given limit
 * @file: file to check
 * @max_size: maximal size of file in kB
 * return: returns 1 if file is above the limit, else 0
 */
int has_max_size(char *powquty_path, off_t max_size);

/*
 * get the number of characters in a regular line in file
 * this assumes, that all lines have the same character count
 * @file: file to check for line length
 * return: returns character count of first line
 */
ssize_t get_character_count_per_line(FILE *file);

/*
 * set the position to resume write operations after powqutyd stopped
 * uses logarithmic search to get the latest timestamp
 * @file: file to write to
 * @u_bound upper boundary for interval
 * @l_bound lower boundary for latest timestamp search
 * @char_count: length of line to calculate offset
 */
void set_position(FILE *file, long u_bound, long l_bound, ssize_t char_count);

#endif
