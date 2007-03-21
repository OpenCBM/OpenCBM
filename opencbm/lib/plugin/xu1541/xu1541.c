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
** \version $Id: xu1541.c,v 1.1.2.9 2007-03-21 17:58:29 harbaum Exp $ \n
** \n
** \brief libusb based xu1541 access routines
**
****************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "opencbm.h"
#include "xu1541.h"
#include "arch.h"

static int debug_level = -10000;
static usb_dev_handle *xu1541_handle = NULL;

#define TIMEOUT_DELAY  25000   // 25ms

static void xu1541_dbg(int level, char *msg, ...) 
{
    va_list argp;
    
    /* determine debug mode if not yet known */
    if(debug_level == -10000)
    {
       char *val = getenv("XU1541_DEBUG");
       if(val)
	   debug_level = atoi(val);
    }

    if(level <= debug_level) 
    {
        fprintf(stderr, "[XU1541] ");
        va_start(argp, msg);
	vfprintf(stderr, msg, argp);
	va_end(argp);
	fprintf(stderr, "\n");
    }
}

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
  unsigned char ret[4];
  int len;

  xu1541_dbg(0, "Scanning usb ...");

  usb_init();
  
  usb_find_busses();
  usb_find_devices();

  /* usb_find_devices sets errno if some devices don't reply 100% correct. */
  /* make lib ignore this as this has nothing to do with our device */
  errno = 0;

  for(bus = usb_get_busses(); !xu1541_handle && bus; bus = bus->next) {
    xu1541_dbg(1, "Scanning bus %s", bus->dirname);

    for(dev = bus->devices; !xu1541_handle && dev; dev = dev->next) {
      xu1541_dbg(1, "Device %04x:%04x at %s", 
		 dev->descriptor.idVendor, dev->descriptor.idProduct,
		 dev->filename);

      if((dev->descriptor.idVendor == XU1541_VID) &&
         (dev->descriptor.idProduct == XU1541_PID)) {
	char    string[256];
	int     len;
	
        xu1541_dbg(0, "Found xu1541 device on bus %s device %s.",
               bus->dirname, dev->filename);

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
      }
    }
  }

  if(!xu1541_handle) {
      fprintf(stderr, "ERROR: No xu1541 device found\n");
      return -1;
  }

  if (usb_set_configuration(xu1541_handle, 1) != 0) {
      fprintf(stderr, "USB error: %s\n", usb_strerror());
      return -1;
  }
      
  /* Get exclusive access to interface 0. */
  if (usb_claim_interface(xu1541_handle, 0) != 0) {
      fprintf(stderr, "USB error: %s\n", usb_strerror());
      return -1;
  }

  /* check the devices version number as firmware x.06 changed everything */
  len = usb_control_msg(xu1541_handle, 
	   USB_TYPE_CLASS | USB_ENDPOINT_IN, 
	   XU1541_INFO, 0, 0, (char*)ret, sizeof(ret), 1000);

  if(len < 0) {
    fprintf(stderr, "USB request for XU1541 info failed: %s!\n", 
	    usb_strerror());
    return -1;
  }

  if(len != sizeof(ret)) {
    fprintf(stderr, "Unexpected number of bytes (%d) returned\n", len);
    return -1;
  }

  xu1541_dbg(0, "firmware version %u.%02u", ret[0], ret[1]);

  if(ret[1] < 8) {
    fprintf(stderr, "Device reports firmware version %u.%02u\n", 
	    ret[0], ret[1]);
    fprintf(stderr, "but this version of opencbm requires at least "
	    "version x.08\n");
    return -1;
  }

  return 0;
}

void xu1541_close(void)
{
    xu1541_dbg(0, "Closing USB link");

    if(usb_release_interface(xu1541_handle, 0))
      fprintf(stderr, "USB error: %s\n", usb_strerror());

    usb_close(xu1541_handle);
}

int xu1541_ioctl(unsigned int cmd, unsigned int addr, unsigned int secaddr)
{
  int nBytes;
  char ret[4];

  xu1541_dbg(1, "ioctl %d for device %d, sub %d", cmd, addr, secaddr);

  /* some commands are being handled asynchronously, namely the ones that */
  /* send a iec byte. These need to ask for the result with a seperate */
  /* command */
  if((cmd == XU1541_TALK)   || (cmd == XU1541_UNTALK) ||
     (cmd == XU1541_LISTEN) || (cmd == XU1541_UNLISTEN) ||
     (cmd == XU1541_OPEN)   || (cmd == XU1541_CLOSE)) 
  {
      int link_ok = 0, err = 0;

      /* USB_TIMEOUT msec timeout required for reset */
      if((nBytes = usb_control_msg(xu1541_handle, 
				   USB_TYPE_CLASS | USB_ENDPOINT_IN, 
				   cmd, (secaddr << 8) + addr, 0, 
				   NULL, 0, 
				   1000)) < 0) 
      {
	  fprintf(stderr, "USB error in xu1541_ioctl(async): %s\n", 
		  usb_strerror());
	  exit(-1);
	  return -1;
      }

      /* wait for USB to become available again by requesting the result */
      do 
      {
	  unsigned char rv[2];
	  
	  /* request async result code */
	  if(usb_control_msg(xu1541_handle, 
			     USB_TYPE_CLASS | USB_ENDPOINT_IN, 
			     XU1541_GET_RESULT, 0, 0, 
			     (char*)rv, sizeof(rv), 
			     1000) == sizeof(rv)) 
	  {
	      /* we got a valid result */
	      if(rv[0] == XU1541_IO_RESULT) 
	      {
		  /* use that result */
		  nBytes = sizeof(rv)-1;
		  ret[0] = rv[1];
		  /* a returned byte means that the USB link is fine */
		  link_ok = 1;
		  errno = 0;
	      } 
	      else 
	      {
		  xu1541_dbg(3, "unexpected result (%d/%d)", rv[0], rv[1]);

		  /* not the expected result */
		  arch_usleep(TIMEOUT_DELAY);
	      }
	  } 
	  else 
	  {
	      xu1541_dbg(3, "usb timeout");

	      /* count the error states (just out of couriosity) */
	      err++;

	      arch_usleep(TIMEOUT_DELAY);
	  }
      } 
      while(!link_ok);
  } 
  else 
  {

      /* sync transfer, read result directly */
      if((nBytes = usb_control_msg(xu1541_handle, 
		   USB_TYPE_CLASS | USB_ENDPOINT_IN, 
		   cmd, (secaddr << 8) + addr, 0, 
		   ret, sizeof(ret), 
		   USB_TIMEOUT)) < 0) 
      {
	  fprintf(stderr, "USB error in xu1541_ioctl(sync): %s\n", 
		  usb_strerror());
	  exit(-1);
	  return -1;
      }
  }

  xu1541_dbg(2, "returned %d bytes", nBytes);

  /* return ok(0) if command does not have a return value */
  if(nBytes == 0) 
      return 0;

  xu1541_dbg(2, "return val = %x", ret[0]);
  return ret[0];
}

int xu1541_write(const __u_char *data, size_t len) 
{
    int bytesWritten = 0;

    xu1541_dbg(1, "write %d bytes from address %p", len, data);

    while(len) 
    {
        int link_ok = 0, err = 0;
        int wr, bytes2write;
	bytes2write = (len > XU1541_IO_BUFFER_SIZE)?XU1541_IO_BUFFER_SIZE:len;
	
	/* the write itself moved the data into the buffer, the actual */
	/* iec write is triggered _after_ this USB write is done */
	if((wr = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_OUT, 
				 XU1541_WRITE, bytes2write, 0, 
				 (char*)data, bytes2write, 
				 USB_TIMEOUT)) < 0) 
	{
	    fprintf(stderr, "USB error xu1541_write(): %s\n", usb_strerror());
	    exit(-1);
	    return -1;
	}
	
	len -= wr;
	data += wr;
	bytesWritten += wr;

	xu1541_dbg(2, "wrote chunk of %d bytes, total %d, left %d", 
		   wr, bytesWritten, len);
 
	/* wait for USB to become available again by requesting the result */
	do 
	{
	    unsigned char rv[2];

	    /* request async result */
	    if(usb_control_msg(xu1541_handle, 
			       USB_TYPE_CLASS | USB_ENDPOINT_IN, 
			       XU1541_GET_RESULT, 0, 0, 
			       (char*)rv, sizeof(rv), 
			       1000) == sizeof(rv)) 
	    {
	        /* the USB link is available again if we got a valid result */
	        if(rv[0] == XU1541_IO_RESULT) {
		    /* device reports failure, stop writing */
		    if(!rv[1])
		        len = 0;

		    link_ok = 1;
		    errno = 0;
		} 
		else
		{
		    xu1541_dbg(3, "unexpected result (%d/%d)", rv[0], rv[1]);
		    arch_usleep(TIMEOUT_DELAY);
		}
	    } 
	    else 
	    {
	        xu1541_dbg(3, "usb timeout");
	        /* count the error states (just out of couriosity) */
	        err++;
	    }
	} 
	while(!link_ok);
    }
    return bytesWritten;
}

int xu1541_read(__u_char *data, size_t len) 
{
    int bytesRead = 0;
    
    xu1541_dbg(1, "read %d bytes to address %p", len, data);
    
    while(len > 0) 
    {
	int rd, bytes2read;
	int link_ok = 0, err = 0;
	unsigned char rv[2];
	  
	/* limit transfer size */
	bytes2read = (len > XU1541_IO_BUFFER_SIZE)?XU1541_IO_BUFFER_SIZE:len;

	/* request async read, ignore errors as they happen due to */
	/* link being disabled */
	rd = usb_control_msg(xu1541_handle, 
			USB_TYPE_CLASS | USB_ENDPOINT_IN, 
			XU1541_REQUEST_READ, bytes2read, 0, 
			NULL, 0,
			1000);
	
	if(rd < 0) {
	    fprintf(stderr, "USB error in xu1541_request_read(): %s\n", 
		    usb_strerror());
	    exit(-1);
	    return -1;
	}

	/* since the xu1541 may disable interrupts and wouldn't be able */
	/* to do proper USB communication we'd have to expect USB errors */

	/* try to get result, to make sure usb link is working again */
	/* we can't do this in the read itself since windows returns */
	/* just 0 bytes while the USB link is down which we can't */
	/* distinguish from a real 0 byte read event */

	xu1541_dbg(2, "sent request for %d bytes, waiting for result", 
		   bytes2read);

	do 
	{
	    /* get the result code which also contains the current state */
	    /* the xu1541 is in so we know when it's done reading on IEC */
	    if((rd = usb_control_msg(xu1541_handle, 
				     USB_TYPE_CLASS | USB_ENDPOINT_IN, 
				     XU1541_GET_RESULT, 0, 0, 
				     (char*)rv, sizeof(rv), 
				     1000)) == sizeof(rv)) 
	    {
	        xu1541_dbg(2, "got result %d/%d", rv[0], rv[1]);
	      
		// if the first byte is still not XU1541_IO_READ_DONE,
	        // then the device hasn't even entered the copy routine yet, 
	        // so sleep to not slow it down by overloading it with USB 
	        // requests
	        if(rv[0] != XU1541_IO_READ_DONE) 
		{
		    xu1541_dbg(3, "unexpected result");
		    arch_usleep(TIMEOUT_DELAY);
		} 
		else
		{
		    xu1541_dbg(3, "link ok");

	            link_ok = 1;
		    errno = 0;
		}
	    } 
	    else 
	    {
		xu1541_dbg(3, "usb timeout");

		/* count the error states (just out of couriosity) */
		err++;
	    }
	} 
	while(!link_ok);
	
	/* finally read data itself */
	if((rd = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_IN, 
				 XU1541_READ, bytes2read, 0, 
				 (char*)data, bytes2read, 1000)) < 0) 
	{
	    fprintf(stderr, "USB error in xu1541_read(): %s\n", 
		    usb_strerror());
	    return -1;
	}
	
	len -= rd;
	data += rd;
	bytesRead += rd;

	xu1541_dbg(2, "received chunk of %d bytes, total %d, left %d", 
		   rd, bytesRead, len);
	
	/* force end of read */
	if(rd < bytes2read) 
	    len = 0;
    }
    return bytesRead;
}

/* current all special mode are able to work asynchronously. this means */
/* that we can just handle them in the device at the same time as the USB */
/* transfers. */
int xu1541_special_write(int mode, const __u_char *data, size_t size) 
{
    int bytesWritten = 0;

    xu1541_dbg(1, "special write %d %d bytes from address %p", 
	       mode, size, data);

    while(size > 0) 
    {
	int wr, bytes2write = (size>128)?128:size;

	if((wr = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_OUT, 
				 mode, XU1541_WRITE, bytes2write, 
				 (char*)data, bytes2write, 1000)) < 0) 
	{
	    fprintf(stderr, "USB error in xu1541_special_write(): %s\n", 
		    usb_strerror());
	    return -1;
	}
	
	xu1541_dbg(2, "special wrote %d bytes", wr);

	size -= wr;
	data += wr;
	bytesWritten += wr;
    }

    return bytesWritten;
}

int xu1541_special_read(int mode, __u_char *data, size_t size) 
{
    int bytesRead = 0;

    xu1541_dbg(1, "special read %d %d bytes to address %p", 
	       mode, size, data);

    while(size > 0) 
    {
	int rd, bytes2read = (size>128)?128:size;
	
	if((rd = usb_control_msg(xu1541_handle, 
				 USB_TYPE_CLASS | USB_ENDPOINT_IN, 
				 mode, XU1541_READ, bytes2read, 
				 (char*)data, bytes2read, 
				 USB_TIMEOUT)) < 0) 
	{
	    fprintf(stderr, "USB error in xu1541_special_read(): %s\n", 
		    usb_strerror());
	    return -1;
	}
	
	xu1541_dbg(2, "special read %d bytes", rd);

	size -= rd;
	data += rd;
	bytesRead += rd;

	/* stop if there's nothing more to read */
	if(rd < bytes2read) 
	  size = 0;
    }
    
    return bytesRead;
}
