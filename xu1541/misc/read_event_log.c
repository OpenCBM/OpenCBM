/*
 * read_event_log.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>

/* vendor and product id */
#define XU1541_VID  0x0403
#define XU1541_PID  0xc632

#include "xu1541_types.h"
#include "xu1541_event_log.h"

usb_dev_handle      *handle = NULL;

#ifdef WIN
#define QUIT_KEY  { printf("Press return to quit\n"); getchar(); }
#else
#define QUIT_KEY
#endif

void dump_event_log(void) {

  int nBytes, i, log_len;
  unsigned char ret[2];

  nBytes = usb_control_msg(handle, 
	   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
	   XU1541_GET_EVENT, 0, 0, (char*)ret, sizeof(ret), 1000);

  if(nBytes < 0) {
    fprintf(stderr, "USB request failed: %s!\n", usb_strerror());
    return;
  } else if(nBytes != sizeof(ret)) {
    fprintf(stderr, "Unexpected number of bytes (%d) returned\n", nBytes);
    return;
  }

  log_len = ret[0];

  printf("Event log buffer size: %d\n", log_len);
  printf("Event log:\n");

  for(i=0;i<log_len;i++) {
    nBytes = usb_control_msg(handle, 
		  USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
		  XU1541_GET_EVENT, i, 0, (char*)ret, sizeof(ret), 1000);

    if(nBytes < 0) {
      fprintf(stderr, "USB request failed: %s!\n", usb_strerror());
      return;
    } else if(nBytes != sizeof(ret)) {
      fprintf(stderr, "Unexpected number of bytes (%d) returned\n", nBytes);
      return;
    }

    switch(ret[1]) {
    case EVENT_NONE:      /* ignore unused entries */
      break;
      
    case EVENT_START:
      printf("  system started\n");
      break;

    case EVENT_RESET_EXT:
      printf("  booted by external reset\n");
      break;
      
    case EVENT_RESET_WATCHDOG:
      printf("  booted by watchdog reset\n");
      break;
      
    case EVENT_RESET_BROWNOUT:
      printf("  booted by brownout reset\n");
      break;
      
    case EVENT_TIMEOUT_FREE_BUS:
      printf("  timeout while waiting for free bus\n");
      break;

    case EVENT_BYTE_NAK:
      printf("  byte not acknowledged\n");
      break;

    case EVENT_TIMEOUT_LISTENER:
      printf("  timeout waiting for listener\n");
      break;

    case EVENT_WRITE_NO_DEV:
      printf("  write: no device\n");
      break;

    case EVENT_WRITE_FAILED:
      printf("  write failed\n");
      break;

    case EVENT_WRITE_DEV_NOT_PRESENT:
      printf("  write: device not present\n");
      break;

    case EVENT_READ_TIMEOUT:
      printf("  read timeout\n");
      break;
      
    case EVENT_READ_ERROR:
      printf("  read error\n");
      break;

    case EVENT_TIMEOUT_IEC_WAIT:
      printf("  timeout in iec wait\n");
      break;
      
    default:
      printf("  Unknown event!\n");
      break;    
    }
  }
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
  } else if((nBytes != sizeof(reply)) && (nBytes != 4)) {
    fprintf(stderr, "Unexpected number of bytes (%d) returned\n", nBytes);
    return;
  }

  if (nBytes > 4)
    printf("Device reports BIOS version %x.%02x\n", reply[4], reply[5]);

  printf("Device reports version %x.%02x\n", reply[0], reply[1]);
  printf("Device reports capabilities 0x%04x\n", *(unsigned short*)(reply+2));
}

int main(int argc, char *argv[]) {
  struct usb_bus      *bus;
  struct usb_device   *dev;
  
  printf("--       XU1541 event log dumper       --\n");
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

    QUIT_KEY;
    exit(-1);
  }

  if (usb_set_configuration(handle, 1) != 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());

    QUIT_KEY;
    return -1;
  }

  /* Get exclusive access to interface 0. */
  if (usb_claim_interface(handle, 0) != 0) {
    fprintf(stderr, "USB error: %s\n", usb_strerror());

    QUIT_KEY;
    return -1;
  }

  display_device_info();

  dump_event_log();

  if(usb_release_interface(handle, 0))
    fprintf(stderr, "USB error: %s\n", usb_strerror());
  
  usb_close(handle);
  
  QUIT_KEY;

  return 0;
}
