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

static int raw_print = 0;

int serial_port_open(const char* device);
static void *reading_thread_run(void* param);

void handle_other_message(int read_size);
void handle_calib_message(int read_size);
void handle_status_message(int read_size);
void handle_data_message(int read_size);
int calibrate_device();
int start_sampling();
int stop_sampling();


void set_raw_print(int i) {
	raw_print = i;
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

unsigned char *current_frame;


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

	unsigned short curr_idx = get_unsigned_short_val(current_frame+4);
	// do not store the same index twice
	if (last_frame_idx != curr_idx) {
		// new Frame
		// update last_idx
		last_frame_idx = curr_idx;

		//printf("%d ",curr_idx);
		if (raw_print) {
			printf("%lld,,", current_time);
			print_data(current_frame+6);
		}
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
	res = write(retrieval_fd, command, 5);

	while(!got_status_resp) {}
	return res;
}

int stop_sampling() {
	int res = -1;
	unsigned char command[] = {0x03, 0x04, 0x01, 0x00, 0x00};
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

	int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
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
	current_frame = malloc(MAX_FRAME_SIZE);
	memset(current_frame,0,MAX_FRAME_SIZE);

	// start reading thread
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
	stop_sampling();
	stop_reading = 1;
	pthread_join(reading_thread, NULL);
	free(current_frame);
}

static void *reading_thread_run(void* param) {
	int read_size = 0;
	while (!stop_reading) {
		read_size = read(retrieval_fd, current_frame, MAX_FRAME_SIZE);
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
	return NULL;
}

