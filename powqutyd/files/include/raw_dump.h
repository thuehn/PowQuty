/*
 * raw_dump.h
 *
 *  Created on: Feb 15, 2017
 *      Author: nadim
 */

#ifndef POWQUTYD_FILES_INCLUDE_RAW_DUMP_H_
#define POWQUTYD_FILES_INCLUDE_RAW_DUMP_H_

void dump_raw_packet(unsigned char* frame, int read_size, char mode);

int raw_dump_init(float device_offset, float device_scaling_factor);

void* raw_dump_run(void* args);

void raw_dump_stop();

void raw_dump_join();

/* set path to file for raw log
 * @param path: path for raw log file
 */
int dump_raw_to_file(const char* path);


#endif /* POWQUTYD_FILES_INCLUDE_RAW_DUMP_H_ */
