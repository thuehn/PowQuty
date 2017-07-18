/*
 * calculation.h
 *
 *  Created on: Aug 11, 2016
 *      Author: neez
 */

#ifndef CALCULATION_H_
#define CALCULATION_H_

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
 * init function for the calculation functionality
 */
int calculation_init(struct powquty_conf* conf);

/*
 * stop function for the calculation functionality
 */
void stop_calculation();

void join_calculation();

void do_calculation(unsigned int stored_frame_idx);

void store_data(unsigned char * buf, unsigned int stored_frame_idx, long long ts);

#endif /* CALCULATION_H_ */
