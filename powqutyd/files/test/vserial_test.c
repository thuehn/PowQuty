/*
 * vserial_test.c
 *
 *  Created on: Jun 30, 2016
 *      Author: neez
 */


#include <stdio.h>
#include <string.h>
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


short get_signed_short(unsigned char c1, unsigned char c2) {
	return (short) (c2<<8 | c1);
}

unsigned short get_unsigned_short(unsigned char c1, unsigned char c2) {
	return (unsigned short) (c2<<8 | c1);
}

void print_data(unsigned char* buf, int len, struct timeval* tv) {
	if(len > 0) {
		gettimeofday(tv, NULL);
		int i = 0;
		unsigned short idx = get_unsigned_short (buf[4], buf[5]);
		// short sample;
		// unsigned char c;
		printf("Received[%d - %ld.%ld] \n",idx, tv->tv_sec, tv->tv_usec);
		i=6;
		while (i<len) {
			printf("%d ", get_signed_short(buf[i], buf[i+1]));
			i+=2;
		}
		printf("\n");
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

int serial_port_open(const char* device);


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
	size_t len = 138, r=0, w=0;
	unsigned char command [5];
	command[0] = (char)0x03;
	command[1] = (char)0x04;
	command[2] = (char)0x01;
	command[3] = (char)0x0;
	command[4] = (char)0x01;
	do {
		w = write(fd, command, 5);
		printf("write returned %d\n",(int)w);
		r = read(fd, buf, len);
	} while (r<=0);
	int s_idx=0;
	struct timeval tv;
	long long t_sec, t_usec, r_sec, r_usec;
	while (!stop_main) {
		r = read(fd, buf, len);
		if(r>0) {

			if(buf[0] == (unsigned char)0x5) {
				if (s_idx != get_unsigned_short(buf[4],buf[5])) {
					s_idx = get_unsigned_short(buf[4],buf[5]);
					print_data(buf,len, &tv);
					r_sec = tv.tv_sec - t_sec;
					r_usec = tv.tv_usec - t_usec;
					t_sec = tv.tv_sec;
					t_usec = tv.tv_usec;
					printf ("%lld.%lld\n", r_sec, r_usec);
				}
			} else {
				print_received_buffer(buf,r);
			}
		}
		//sleep(1);
	}

	if(fd>=0) {
		printf("closing the file descriptor for %s\n",device_tty);
		close(fd);
	}
	free(buf);
	return 0;
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

