/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007 Till Harbaum <till@harbaum.org>
 *
*/

/*! ************************************************************** 
** \file lib/xu1541.c \n
** \author Till Harbaum \n
** \version $Id: xu1541.c,v 1.1.2.1 2007-02-08 19:19:40 harbaum Exp $ \n
** \n
** \brief libusb based xu1541 access routines
**
****************************************************************/

#ifdef ENABLE_XU1541 

#include <stdio.h>

#include "opencbm.h"
#include "xu1541.h"

usb_dev_handle *xu1541_handle = NULL;

/* try to find a xu1541 cable */
int xu1541_init(void) {
  struct usb_bus      *bus;
  struct usb_device   *dev;

  DEBUGF("Scanning usb ...");

  usb_init();
  
  usb_find_busses();
  usb_find_devices();
  
  for(bus = usb_get_busses(); bus; bus = bus->next) {
    for(dev = bus->devices; dev; dev = dev->next) {
      if((dev->descriptor.idVendor == XU1541_VID) &&
         (dev->descriptor.idProduct == XU1541_PID)) {

        DEBUGF("Found xu1541 device on bus %s device %s.",
               bus->dirname, dev->filename);

        /* open device */
        if(!(xu1541_handle = usb_open(dev)))
          fprintf(stderr, "Error: Cannot open USB device: %s\n",
                  usb_strerror());

        break;
      }
    }
  }


  if(!xu1541_handle) {
    DEBUGF("No xu1541 device found");
    return -1;
  }

  return 0;
}

void xu1541_close(void)
{
    DEBUGF("Closing USB link");
    usb_close(xu1541_handle);
}

int xu1541_ioctl(__u_char cmd, __u_char addr, __u_char secaddr)
{
  int nBytes;
  char ret[4] =  { 0x11, 0x22, 0x33, 0x44 };

  DEBUGF("ioctl %d for device %d, sub %d", cmd, addr, secaddr);

  /* USB_TIMEOUT msec timeout required for reset */
  if((nBytes = usb_control_msg(xu1541_handle, 
		   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, cmd,
		     (secaddr << 8) + addr, 0, ret, sizeof(ret), USB_TIMEOUT)) < 0) {
      fprintf(stderr, "USB error in xu1541_ioctl(): %s\n", usb_strerror());
      exit(-1);
      return -1;
  }

  DEBUGF("returned %d bytes", nBytes);

  /* return ok(0) if command does not have a return value */
  if(nBytes == 0) 
    return 0;

  DEBUGF("return val = %x", ret[0]);
  return ret[0];
}

int xu1541_write(const void *data, size_t len) 
{
    int bytesWritten = 0;

    while(len) 
    {
	int wr, bytes2write = (len > 128)?128:len;

	/* USB_TIMEOUT msec timeout required for reset */
	if((wr = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_OUT, 
				 XU1541_WRITE,
				 bytes2write, 0, (void*)data, 
				 bytes2write, USB_TIMEOUT)) < 0) 
	{
	    fprintf(stderr, "USB error xu1541_write(): %s\n", usb_strerror());
      exit(-1);
	    return -1;
	}
	
	len -= wr;
	data += wr;
	bytesWritten += wr;

	DEBUGF("wrote chunk %d bytes, total %d, left %d", 
	       wr, bytesWritten, len);
    }
    return bytesWritten;
}

int xu1541_read(void *data, size_t len) 
{
    int bytesRead = 0;
    
    DEBUGF("request to read %d bytes to %p", len, data);
    
    while(len > 0) 
    {
	int rd, bytes2read = (len > 128)?128:len;
	
	/* USB_TIMEOUT msec timeout required for reset */
	if((rd = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_IN, 
				 XU1541_READ,
				 bytes2read, 0, data, 
				 bytes2read, USB_TIMEOUT)) < 0) 
	{
	    fprintf(stderr, "USB error in xu1541_read(): %s\n", usb_strerror());
      exit(-1);
	    return -1;
	}
	
	len -= rd;
	data += rd;
	bytesRead += rd;

	DEBUGF("received chunk %d bytes, total %d ,left %d", 
	       rd, bytesRead, len);
	
	/* force end of read */
	if(rd < bytes2read) len = 0;
    }
    return bytesRead;
}

int xu1541_special_write(int mode, const void *data, size_t size) 
{
    int bytesWritten = 0;

    while(size > 0) {
	int wr, bytes2write = (size>128)?128:size;

	if((wr = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_OUT, mode,
				 XU1541_WRITE, bytes2write, (void*)data, 
				 bytes2write, USB_TIMEOUT)) < 0) {
	    fprintf(stderr, "USB error in xu1541_special_write(): %s\n", usb_strerror());
      exit(-1);
	    return -1;
	}
	
	size -= wr;
	data += wr;
	bytesWritten += wr;
    }

    return bytesWritten;
}

int xu1541_special_read(int mode, void *data, size_t size) 
{
    int bytesRead = 0;

    while(size > 0) {
	int rd, bytes2read = (size>128)?128:size;
	
	if((rd = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_IN, mode,
				 XU1541_READ, bytes2read, data, 
				 bytes2read, USB_TIMEOUT)) < 0) {
	    fprintf(stderr, "USB error in xu1541_special_read(): %s\n", usb_strerror());
      exit(-1);
	    return -1;
	}
	
	size -= rd;
	data += rd;
	bytesRead += rd;

	/* stop if there's nothing more to read */
	if(rd < bytes2read) size = 0;
    }
    
    return bytesRead;
}

#endif // ENABLE_XU1541 
