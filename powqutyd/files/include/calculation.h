/*
 * calculation.h
 *
 *  Created on: Aug 11, 2016
 *      Author: neez
 */

#ifndef CALCULATION_H_
#define CALCULATION_H_

/*
 * init function for the calculation functionality
 */
int calculation_init();

/*
 * stop function for the calculation functionality
 */
void stop_calculation();

void do_calculation(unsigned int stored_frame_idx);

void store_data(unsigned char * buf, unsigned int stored_frame_idx, long long ts);

#endif /* CALCULATION_H_ */
