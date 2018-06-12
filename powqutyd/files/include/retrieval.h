/*
 * retrieval.h
 *
 *  Created on: Aug 10, 2016
 *      Author: neez
 */

#ifndef RETRIEVAL_H_
#define RETRIEVAL_H_

#define MAX_FRAME_SIZE			134
#define SAMPLES_PER_FRAME		64
#define SAMPLES_PER_BLOCK		2048
#define FRAMES_PER_BLOCK		(SAMPLES_PER_BLOCK / SAMPLES_PER_FRAME)	// 32
#define NUMBER_OF_BLOCKS_IN_BUFFER	50
#define BLOCK_BUFFER_SIZE		NUMBER_OF_BLOCKS_IN_BUFFER * SAMPLES_PER_BLOCK // 5*2048 = 10240
#define TS_BUFFER_SIZE			NUMBER_OF_BLOCKS_IN_BUFFER * FRAMES_PER_BLOCK // 5*32 = 160
#define FRAMES_IN_BLOCK_BUFFER		NUMBER_OF_BLOCKS_IN_BUFFER * FRAMES_PER_BLOCK // 5*32 = 160

unsigned char get_device_information;

void store_data(unsigned char * buf, unsigned int stored_frame_idx, long long ts);

void do_calculation(unsigned int stored_frame_idx);

/*
 * init function for the retrieval functionality
 */
int retrieval_init(const char* tty_device);
void set_raw_print(int i);
void set_debug(int i);

/*
 * stop function for the retrieval functionality
 */
void stop_retrieval();

void join_retrieval();

float get_hw_offset();

float get_hw_scaling();

#endif /* RETRIEVAL_H_ */
