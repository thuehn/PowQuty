/*
 * retrieval.h
 *
 *  Created on: Aug 10, 2016
 *      Author: neez
 */

#ifndef RETRIEVAL_H_
#define RETRIEVAL_H_


/*
 * init function for the retrieval functionality
 */
int retrieval_init(const char* tty_device);

/*
 * stop function for the retrieval functionality
 */
void stop_retrieval();

float get_hw_offset();

float get_hw_scaling();

#endif /* RETRIEVAL_H_ */
