/*
 * vserial_test.c
 *
 *  Created on: Jun 30, 2016
 *      Author: neez
 */


#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "config.h"
#include <signal.h>
#include <sys/time.h>


static volatile int stop_main = 0;

void handle_signal()
{
	stop_main = 1;
}

int serial_port_open(const char* device)
{
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

int start_sampling(int fd) {
	int res = -1;
	unsigned char command[] = {0x03, 0x04, 0x01, 0x00, 0x01};
	res = write(fd, command, 5);
	return res;
}

int stop_sampling(int fd) {
	int res = -1;
	unsigned char command[] = {0x03, 0x04, 0x01, 0x00, 0x00};
	res = write(fd, command, 5);
	return res;
}

long long get_curr_time_in_milliseconds() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (long long) ( (tv.tv_sec * 1000) + (int)tv.tv_usec/1000 );
}

short get_signed_short(unsigned char c1, unsigned char c2) {
	return (short) (c2<<8 | c1);
}

unsigned short get_unsigned_short(unsigned char c1, unsigned char c2) {
	return (unsigned short) (c2<<8 | c1);
}

unsigned short get_unsigned_short_ptr(unsigned char* buf) {
	unsigned char c1 = buf[0], c2 = buf[1];
	return (unsigned short) (c2<<8 | c1);
}

void print_data(unsigned char* buf, int len, long long curr_time) {
	if(len > 0) {
		int i = 0;
		unsigned short idx = get_unsigned_short_ptr (buf+4);
		unsigned short actual_length = get_unsigned_short(buf[2],buf[3]);
		// unsigned char c;
		printf("Received[%d - %lld] \n",idx, curr_time);
		i=6;
		/*
		 * actual_length := is length of Data
		 * actual_length + 4 := is the idx of the first byte after the Data is over
		 * this should never be called!
		 */
		/*
		while (i<actual_length + 4) {
			printf("%d ", get_signed_short(buf[i], buf[i+1]));
			i+=2;
		}
		printf("\n");
		*/
	}
}

void print_received_buffer(unsigned char* buf, int len) {
	if(len>0) {
		int i=0;
		char c;
		printf("Received[%d] ",len);
		for (i=0;i<len;i++) {
			c= buf[i];
			printf("%x ", (unsigned char) c);
		}
		printf("\n");
	}
}


int main (int argc, char *argv[]) {
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);

	int fd= 0;
	fd = serial_port_open(device_tty);
	if (fd<0) {
		printf("cannot open %s\n",device_tty);
		return 0;
	} else {
		printf("%s is open with the file descriptor = %d\n",device_tty,fd);
	}

	unsigned char * buf = malloc(160);
	memset(buf, 0, 160);
	size_t len = 138, r=0;
	while (start_sampling(fd)<0) {
		printf("?");
	}
	volatile int s_idx=0;
	long long last_time, delta_time, curr_time;
	while (!stop_main) {
		r = read(fd, buf, len);
		if(r>0) {
			if(buf[0] == (unsigned char)0x5) {
				// do not print the same index twice
				if (s_idx != get_unsigned_short(buf[4],buf[5])) {
					// update curr_time on new Frame
					curr_time = get_curr_time_in_milliseconds();
					// update the last printed index
					s_idx = get_unsigned_short(buf[4],buf[5]);
					print_data(buf,r, curr_time);
					delta_time = curr_time - last_time;
					last_time = curr_time;
				} else {
					// TODO warn that a frame has the last frame's index
					// printf("warning: last_idx: %d \tcurr_idx: %d\n",s_idx, get_unsigned_short(buf[4],buf[5]));
				}
			} else {
				print_received_buffer(buf,r);
			}
		}
		//sleep(1);
	}

	while(stop_sampling(fd)<0) {
		printf("!");
	}

	if(fd>=0) {
		printf("closing the file descriptor for %s\n",device_tty);
		close(fd);
	}
	free(buf);
	return 0;
}
