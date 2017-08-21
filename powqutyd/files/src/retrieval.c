/*
 * retrieval.c
 *
 *  Created on: Aug 10, 2016
 *      Author: neez
 */

#include "retrieval.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "calculation.h"
#include "helper.h"
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include "raw_dump.h"
#include "main.h"

static int raw_print = 0;
static int debug_flag = 0;

int serial_port_open(const char* device);
static void *reading_thread_run(void* param);

void handle_other_message(int read_size);
void handle_calib_message(int read_size);
void handle_status_message(int read_size);
void handle_data_message(int read_size);
int calibrate_device();
int start_sampling();
int stop_sampling();
void go_sleep(int us);


void set_raw_print(int i) {
	raw_print = i;
}

void set_debug(int i) {
	debug_flag = i;
}
void print_data(unsigned char* buf);

static pthread_t reading_thread;
static int retrieval_fd = -1;

static volatile float device_offset= 0.0, device_scaling_factor=0.0;

static volatile int stop_reading = 0,
		got_calib_resp = 0,
		got_status_resp = 0;

static volatile unsigned short last_frame_idx = 0;

// points to the next index where to store the next frame
// it should range from 0 - to - Block_Buffer_Size
// Block-Buffer-Size is an multiple of SAMPLES_PER_BLOCK
static volatile unsigned int stored_frame_idx = 0;

unsigned char current_frame[MAX_FRAME_SIZE];


float get_hw_offset() {return device_offset;}

float get_hw_scaling() {return device_scaling_factor;}



void handle_other_message(int read_size) {
	// currently irrelevant
	// print_received_buffer(current_frame, read_size);
}

void handle_calib_message(int read_size) {
	// parse the calibration parameters
	if( read_size > 2 && current_frame[1] == (unsigned char) 0x82)  {
		// printf("handle_calib_message: testing print float ==> %f \n", 3.1416);
		// print_received_buffer(current_frame, read_size);
		device_offset = get_float_val(current_frame+4);
		device_scaling_factor = get_float_val(current_frame+8);
		printf("Offset: %f\tScaling: %f\n",device_offset, device_scaling_factor);
		got_calib_resp = 1;
	} else {
		handle_other_message(read_size);
	}
}

void handle_status_message(int read_size) {
	got_status_resp = 1;
}

void handle_data_message(int read_size) {
	long long current_time = get_curr_time_in_milliseconds();
	// Check that Data Messages has to have a read size equal to 134 (= 1xID + 1xCC + 2xLEN + 130-Data)
	if (debug_flag) {
		if (read_size != 134) {
			printf("WARNING - READ_SIZE != 134\t read_size=%d\n", read_size);
			print_received_buffer(current_frame, read_size);
			exit(EXIT_FAILURE);
		}
	}

	unsigned short curr_idx = get_unsigned_short_val(current_frame+4);
	// do not store the same index twice
	if (last_frame_idx != curr_idx) {
		if(debug_flag) {
			if ((last_frame_idx+1)%65536 != curr_idx%65536) {
				printf("WARNING - Frame Got Missing:\t"
					"last_Frame_idx: %d,\t"
					"curr_Frame_idx: %d\n",
					last_frame_idx, curr_idx);
			}
			// Check the frame Length at Bytes [2-3]
			if(130 != get_unsigned_short_val(current_frame+2)) {
				printf("WARNING - Packet with unexpected"
					"Data-Length: \tLEN: %d\n",
					get_unsigned_short_val(current_frame+2));
			}
		}
		// new Frame
		// update last_idx
		last_frame_idx = curr_idx;

		//printf("%d ",curr_idx);
		/*
		if (raw_print) {
			printf("%lld,,", current_time);
			print_data(current_frame+6);
		}
		*/
		// Store the frame and the TS for the frame
		store_data(current_frame+6, stored_frame_idx, current_time);

		// update stored frame idx and stored TS idx (same)
		stored_frame_idx++;
		stored_frame_idx%=FRAMES_IN_BLOCK_BUFFER;

		if(stored_frame_idx%FRAMES_PER_BLOCK == 0) {
			// apply PQ-lib
			// printf("\n--->%d\n", stored_frame_idx);
			do_calculation(stored_frame_idx);
		}
	}
}

int calibrate_device() {
	int res = -1;
	unsigned char command[] = {0x02, 0x02, 0x00, 0x00};
	if (raw_print) {
		dump_raw_packet(command,5,'w');
	}
	res = write(retrieval_fd, command, 4);
	if(res <= 0) {
		return res;
	}

	// active waiting until we get a response
	// printf("Wating for device parameters .... \t");
	while(!got_calib_resp) {}
	// printf("Done!\n");

	return res;
}

int start_sampling() {
	int res = -1;
	unsigned char command[] = {0x03, 0x04, 0x01, 0x00, 0x01};
	if (raw_print) {
		dump_raw_packet(command,5,'w');
	}
	res = write(retrieval_fd, command, 5);

	while(!got_status_resp) {}
	return res;
}

int stop_sampling() {
	int res = -1;
	unsigned char command[] = {0x03, 0x04, 0x01, 0x00, 0x00};
	if (raw_print) {
		dump_raw_packet(command,5,'w');
	}
	res = write(retrieval_fd, command, 5);
	return res;
}

void print_data(unsigned char* buf){
	int i = 0;
	while(i<128) {
		printf("%d ", get_short_val(buf+i) );
		i+=2;
	}
	printf("\n");
}

int serial_port_open(const char* device) {
	int bits;
	struct termios config;
	memset(&config, 0, sizeof(config));

	// int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	int fd = open(device, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		printf("open(%s): %s", device, strerror(errno));
		return -1;
	}

	// set RTS
	ioctl(fd, TIOCMGET, &bits);
	bits |= TIOCM_RTS;
	ioctl(fd, TIOCMSET, &bits);

	tcgetattr( fd, &config ) ;

	// set 8-N-1
	config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	config.c_oflag &= ~OPOST;
	config.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	config.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB);
	config.c_cflag |= CS8;

	// set speed to 115200 baud
	cfsetispeed( &config, B115200);
	cfsetospeed( &config, B115200);

	tcsetattr(fd, TCSANOW, &config);
	return fd;
}

int retrieval_init(const char* tty_device) {
	int res = 0;

	// open fd
	retrieval_fd = serial_port_open(tty_device);
	if(retrieval_fd<0) {
		printf("\ncannot open %s\n",tty_device);
		return retrieval_fd;
	}

	// initialize Buffers, ring_buffer etc.
	memset(current_frame,0,MAX_FRAME_SIZE);

	if (raw_print) {
		if(!raw_dump_init()) {
			printf("DEBUG:\tDump Thread Created\n");
			//stop_sampling();
		}
	}
	// start reading thread
	printf("DEBUG:\tCreating Retrieval Thread\n");
	res = pthread_create(&reading_thread,NULL, reading_thread_run,NULL);

	// send command get Hardware parameters
	// Blocking call until we get resp see calibrate_device()
	if (calibrate_device() <= 0) {
		// failed to calibrate device
		res = -1;
	}

	if (start_sampling() <= 0){
		res = -1;
	}

	return res;
}

void stop_retrieval() {
	printf("DEBUG:\tStopping Retrieval Thread [raw=%d]\n",raw_print);
	stop_sampling();
	stop_reading = 1;
	if (raw_print) {
		raw_dump_stop();
	}
}

void join_retrieval() {
	printf("DEBUG:\tJoining Retrieval Thread \n");
	if (raw_print) {
		raw_dump_join();
	}
	pthread_join(reading_thread, NULL);
	printf("DEBUG:\tRetrieval Thread joined \n");
}

static void *reading_thread_run(void* param) {
	printf("DEBUG:\tRetrieval Thread has started\n");
	// int read_size = 0, poll_time_out_ms = 10, offset = 0, done_reading = 0;
	int read_size = 0, offset = 0, done_reading = 0;
	// struct pollfd fd[1];
	// fd[0].fd = retrieval_fd;
	// fd[0].events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;

	while (!stop_reading) {
		// poll(fd,1,poll_time_out_ms);
		offset = 0;
		read_size = 0;
		done_reading = 0;
		do {
			// poll(fd,1,poll_time_out_ms);
			offset = read(retrieval_fd, current_frame+read_size,
					MAX_FRAME_SIZE-read_size);
			if(offset < 0) {
				printf("\n\n\nERROR:\t error while reading"
					"\toffset = %d\t errno; %s \n\n\n\n",
					offset,strerror(errno));
				go_sleep(1000);
				continue;
				//exit(EXIT_FAILURE);
			} else if (offset == 0) {
				printf("\n\n\nERROR:\t error while reading\t"
					"offset = %d\t errno; %s \n\n\n\n",
					offset,strerror(errno));
				stop_powqutyd();
				//go_sleep(1000);
				break;
			} else {
				read_size += offset;
				if (current_frame[0] != 0x05) {
					done_reading = 1;
				}
			}
		}while(read_size < MAX_FRAME_SIZE && !done_reading);
		// read_size = read(retrieval_fd, current_frame, MAX_FRAME_SIZE);
		if(raw_print) {
			dump_raw_packet(current_frame,read_size,'r');
		}
		if(read_size > 0) {
			switch (current_frame[0]) {
			// Calibraton Message
			case 0x02:
			{
				handle_calib_message(read_size);
			}
			break;
			// Mode Message
			case 0x03:
			{
				handle_status_message(read_size);
			}
			break;
			// Data Message
			case 0x05:
			{
				handle_data_message(read_size);
			}
			break;

			// Other Message
			default:
			{
				handle_other_message(read_size);
			}
			break;
			}
		}
		memset(current_frame,0,MAX_FRAME_SIZE);
	}
	printf("DEBUG:\tRetrieval Thread has ended\n");
	return NULL;
}

void go_sleep(int us) {
	struct timeval tv;
	tv.tv_sec=0;
	tv.tv_usec= us;
	select(0, NULL, NULL, NULL, &tv);
}
