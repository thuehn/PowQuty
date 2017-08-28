/*
 * file_handling.h
 *
 *  Created on: Aug 14, 2017
 *  	Author: ikstream
 */
#ifndef FILE_HANDLING_H_
#define FILE_HANDLING_H_

#include "event_handling.h"

/*
 * use file instead of usb oscilloscope for data input
 * @param path: file to use as input
 * returns: 1 on failure, 0 else
 */
int set_file_read(const char* path);

/*
 * check if a file is used instead of a usb=oscilloscope for data input
 * returns: 1 if file is used, else 0
 */
int get_input_file_state();

/*
 * init function for the input file read option
 * @param conf: powquty configuration struct
 * returns: 0 on success else 1
 */
int file_read_init(struct powquty_conf *conf);

/*
 * stop file read thread
 */
void stop_file_read();

/*
 * join powquty file read thread
 */
void join_file_read();

#endif /* FILE_HANDLING_H_ */

