/*
 * xum1541 driver for bulk and control messages
 * Copyright 2009 Nate Lawson <nate@root.org>
 *
 * Incorporates some code from the xu1541 driver by:
 * Copyright 2007 Till Harbaum <till@harbaum.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

/*! **************************************************************
** \file lib/plugin/xum1541/xum1541.c \n
** \author Nate Lawson \n
** \version $Id: xum1541.c,v 1.1 2009-12-09 05:39:00 natelawson Exp $ \n
** \n
** \brief libusb-based xum1541 access routines
****************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "opencbm.h"
#include "xum1541.h"
#include "arch.h"

static int debug_level = -10000; /*!< \internal \brief the debugging level for debugging output */
static usb_dev_handle *xu1541_handle = NULL; /*!< \internal \brief handle to the xu1541 device */

/*! \brief timeout value, used mainly after errors \todo What is the exact purpose of this? */
#define TIMEOUT_DELAY  25000   // 25ms

/*! \internal \brief Output debugging information for the xu1541

 \param level
   The output level; output will only be produced if this level is less or equal the debugging level

 \param msg
   The printf() style message to be output
*/
static void
xu1541_dbg(int level, char *msg, ...)
{
    va_list argp;

    /* determine debug mode if not yet known */
    if(debug_level == -10000)
    {
       char *val = getenv("XUM1541_DEBUG");
       if(val)
           debug_level = atoi(val);
    }

    if(level <= debug_level)
    {
        fprintf(stderr, "[XUM1541] ");
        va_start(argp, msg);
        vfprintf(stderr, msg, argp);
        va_end(argp);
        fprintf(stderr, "\n");
    }
}

/*! \internal \brief @@@@@ \todo document

 \param dev

 \param index

 \param langid

 \param buf

 \param buflen

 \return
*/
static int
usbGetStringAscii(usb_dev_handle *dev, int index, int langid,
                  char *buf, int buflen)
{
    char buffer[256];
    int rval, i;

    if ((rval = usb_control_msg(dev, USB_ENDPOINT_IN,
                                USB_REQ_GET_DESCRIPTOR,
                                (USB_DT_STRING << 8) + index,
                                langid, buffer, sizeof(buffer), 1000)) < 0)
        return rval;

    if (buffer[1] != USB_DT_STRING)
        return 0;

    if ((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];

    rval /= 2;
    /* lossy conversion to ISO Latin1 */
    for (i = 1; i < rval; i++) {
        if (i > buflen)  /* destination buffer overflow */
            break;
        buf[i-1] = buffer[2 * i];
        if (buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i-1] = '?';
    }
    buf[i-1] = 0;
    return i - 1;
}

/*! \brief initialise the xu1541 device

  This function tries to find and identify the xu1541 device.

  \return
    0 on success, -1 on error.

  \remark
    On success, xu1541_handle contains a valid handle to the xu1541 device.
    In this case, the device configuration has been set and the interface
    been claimed.

  \bug
    On some error types, this function might return error, but might
    has opened the xu1541_handle. In this case, the handle is leaked, as
    xu1541_close() is not to be called.
*/
/* try to find a xu1541 cable */
int
xu1541_init(void)
{
  struct usb_bus      *bus;
  struct usb_device   *dev;
  unsigned char ret[2];
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

      if((dev->descriptor.idVendor == XUM1541_VID) &&
         (dev->descriptor.idProduct == XUM1541_PID)) {
        char    string[256];
        int     len;

        xu1541_dbg(0, "Found xum1541 device version %d on bus %s device %s.",
               dev->descriptor.bcdDevice, bus->dirname, dev->filename);

        /* open device */
        if(!(xu1541_handle = usb_open(dev)))
          fprintf(stderr, "Error: Cannot open USB device: %s\n",
                  usb_strerror());

        /* get device name */
        len = usbGetStringAscii(xu1541_handle, dev->descriptor.iProduct,
                                0x0409, string, sizeof(string));
        if(len < 0){
          fprintf(stderr, "warning: cannot query product "
                  "name for device: %s\n", usb_strerror());
          if(xu1541_handle) usb_close(xu1541_handle);
          xu1541_handle = NULL;
        }
        xu1541_dbg(0, "xum1541 name: %.*s\n", len, string);
      }
    }
  }

  if(!xu1541_handle) {
      fprintf(stderr, "ERROR: No xum1541 device found\n");
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

  /* check the basic device info message */
  len = usb_control_msg(xu1541_handle,
           USB_TYPE_CLASS | USB_ENDPOINT_IN,
           XUM1541_INFO, 0, 0, (char*)ret, sizeof(ret), 1000);
  if (len < 0) {
    fprintf(stderr, "USB request for XUM1541 info failed: %s!\n",
            usb_strerror());
    return -1;
  }

  if(len != sizeof(ret)) {
    fprintf(stderr, "Unexpected number of bytes (%d) returned\n", len);
    return -1;
  }

  xu1541_dbg(0, "firmware version %x.%02x", ret[0], ret[1]);
  if (ret[0] < 4) {
    fprintf(stderr, "xum1541 version too low (%x < 4)\n", ret[0]);
    xu1541_close();
    return -1;
  }

  return 0;
}

/*! \brief close the xu1541 device

 \remark
    This function releases the interface and closes the xu1541 handle.
*/
void
xu1541_close(void)
{
    xu1541_dbg(0, "Closing USB link");

    if (usb_release_interface(xu1541_handle, 0) != 0)
        fprintf(stderr, "USB release intf error: %s\n", usb_strerror());

    if (usb_close(xu1541_handle) != 0)
        fprintf(stderr, "USB close error: %s\n", usb_strerror());
    xu1541_handle = NULL;
}

/*! \brief perform an ioctl on the xu1541

 \param cmd
   The IOCTL number

 \param addr
   The (IEC) device to use

 \param secaddr
   The (IEC) secondary address to use

 \return
   Depends upon the IOCTL.

 \todo
   Rework for cleaner structure. Currently, this is a mess!
*/
int
xu1541_ioctl(unsigned int cmd, unsigned int addr, unsigned int secaddr)
{
  int nBytes;
  char ret[4], cmdBuf[XUM_CMDBUF_SIZE];

  xu1541_dbg(1, "ioctl %d for device %d, sub %d", cmd, addr, secaddr);
  cmdBuf[0] = (char)cmd;
  cmdBuf[1] = (char)addr;
  cmdBuf[2] = (char)secaddr;
  cmdBuf[3] = 0;

  /* some commands are being handled asynchronously, namely the ones that */
  /* send a iec byte. These need to ask for the result with a seperate */
  /* command */
  if (cmd == XUM1541_RESET)
  {
      /* sync transfer, read result directly */
      /* USB_TIMEOUT msec timeout required for reset */
      if((nBytes = usb_control_msg(xu1541_handle,
                   USB_TYPE_CLASS | USB_ENDPOINT_IN,
                   cmd, (secaddr << 8) + addr, 0,
                   NULL, 0,
                   USB_TIMEOUT)) < 0)
      {
          fprintf(stderr, "USB error in xu1541_ioctl(control): %s\n",
                  usb_strerror());
          exit(-1);
      }
  }
  else
  {
      int link_ok, err = 0;
      unsigned char rv[2];

      if((nBytes = usb_bulk_write(xu1541_handle,
                                  XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                  cmdBuf, sizeof(cmdBuf), 1000)) < 0)
      {
          fprintf(stderr, "USB error in xu1541_ioctl(async) cmd: %s\n",
                  usb_strerror());
          exit(-1);
          return -1;
      }

      if((cmd == XUM1541_TALK)   || (cmd == XUM1541_UNTALK) ||
         (cmd == XUM1541_LISTEN) || (cmd == XUM1541_UNLISTEN) ||
         (cmd == XUM1541_OPEN)   || (cmd == XUM1541_CLOSE))
      {
          /* wait for USB to become available again by requesting the result */
          cmdBuf[0] = XUM1541_GET_RESULT;
          cmdBuf[1] = 0;
          cmdBuf[2] = sizeof(rv);
          cmdBuf[3] = 0;
          if((nBytes = usb_bulk_write(xu1541_handle,
                                  XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                  cmdBuf, sizeof(cmdBuf), 1000)) < 0)
          {
              fprintf(stderr, "USB error in xu1541_ioctl(async) result: %s\n",
                      usb_strerror());
              exit(-1);
          }

          link_ok = 0;
          while (!link_ok)
          {
              if((nBytes = usb_bulk_read(xu1541_handle,
                  XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
                  (char*)rv, sizeof(rv), LIBUSB_NO_TIMEOUT)) == sizeof(rv))
              {
                  /* we got a valid result */
                  if (rv[0] == XUM1541_IO_RESULT)
                  {
                      /* use that result */
                      nBytes = sizeof(rv) - 1;
                      ret[0] = rv[1];
                      /* a returned byte means that the USB link is fine */
                      link_ok = 1;
                      errno = 0;
                  }
                  else
                  {
                      xu1541_dbg(3, "unexpected result (%d/%d)", rv[0], rv[1]);
                      return -1;
                  }
              }
              else
              {
                  /* count the error states (just out of couriosity) */
                  xu1541_dbg(3, "usb timeout in async");
                  err++;
                  arch_usleep(TIMEOUT_DELAY);
              }
          }
      }
      else if (cmd == XUM1541_GET_EOI || cmd == XUM1541_IEC_WAIT ||
               cmd == XUM1541_IEC_POLL || cmd == XUM1541_PP_READ)
      {
        int retries = 5;
        do {
            if((nBytes = usb_bulk_read(xu1541_handle,
                               XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
                               ret, 1, 1000)) == 1)
            {
                break;
            } else {
                xu1541_dbg(3, "usb timeout in get retval: %s",
                    usb_strerror());
            }
        } while (retries-- != 0);
        if (retries == 0) {
            fprintf(stderr, "ERROR: get retval retries exhausted\n");
            return -1;
        }
      }
  }

  xu1541_dbg(2, "returned %d bytes", nBytes);

  /* return ok(0) if command does not have a return value */
  if (nBytes == 0)
      return 0;

  xu1541_dbg(2, "return val = %x", ret[0]);
  return ret[0];
}

/*! \brief write data to the xu1541 device

 \param data
    Pointer to buffer which contains the data to be written to the xu1541

 \param len
    The length of the data buffer to be written to the xu1541

 \return
    The number of bytes written
*/
int
xu1541_write(const __u_char *data, size_t len)
{
    size_t bytesWritten;
    char cmdBuf[XUM_CMDBUF_SIZE];

    xu1541_dbg(1, "write %d bytes from address %p", len, data);

    bytesWritten = 0;
    while (bytesWritten < len)
    {
        int wr, bytes2write, bytesWrittenChunk;
        int link_ok, retries, err = 0;
        unsigned char rv[2];

        bytes2write = len - bytesWritten;
        bytes2write = (bytes2write > XUM1541_IO_BUFFER_SIZE) ?
            XUM1541_IO_BUFFER_SIZE : bytes2write;

        // Setup our command descriptor
        cmdBuf[0] = XUM1541_WRITE;
        cmdBuf[1] = XUM1541_CBM;
        cmdBuf[2] = bytes2write & 0xff;
        cmdBuf[3] = (bytes2write >> 8) & 0xff;

        /* the write itself moved the data into the buffer, the actual */
        /* iec write is triggered _after_ this USB write is done */
        if((wr = usb_bulk_write(xu1541_handle,
                                XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                cmdBuf, sizeof(cmdBuf), 1000)) < 0)
        {
            fprintf(stderr, "USB error xu1541_write cmd: %s\n", usb_strerror());
            return -1;
        }

        // Move the data itself now in chunks of the endpoint size
        bytesWrittenChunk = 0;
        retries = 5;
        while (bytesWrittenChunk < bytes2write) {
            int chunkSize;

            chunkSize = bytes2write - bytesWrittenChunk;
            chunkSize = (chunkSize > XUM_ENDPOINT_BULK_SIZE) ?
                XUM_ENDPOINT_BULK_SIZE : chunkSize;
            if((wr = usb_bulk_write(xu1541_handle,
                                    XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                    (char *)data, chunkSize,
                                    USB_TIMEOUT)) < 0)
            {
                fprintf(stderr, "USB error in xu1541_write data: %s\n",
                    usb_strerror());
                if (retries-- == 0) {
                    fprintf(stderr, "retry count exceeded, exiting\n");
                    return -1;
                }
            }

            // Stop if nothing left to write
            if (wr == 0)
                break;

            data += wr;
            bytesWrittenChunk += wr;
            bytesWritten += wr;
            xu1541_dbg(2, "wrote chunk of %d bytes, total %d, left %d",
                       wr, bytesWritten, len - bytesWritten);
        }

        /* wait for USB to become available again by requesting the result */
        cmdBuf[0] = XUM1541_GET_RESULT;
        cmdBuf[1] = 0;
        cmdBuf[2] = sizeof(rv);
        cmdBuf[3] = 0;
        if ((wr = usb_bulk_write(xu1541_handle,
                                 XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                 cmdBuf, sizeof(cmdBuf), 1000)) < 0)
        {
            fprintf(stderr, "USB error in xu1541_ioctl(async): %s\n",
                  usb_strerror());
            exit(-1);
        }

        link_ok = 0;
        while (!link_ok) {
            if((wr = usb_bulk_read(xu1541_handle,
                               XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
                               (char*)rv, sizeof(rv), 1000)) == sizeof(rv))
            {
                xu1541_dbg(2, "got result %d/%d", rv[0], rv[1]);

                /* the USB link is available again if we got a valid result */
                if (rv[0] == XUM1541_IO_RESULT) {
                    link_ok = 1;
                    errno = 0;

                    /* device reports failure, stop writing */
                    if (rv[1] == 0)
                        return bytesWritten;
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
    }
    xu1541_dbg(2, "xu1541_write done, got %d bytes", bytesWritten);
    return bytesWritten;
}

/*! \brief read data from the xu1541 device

 \param data
    Pointer to a buffer which will contain the data read from the xu1541

 \param len
    The number of bytes to read from the xu1541

 \return
    The number of bytes read
*/
int
xu1541_read(__u_char *data, size_t len)
{
    size_t bytesRead;
    char cmdBuf[XUM_CMDBUF_SIZE];

    xu1541_dbg(1, "read %d bytes to address %p", len, data);

    bytesRead = 0;
    while (bytesRead < len)
    {
        int rd, bytes2read, bytesReadChunk;
        int link_ok, retries, err = 0;
        unsigned char rv[2];

        /* limit transfer size to xum1541 internal buffer */
        bytes2read = len - bytesRead;
        bytes2read = (bytes2read > XUM1541_IO_BUFFER_SIZE) ?
            XUM1541_IO_BUFFER_SIZE : bytes2read;

        // Setup our command descriptor
        cmdBuf[0] = XUM1541_REQUEST_READ;
        cmdBuf[1] = XUM1541_CBM;
        cmdBuf[2] = bytes2read & 0xff;
        cmdBuf[3] = (bytes2read >> 8) & 0xff;

        /* request async read, ignore errors as they happen due to */
        /* link being disabled */
        if ((rd = usb_bulk_write(xu1541_handle,
                                 XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                 cmdBuf, sizeof(cmdBuf), 1000)) < 0)
        if (rd < 0) {
            fprintf(stderr, "USB error in xu1541_request_read(): %s\n",
                    usb_strerror());
            exit(-1);
        }

        /* since the xu1541 may disable interrupts and wouldn't be able */
        /* to do proper USB communication we'd have to expect USB errors */

        /* try to get result, to make sure usb link is working again */
        /* we can't do this in the read itself since windows returns */
        /* just 0 bytes while the USB link is down which we can't */
        /* distinguish from a real 0 byte read event */

        xu1541_dbg(2, "sent request for %d bytes, waiting for result",
                   bytes2read);

        /* get the result code which also contains the current state */
        /* the xu1541 is in so we know when it's done reading on IEC */
        cmdBuf[0] = XUM1541_GET_RESULT;
        cmdBuf[1] = 0;
        cmdBuf[2] = sizeof(rv);
        cmdBuf[3] = 0;
        if((rd = usb_bulk_write(xu1541_handle,
                                XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                cmdBuf, sizeof(cmdBuf), 1000)) < 0)
        {
            fprintf(stderr, "USB error in xu1541_ioctl(async): %s\n",
                  usb_strerror());
            exit(-1);
        }

        // Loop to wait for the result
        link_ok = 0;
        while (!link_ok) {
            if((rd = usb_bulk_read(xu1541_handle,
                               XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
                               (char*)rv, sizeof(rv), 1000)) == sizeof(rv))
            {
                xu1541_dbg(2, "got result %d/%d", rv[0], rv[1]);

                // if the first byte is still not XUM1541_IO_READ_DONE,
                // then the device hasn't even entered the copy routine yet,
                // so sleep to not slow it down by overloading it with USB
                // requests
                if (rv[0] != XUM1541_IO_READ_DONE)
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
                /* count the error states (just out of couriosity) */
                xu1541_dbg(3, "usb timeout");
                err++;
            }
        }

        // Now that it's ready, send the read command for the data itself
        cmdBuf[0] = XUM1541_READ;
        cmdBuf[1] = XUM1541_CBM;
        cmdBuf[2] = bytes2read & 0xff;
        cmdBuf[3] = (bytes2read >> 8) & 0xff;
        if((rd = usb_bulk_write(xu1541_handle,
                                XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                cmdBuf, sizeof(cmdBuf), 1000)) < 0)
        {
            fprintf(stderr, "USB error in xu1541_read data: %s\n",
                    usb_strerror());
            return -1;
        }

        // Pull in the data in chunks of the endpoint size
        bytesReadChunk = 0;
        retries = 5;
        while (bytesReadChunk < bytes2read) {
            int chunkSize;

            chunkSize = bytes2read - bytesReadChunk;
            chunkSize = (chunkSize > XUM_ENDPOINT_BULK_SIZE) ?
                XUM_ENDPOINT_BULK_SIZE : chunkSize;
            if ((rd = usb_bulk_read(xu1541_handle,
                                    XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
                                    (char *)data, chunkSize, 1000)) < 0)
            {
                fprintf(stderr, "USB error in xu1541_read(): %s\n",
                    usb_strerror());
                if (retries-- == 0) {
                    fprintf(stderr, "retry count exceeded, exiting\n");
                    return -1;
                }
            }

            data += rd;
            bytesReadChunk += rd;
            bytesRead += rd;
            xu1541_dbg(2, "received chunk of %d bytes, total %d, left %d",
                       rd, bytesRead, len - bytesRead);

            // Stop if nothing left to read
            if (rd < chunkSize)
                goto out;
        }
    }

out:
    xu1541_dbg(2, "xu1541_read done, got %d bytes", bytesRead);
    return bytesRead;
}

/*! \brief "special" write data to the xu1541 device

 \todo
    What is so special?

 \param mode
    \todo ???

 \param data
    Pointer to buffer which contains the data to be written to the xu1541

 \param size
    The length of the data buffer to be written to the xu1541

 \return
    The number of bytes written

 \remark
     current all special mode are able to work asynchronously. this means
     that we can just handle them in the device at the same time as the USB
     transfers.
*/
int
xu1541_special_write(__u_char mode, const __u_char *data, size_t size)
{
    int wr;
    size_t bytesWritten;
    char cmdBuf[XUM_CMDBUF_SIZE];
    int retries;

    xu1541_dbg(1, "special write %d %d bytes from address %p",
               mode, size, data);

    // Send the write command
    cmdBuf[0] = XUM1541_WRITE;
    cmdBuf[1] = mode;
    cmdBuf[2] = size & 0xff;
    cmdBuf[3] = (size >> 8) & 0xff;
    if((wr = usb_bulk_write(xu1541_handle,
                            XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                            cmdBuf, sizeof(cmdBuf), 1000)) < 0)
    {
        fprintf(stderr, "USB error in special_write cmd: %s\n",
                usb_strerror());
        return -1;
    }

    bytesWritten = 0;
    retries = 5;
    while (bytesWritten < size)
    {
        int bytes2write;

        bytes2write = size - bytesWritten;
        if (bytes2write > XUM_ENDPOINT_BULK_SIZE)
            bytes2write = XUM_ENDPOINT_BULK_SIZE;
        if((wr = usb_bulk_write(xu1541_handle,
                                XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                                (char *)data, bytes2write, 1000)) < 0)
        {
            if (retries-- == 0) {
                fprintf(stderr, "USB error in special_write data: %s\n",
                        usb_strerror());
                return -1;
            }
            continue;
        }

        /* stop if there's nothing more to write */
        if (wr == 0)
            break;

        xu1541_dbg(2, "special wrote %d bytes", wr);
        data += wr;
        bytesWritten += wr;
    }

    xu1541_dbg(2, "special write done, got %d bytes", bytesWritten);
    return bytesWritten;
}

/*! \brief "special" read data from the xu1541 device

 \todo
    What is so special?

 \param mode
    \todo ???

 \param data
    Pointer to a buffer which will contain the data read from the xu1541

 \param size
    The number of bytes to read from the xu1541

 \return
    The number of bytes read
*/
int
xu1541_special_read(__u_char mode, __u_char *data, size_t size)
{
    int rd;
    size_t bytesRead;
    char cmdBuf[XUM_CMDBUF_SIZE];
    int retries;

    xu1541_dbg(1, "special read %d %d bytes to address %p",
               mode, size, data);

    // Send the read command
    cmdBuf[0] = XUM1541_READ;
    cmdBuf[1] = mode;
    cmdBuf[2] = size & 0xff;
    cmdBuf[3] = (size >> 8) & 0xff;
    if((rd = usb_bulk_write(xu1541_handle,
                            XUM_BULK_OUT_ENDPOINT | USB_ENDPOINT_OUT,
                            cmdBuf, sizeof(cmdBuf), 1000)) < 0)
    {
        fprintf(stderr, "USB error in special_read cmd: %s\n",
                usb_strerror());
        return -1;
    }

    bytesRead = 0;
    retries = 5;
    while (bytesRead < size)
    {
        int bytes2read;

        bytes2read = size - bytesRead;
        if (bytes2read > XUM_ENDPOINT_BULK_SIZE)
            bytes2read = XUM_ENDPOINT_BULK_SIZE;
        if((rd = usb_bulk_read(xu1541_handle,
                               XUM_BULK_IN_ENDPOINT | USB_ENDPOINT_IN,
                               (char *)data, bytes2read, 1000)) < 0)
        {
            if (retries-- == 0) {
                fprintf(stderr, "USB error in special_read data: %s\n",
                        usb_strerror());
                return -1;
            }
            continue;
        }

        /* stop if there's nothing more to read */
        if (rd == 0)
            break;

        xu1541_dbg(2, "special read %d bytes", rd);
        data += rd;
        bytesRead += rd;
    }

    xu1541_dbg(2, "special read done, got %d bytes", bytesRead);
    return bytesRead;
}
