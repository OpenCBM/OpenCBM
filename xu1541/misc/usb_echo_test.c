/*
 * usb_echo_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>

/* vendor and product id */
#define XU1541_VID  0x0403
#define XU1541_PID  0xc632

#include "../firmware/xu1541_types.h"

usb_dev_handle      *handle = NULL;

/* send a number of 16 bit words to the xu1541 interface */
/* and verify that they are correctly returned by the echo */
/* command. This may be used to check the reliability of */
/* the usb interfacing */
#define ECHO_NUM 256

void usb_echo(void) {

  int i, nBytes, errors=0;
  unsigned short val[2], ret[2];

  for(i=0;i<ECHO_NUM;i++) {
    val[0] = rand() & 0xffff;
    val[1] = rand() & 0xffff;
    
    nBytes = usb_control_msg(handle, 
	   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
	   XU1541_ECHO, val[0], val[1], (char*)ret, sizeof(ret), 1000);

    if(nBytes < 0) {
      fprintf(stderr, "USB request failed: %s!\n", usb_strerror());
      return;
    } else if(nBytes != sizeof(ret)) {
      fprintf(stderr, "Unexpected number of bytes (%d) returned\n", nBytes);
      errors++;
    } else if((val[0] != ret[0]) ||(val[1] != ret[1])) {
      fprintf(stderr, "Echo payload mismatch (%x/%x -> %x/%x)\n",
	      val[0], val[1], ret[0], ret[1]);
      errors++;
    }
  }

  if(errors) 
    fprintf(stderr, "ERROR: %d out of %d echo transfers failed!\n", 
	    errors, ECHO_NUM);
  else 
    printf("%d echo test transmissions successful!\n", ECHO_NUM);
}

void display_device_info(void) {
  int nBytes;
  unsigned char reply[4];

  nBytes = usb_control_msg(handle, 
	   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
	   XU1541_INFO, 0, 0, (char*)reply, sizeof(reply), 1000);

  if(nBytes < 0) {
    fprintf(stderr, "USB request for XU1541 info failed: %s!\n", 
	    usb_strerror());
    return;
  } else if(nBytes != sizeof(reply)) {
    fprintf(stderr, "Unexpected number of bytes (%d) returned\n", nBytes);
    return;
  }

  printf("Device reports version %u.%02u\n", reply[0], reply[1]);
  printf("Device reports capabilities 0x%04x\n", *(unsigned short*)(reply+2));
}

int main(int argc, char *argv[]) {
  struct usb_bus      *bus;
  struct usb_device   *dev;
  
  printf("--    XU1541 USB test application      --\n");
  printf("--      (c) 2007 by Till Harbaum       --\n");
  printf("-- http://www.harbaum.org/till/xu1541  --\n");

  usb_init();
  
  usb_find_busses();
  usb_find_devices();
  
  for(bus = usb_get_busses(); bus; bus = bus->next) {
    for(dev = bus->devices; dev; dev = dev->next) {
      if((dev->descriptor.idVendor == XU1541_VID) && 
	 (dev->descriptor.idProduct == XU1541_PID)) {
	
	printf("Found XU1541 device on bus %s device %s.\n", 
	       bus->dirname, dev->filename);
	
	/* open device */
	if(!(handle = usb_open(dev))) 
	  fprintf(stderr, "Error: Cannot open USB device: %s\n", 
		  usb_strerror());

	break;
      }
    }
  }
  
  if(!handle) {
    fprintf(stderr, "Error: Could not find XU1541 device\n");

#ifdef WIN
    printf("Press return to quit\n");
    getchar();
#endif

    exit(-1);
  }

  display_device_info();

  /* make xu1541 interface return some bytes to */
  /* test transfer reliability */
  usb_echo();

  usb_close(handle);

#ifdef WIN
  printf("Press return to quit\n");
  getchar();
#endif

  return 0;
}
