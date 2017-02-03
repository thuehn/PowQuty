/*
 * helper.h
 *
 *  Created on: Aug 17, 2016
 *      Author: neez
 */

#ifndef HELPER_H_
#define HELPER_H_

#include "PQ_App.h"

long long get_curr_time_in_milliseconds();

int get_curr_time_in_seconds();

short get_short_val(unsigned char* buf);

void print_received_buffer(unsigned char* buf, int len);

float get_float_val(unsigned char* buf);

unsigned short get_unsigned_short_val(unsigned char* buf);

void print_PQ_Error(PQ_ERROR err);

void store_to_file(PQResult pqResult, char *powquty_path);

#endif /* HELPER_H_ */
