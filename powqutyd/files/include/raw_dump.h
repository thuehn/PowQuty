/*
 * raw_dump.h
 *
 *  Created on: Feb 15, 2017
 *      Author: nadim
 */

#ifndef POWQUTYD_FILES_INCLUDE_RAW_DUMP_H_
#define POWQUTYD_FILES_INCLUDE_RAW_DUMP_H_

void dump_raw_packet(unsigned char* frame, int read_size, char mode);

int raw_dump_init();

void* raw_dump_run(void* args);

void raw_dump_stop();

void raw_dump_join();


#endif /* POWQUTYD_FILES_INCLUDE_RAW_DUMP_H_ */
