/*
 * calculation.h
 *
 *  Created on: Aug 11, 2016
 *      Author: neez
 */

#ifndef CALCULATION_H_
#define CALCULATION_H_

#include "event_handling.h"
#include <pthread.h>

static pthread_t calculation_thread;

/*
 * init function for the calculation functionality
 */
int calculation_init(struct powquty_conf* conf);

/*
 * stop function for the calculation functionality
 */
void stop_calculation();

void join_calculation();

#endif /* CALCULATION_H_ */
