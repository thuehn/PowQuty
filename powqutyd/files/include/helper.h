/*
 * helper.h
 *
 *  Created on: Aug 17, 2016
 *      Author: neez
 */

#ifndef HELPER_H_
#define HELPER_H_

long long get_curr_time_in_milliseconds();

short get_short_val(unsigned char* buf);

void print_received_buffer(unsigned char* buf, int len);

float get_float_val(unsigned char* buf);

unsigned short get_unsigned_short_val(unsigned char* buf);



#endif /* HELPER_H_ */
