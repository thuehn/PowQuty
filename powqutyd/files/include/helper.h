/*
 * helper.h
 *
 *  Created on: Aug 17, 2016
 *      Author: neez
 */

#ifndef HELPER_H_
#define HELPER_H_

#include "PQ_App.h"
#include "uci_config.h"

long long get_curr_time_in_milliseconds();

long long get_curr_time_in_microseconds();

long get_curr_time_in_seconds();

short get_short_val(unsigned char* buf);

void print_received_buffer(unsigned char* buf, int len);

float get_float_val(unsigned char* buf);

unsigned short get_unsigned_short_val(unsigned char* buf);

void print_PQ_Error(PQ_ERROR err);

void store_to_file(PQResult pqResult, struct powquty_conf *config);

#endif /* HELPER_H_ */
