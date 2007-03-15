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
** \file lib/plugin/xu1541/xu1541.c \n
** \author Till Harbaum \n
** \version $Id: xu1541.c,v 1.1.2.3 2007-03-15 15:08:19 strik Exp $ \n
** \n
** \brief libusb based xu1541 access routines
**
****************************************************************/

#include <stdio.h>
#include <string.h>

#include "opencbm.h"
#include "xu1541.h"

#include "arch.h"

usb_dev_handle *xu1541_handle = NULL;

static int  usbGetStringAscii(usb_dev_handle *dev, int index, int langid, 
			      char *buf, int buflen)
{
  char    buffer[256];
  int     rval, i;

  if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, 
			     USB_REQ_GET_DESCRIPTOR, 
			     (USB_DT_STRING << 8) + index, 
			     langid, buffer, sizeof(buffer), 1000)) < 0)
    return rval;

  if(buffer[1] != USB_DT_STRING)
    return 0;

  if((unsigned char)buffer[0] < rval)
    rval = (unsigned char)buffer[0];
  
  rval /= 2;
  /* lossy conversion to ISO Latin1 */
  for(i=1;i<rval;i++){
    if(i > buflen)  /* destination buffer overflow */
      break;
    buf[i-1] = buffer[2 * i];
    if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
      buf[i-1] = '?';
  }
  buf[i-1] = 0;
  return i-1;
}

/* try to find a xu1541 cable */
int xu1541_init(void) {
  struct usb_bus      *bus;
  struct usb_device   *dev;

  DEBUGF(("Scanning usb ..."));

  usb_init();
  
  usb_find_busses();
  usb_find_devices();

  /* usb_find_devices sets errno if some devices reply 100% correct. */
  /* make lib ignore this as this has nothing to do with our device */
  arch_set_errno(0);

  for(bus = usb_get_busses(); bus; bus = bus->next) {
    for(dev = bus->devices; dev; dev = dev->next) {
      if((dev->descriptor.idVendor == XU1541_VID) &&
         (dev->descriptor.idProduct == XU1541_PID)) {
	char    string[256];
	int     len;
	
        DEBUGF(("Found xu1541 device on bus %s device %s.",
               bus->dirname, dev->filename));

        /* open device */
        if(!(xu1541_handle = usb_open(dev)))
          fprintf(stderr, "Error: Cannot open USB device: %s\n",
                  usb_strerror());
	
	/* get device name and make sure the name is "xu1541" meaning */
	/* that the device is not in boot loader mode */
	len = usbGetStringAscii(xu1541_handle, dev->descriptor.iProduct, 
				0x0409, string, sizeof(string));
	if(len < 0){
	  fprintf(stderr, "warning: cannot query product "
		  "name for device: %s\n", usb_strerror());
	  if(xu1541_handle) usb_close(xu1541_handle);
	  xu1541_handle = NULL;
	}

	/* make sure the name matches what we expect */
	if(strcmp(string, "xu1541") != 0) {
	  fprintf(stderr, "Error: Found xu1541 in unexpected state,"
		  " please make sure device is _not_ in bootloader mode!\n");
	  
	  if(xu1541_handle) usb_close(xu1541_handle);
	  xu1541_handle = NULL;
	}
	
        break;
      }
    }
  }
  
  if(!xu1541_handle) {
    DEBUGF(("No xu1541 device found"));
    return -1;
  } else {
#ifndef WIN32
    /* Get exclusive access to interface 0. Does not work under windows. */
    if (usb_claim_interface(xu1541_handle, 0) != 0) {
      fprintf(stderr, "USB error: %s\n", usb_strerror());
      return -1;
    }
#endif
  }

  return 0;
}

void xu1541_close(void)
{
    DEBUGF(("Closing USB link"));

#ifndef WIN32
    if(usb_release_interface(xu1541_handle, 0))
      fprintf(stderr, "USB error: %s\n", usb_strerror());
#endif

    usb_close(xu1541_handle);
}

int xu1541_ioctl(unsigned int cmd, unsigned int addr, unsigned int secaddr)
{
  int nBytes;
  char ret[4];

  DEBUGF(("ioctl %d for device %d, sub %d", cmd, addr, secaddr));

  /* USB_TIMEOUT msec timeout required for reset */
  if((nBytes = usb_control_msg(xu1541_handle, 
		   USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, cmd,
		     (secaddr << 8) + addr, 0, ret, sizeof(ret), USB_TIMEOUT)) < 0) {
      fprintf(stderr, "USB error in xu1541_ioctl(): %s\n", usb_strerror());
      exit(-1);
      return -1;
  }

  DEBUGF(("returned %d bytes", nBytes));

  /* return ok(0) if command does not have a return value */
  if(nBytes == 0) 
    return 0;

  DEBUGF(("return val = %x", ret[0]));
  return ret[0];
}

int xu1541_write(const __u_char *data, size_t len) 
{
    int bytesWritten = 0;

    while(len) 
    {
	int wr, bytes2write = (len > 128)?128:len;

	/* USB_TIMEOUT msec timeout required for reset */
	if((wr = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_OUT, 
				 XU1541_WRITE,
				 bytes2write, 0, (char*)data, 
				 bytes2write, USB_TIMEOUT)) < 0) 
	{
	    fprintf(stderr, "USB error xu1541_write(): %s\n", usb_strerror());
      exit(-1);
	    return -1;
	}
	
	len -= wr;
	data += wr;
	bytesWritten += wr;

	DEBUGF(("wrote chunk %d bytes, total %d, left %d", 
	       wr, bytesWritten, len));
    }
    return bytesWritten;
}

int xu1541_read(__u_char *data, size_t len) 
{
    int bytesRead = 0;
    
    DEBUGF(("request to read %d bytes to %p", len, data));
    
    while(len > 0) 
    {
	int rd, bytes2read = (len > 128)?128:len;
	
	/* USB_TIMEOUT msec timeout required for reset */
	if((rd = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_IN, 
				 XU1541_READ,
				 bytes2read, 0, (char*)data, 
				 bytes2read, USB_TIMEOUT)) < 0) 
	{
	    fprintf(stderr, "USB error in xu1541_read(): %s\n", usb_strerror());
      exit(-1);
	    return -1;
	}
	
	len -= rd;
	data += rd;
	bytesRead += rd;

	DEBUGF(("received chunk %d bytes, total %d ,left %d", 
	       rd, bytesRead, len));
	
	/* force end of read */
	if(rd < bytes2read) len = 0;
    }
    return bytesRead;
}

int xu1541_special_write(int mode, const __u_char *data, size_t size) 
{
    int bytesWritten = 0;

    while(size > 0) {
	int wr, bytes2write = (size>128)?128:size;

	if((wr = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_OUT, mode,
				 XU1541_WRITE, bytes2write, (char*)data, 
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

int xu1541_special_read(int mode, __u_char *data, size_t size) 
{
    int bytesRead = 0;

    while(size > 0) {
	int rd, bytes2read = (size>128)?128:size;
	
	if((rd = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_IN, mode,
				 XU1541_READ, bytes2read, (char*)data, 
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
