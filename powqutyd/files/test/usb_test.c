/*
 * usb_test.c
 *
 *  Created on: Jul 31, 2016
 *      Author: neez
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "libusb-1.0/libusb.h"

static volatile int stop_main = 0;

void handle_signal()
{
	stop_main = 1;
}

void print_received_buffer(unsigned char* buf, int len) {
	int i=0;
	char c;
	printf("Received[%d] ",len);
	for (i=0;i<len;i++) {
		c= buf[i];
		printf("%x ",(uint8_t)c);
	}
	printf("\n");
}
#if 1
#define VENDOR_ID		0x1afe		// A.Eberle
#define PRODUCT_ID		0x0009		// Wesense

#define ACM_CTRL_DTR	0x01
#define ACM_CTRL_RTS 	0x02

static struct libusb_device_handle *devh = NULL;

/*
 * The Endpoint address are hard coded. You should use lsusb -v to find
 * the values corresponding to your device.
 */
static int ep_in_addr  = 0x81;
static int ep_out_addr = 0x02;

void write_chars(unsigned char* c)
{
	/* To send a chars to the device simply initiate a bulk_transfer to the
	 * Endpoint with address ep_out_addr.
	 */
	int actual_length;
	if (libusb_bulk_transfer(devh, ep_out_addr, c, 3,
			&actual_length, 0) < 0) {
		fprintf(stderr, "Error while sending chars\n");
	}
}
void write_char(unsigned char c)
{
	/* To send a char to the device simply initiate a bulk_transfer to the
	 * Endpoint with address ep_out_addr.
	 */
	int actual_length;
	if (libusb_bulk_transfer(devh, ep_out_addr, &c, 1,
			&actual_length, 0) < 0) {
		fprintf(stderr, "Error while sending char\n");
	}
}


int read_chars(unsigned char * data, int size)
{
	/* To receive characters from the device initiate a bulk_transfer to the
	 * Endpoint with address ep_in_addr.
	 */
	int actual_length;
	int rc = libusb_bulk_transfer(devh, ep_in_addr, data, size, &actual_length,
			1000);
	if (rc == LIBUSB_ERROR_TIMEOUT) {
		printf("timeout (%d)\n", actual_length);
		return -1;
	} else if (rc < 0) {
		fprintf(stderr, "Error while waiting for char\n");
		return -1;
	}

	return actual_length;
}
#endif

#if 0
static void print_devs(libusb_device **devs)
{
	libusb_device *dev;
	int i = 0, j = 0;
	uint8_t path[8];

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return;
		}

		printf("%04x:%04x (bus %d, device %d)",
				desc.idVendor, desc.idProduct,
				libusb_get_bus_number(dev), libusb_get_device_address(dev));

		r = libusb_get_port_numbers(dev, path, sizeof(path));
		if (r > 0) {
			printf(" path: %d", path[0]);
			for (j = 1; j < r; j++)
				printf(".%d", path[j]);
		}
		printf("\n");
	}
}
#endif

int main(void)
{
	static volatile int stop_main = 0;

	void handle_signal()
	{
		stop_main = 1;
	}

#if 1
	int rc, if_num ;

	/* Initialize libusb
	 */
	rc = libusb_init(NULL);
	if (rc < 0) {
		fprintf(stderr, "Error initializing libusb: %s\n", libusb_error_name(rc));
		exit(1);
	}

	/*
	 * Set debugging output to max level.
	 */
	libusb_set_debug(NULL, 3);

	/*
	 * Look for a specific device and open it.
	 */
	devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	if (!devh) {
		fprintf(stderr, "Error finding USB device\n");
		goto out;
	}

	/*
	 * As we are dealing with a CDC-ACM device, it's highly probable that
	 * Linux already attached the cdc-acm driver to this device.
	 * We need to detach the drivers from all the USB interfaces. The CDC-ACM
	 * Class defines two interfaces: the Control interface and the
	 * Data interface.
	 */

	for (if_num = 0; if_num < 2; if_num++) {
		if (libusb_kernel_driver_active(devh, if_num)) {
			printf("detaching interface: %d\n", if_num);
			libusb_detach_kernel_driver(devh, if_num);
		}
		rc = libusb_claim_interface(devh, if_num);
		if (rc < 0) {
			fprintf(stderr, "Error claiming interface: %s\n",
					libusb_error_name(rc));
			goto out;
		} else {
			printf("Interface %d claimed\n",if_num);
		}
	}

	/*
	 * Start configuring the device:
	 * - set line state
	 */
	rc = libusb_control_transfer(devh, 0x21, 0x22, ACM_CTRL_DTR | ACM_CTRL_RTS,
			0, NULL, 0, 0);
	if (rc < 0) {
		fprintf(stderr, "Error during control transfer: %s\n",
				libusb_error_name(rc));
	}

	/*
	 * - set line encoding: here 9600 8N1
	 * 9600 = 0x2580 ~> 0x80, 0x25 in little endian
	 * 115200 = 0x1C200 ~> TODO
	 */
	unsigned char encoding[] = { 0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08 };
	rc = libusb_control_transfer(devh, 0x21, 0x20, 0, 0, encoding,
			sizeof(encoding), 0);
	if (rc < 0) {
		fprintf(stderr, "Error during control transfer: %s\n",
				libusb_error_name(rc));
	}


	int len;
	unsigned char * buf = malloc(137);
	memset(buf, 0, sizeof(buf));
	fprintf(stdout, "Received[%d]: \"%s\"\n",len, buf);
	unsigned char command [4];
	command[0] = (char)0x03;	// ID
	command[1] = (char)0x04;	// CC
	command[2] = (char)0x01;	// LEN
	command[4] = (char)0x01;	// DATA
	write_chars(command);
	while(!stop_main) {
		memset(buf, 0, sizeof(buf));
		len = read_chars(buf, 136);
		print_received_buffer(buf, len);
		sleep(1);
	}

	command[0] = (char)0x03;	// ID
	command[1] = (char)0x04;	// CC
	command[2] = (char)0x01;	// LEN
	command[4] = (char)0x0;		// DATA
	write_chars(command);
	libusb_release_interface(devh, 0);


	out:
	if (devh)
		libusb_close(devh);
	libusb_exit(NULL);
	return rc;
#endif
#if 0
	libusb_device **devs;
	int r;
	ssize_t cnt;

	r = libusb_init(NULL);
	if (r < 0)
		return r;

	cnt = libusb_get_device_list(NULL, &devs);
	if (cnt < 0)
		return (int) cnt;

	print_devs(devs);
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);
	return 0;
#endif

}
