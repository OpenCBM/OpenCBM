/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007 Till Harbaum <till@harbaum.org>
 *  Copyright 2009 Spiro Trikaliotis
 *
*/

/*! **************************************************************
** \file lib/plugin/xu1541/xu1541.c \n
** \author Till Harbaum \n
** \n
** \brief libusb based xu1541 access routines
**
****************************************************************/

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "opencbm.h"

#include "arch.h"
#include "dynlibusb.h"
#include "getpluginaddress.h"
#include "xu1541.h"

static int debug_level = -10000; /*!< \internal \brief the debugging level for debugging output */

/*! \brief timeout value, used mainly after errors \todo What is the exact purpose of this? */
#define TIMEOUT_DELAY  25000   // 25ms

/*! \internal \brief Output debugging information for the xu1541

 \param level
   The output level; output will only be produced if this level is less or equal the debugging level

 \param msg
   The printf() style message to be output
*/
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

/*! \brief initialise the xu1541 device

  This function tries to find and identify the xu1541 device.

  \param HandleXu1541_p
    pointer to where to store the handle to the xu1541 device

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
int xu1541_init(struct opencbm_usb_handle **HandleXu1541_p) {
  struct opencbm_usb_handle *HandleXu1541;
  unsigned char version[4];
  int len;
  int interface_claimed = 0;
  int success = 0;
  int err = 0;
#if HAVE_LIBUSB0
  struct usb_bus      *bus;
  struct usb_device   *dev;
#elif HAVE_LIBUSB1
  libusb_device **list;
  struct libusb_device_descriptor descriptor;
  ssize_t cnt;
  ssize_t i = 0;
  unsigned char string[256];
#endif

  if (HandleXu1541_p == NULL) {
    perror("xu1541_init: HandleXu1541_p is NULL");
    return -1;
  }

  *HandleXu1541_p = HandleXu1541 = malloc(sizeof(struct opencbm_usb_handle));
  if (!HandleXu1541) return -1;
  HandleXu1541->devh = NULL;

  xu1541_dbg(0, "Scanning usb ...");

#if HAVE_LIBUSB0
  usb.init();

  usb.find_busses();
  usb.find_devices();

  /* usb_find_devices sets errno if some devices don't reply 100% correct. */
  /* make lib ignore this as this has nothing to do with our device */
  errno = 0;

#elif HAVE_LIBUSB1
  usb.init(&HandleXu1541->ctx);
#endif

#if HAVE_LIBUSB0
  for(bus = usb.get_busses(); !HandleXu1541->devh && bus; bus = bus->next) {
    xu1541_dbg(1, "Scanning bus %s", bus->dirname);

    for(dev = bus->devices; !HandleXu1541->devh && dev; dev = dev->next) {
      xu1541_dbg(1, "Device %04x:%04x at %s",
                 dev->descriptor.idVendor, dev->descriptor.idProduct,
                 dev->filename);

      if((dev->descriptor.idVendor == XU1541_VID) &&
         (dev->descriptor.idProduct == XU1541_PID)) {
        char string[256];
        int  len;

        xu1541_dbg(0, "Found xu1541 device on bus %s device %s.",
               bus->dirname, dev->filename);

        /* open device */
        if(!(HandleXu1541->devh = usb.open(dev)))
          fprintf(stderr, "Error: Cannot open USB device: %s\n",
                  usb.strerror());

        /* get device name and make sure the name is "xu1541" meaning */
        /* that the device is not in boot loader mode */
        len = usbGetStringAscii(HandleXu1541, dev->descriptor.iProduct,
                                0x0409, string, sizeof(string));
        if(len < 0){
          fprintf(stderr, "warning: cannot query product "
                  "name for device: %s\n", usb.strerror());
          if(HandleXu1541->devh) usb.close(HandleXu1541->devh);
          HandleXu1541->devh = NULL;
        }

        /* make sure the name matches what we expect */
        if(strcmp(string, "xu1541") != 0) {
          fprintf(stderr, "Error: Found xu1541 in unexpected state,"
                  " please make sure device is _not_ in bootloader mode!\n");

          if(HandleXu1541->devh) usb.close(HandleXu1541->devh);
          HandleXu1541->devh = NULL;
        }
      }
    }
  }
#elif HAVE_LIBUSB1
  cnt = usb.get_device_list(HandleXu1541->ctx, &list);
  if (cnt < 0) {
    xu1541_dbg(0, "enumeration error: %s", usb.error_name((int)cnt));
    usb.exit(HandleXu1541->ctx);
    free(HandleXu1541);
    HandleXu1541 = NULL;
    return -1;
  }

  for (i = 0; i < cnt; i++) {
    libusb_device *device = list[i];

    if (usb.get_device_descriptor(device, &descriptor) != LIBUSB_SUCCESS)
      continue;

    xu1541_dbg(1, "Device %04x:%04x", descriptor.idVendor, descriptor.idProduct);

    // First, find our vendor and product id
    if (descriptor.idVendor != XU1541_VID || descriptor.idProduct != XU1541_PID)
      continue;

    xu1541_dbg(0, "Found xu1541 device on bus %s device %s.",
      usb.get_bus_number(device), usb.get_device_address(device));

    /* open device */
    err = usb.open(device, &HandleXu1541->devh);
    if (err != LIBUSB_SUCCESS) {
      fprintf(stderr, "error: Cannot open USB device: %s\n", usb.error_name(err));
      continue;
    }

    /* get device name and make sure the name is "xu1541" meaning */
    /* that the device is not in boot loader mode */
    len = usb.get_string_descriptor_ascii(HandleXu1541->devh, descriptor.iProduct,
                            string, sizeof(string) - 1);
    if (len < 0) {
      fprintf(stderr, "warning: cannot query product "
              "name for device: %s\n", usb.error_name(len));
      if (HandleXu1541->devh)
        usb.close(HandleXu1541->devh);
      HandleXu1541->devh = NULL;
    }

    /* make sure the name matches what we expect */
    if (strcmp((char*)string, "xu1541") != 0) {
      fprintf(stderr, "Error: Found xu1541 in unexpected state,"
              " please make sure device is _not_ in bootloader mode!\n");

      if (HandleXu1541->devh)
        usb.close(HandleXu1541->devh);
      HandleXu1541->devh = NULL;
    }
  }

  if (HandleXu1541->devh == NULL) {
    fprintf(stderr, "error: no xu1541 device found\n");
    usb.exit(HandleXu1541->ctx);
    free(HandleXu1541);
    HandleXu1541 = NULL;
    return -1;
  }

#endif

  if(!HandleXu1541->devh) {
      fprintf(stderr, "ERROR: No xu1541 device found\n");
      return -1;
  }

  do {
    err = usb.set_configuration(HandleXu1541->devh, 1);
    if (err != LIBUSB_SUCCESS) {
        fprintf(stderr, "USB error: %s\n", usb.error_name(err));
        return -1;
    }

    /* Get exclusive access to interface 0. */
    err = usb.claim_interface(HandleXu1541->devh, 0);
    if (err != LIBUSB_SUCCESS) {
        fprintf(stderr, "USB error: %s\n", usb.error_name(err));
        return -1;
    }

    interface_claimed = 1;

    /* check the devices version number as firmware x.06 changed everything */
#if HAVE_LIBUSB0
    len = usb.control_msg(HandleXu1541->devh,
             USB_TYPE_CLASS | USB_ENDPOINT_IN,
             XU1541_INFO, 0, 0, (char*)version, sizeof(version), 1000);
#elif HAVE_LIBUSB1
    len = usb.control_transfer(HandleXu1541->devh,
             LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
             XU1541_INFO, 0, 0, version, sizeof(version), 1000);
#endif

    if(len < 0) {
      fprintf(stderr, "USB request for XU1541 info failed: %s!\n",
              usb.error_name(len));
      break;
    }

    if(len != sizeof(version)) {
      fprintf(stderr, "Unexpected number of bytes (%d) returned\n", len);
      break;
    }

    xu1541_dbg(0, "firmware version %x.%02x", version[0], version[1]);

    if(version[1] < 8) {
      fprintf(stderr, "Device reports firmware version %x.%02x\n",
              version[0], version[1]);
      fprintf(stderr, "but this version of opencbm requires at least "
              "version x.08\n");
      break;
    }

    success = 1;

  } while (0);

  /* error cleanup */
  if (!success) {
    if (interface_claimed) {
      usb.release_interface(HandleXu1541->devh, 0);
    }

    xu1541_close(HandleXu1541);
    HandleXu1541 = NULL;
  }

  return success ? 0 : -1;
}

/*! \brief close the xu1541 device

 \param HandleXu1541
   handle to the xu1541 device

 \remark
    This function releases the interface and closes the xu1541 handle.
*/
void xu1541_close(struct opencbm_usb_handle *HandleXu1541)
{
    int ret;

    xu1541_dbg(0, "Closing USB link");

    ret = usb.release_interface(HandleXu1541->devh, 0);
    if(ret != LIBUSB_SUCCESS) {
      fprintf(stderr, "USB error: %s\n", usb.error_name(ret));
    }

    usb.close(HandleXu1541->devh);
#if HAVE_LIBUSB1
    usb.exit(HandleXu1541->ctx);
#endif

    free(HandleXu1541);
}

/*! \brief perform an ioctl on the xu1541

 \param HandleXu1541
   handle to the xu1541 device

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
int xu1541_ioctl(struct opencbm_usb_handle *HandleXu1541, unsigned int cmd, unsigned int addr, unsigned int secaddr)
{
  int nBytes;
  unsigned char ret[4];

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
#if HAVE_LIBUSB0
      if((nBytes = usb.control_msg(HandleXu1541->devh,
                                   USB_TYPE_CLASS | USB_ENDPOINT_IN,
#elif HAVE_LIBUSB1
      if ((nBytes = usb.control_transfer(HandleXu1541->devh,
                                   LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
#endif
                                   (uint8_t) cmd, (secaddr << 8) + addr, 0,
                                   NULL, 0,
                                   1000)) < 0)
      {
          fprintf(stderr, "USB error in xu1541_ioctl(async): %s\n",
                  usb.error_name(nBytes));
          exit(-1);
          return -1;
      }

      /* wait for USB to become available again by requesting the result */
      do
      {
          unsigned char rv[2];

          /* request async result code */
#if HAVE_LIBUSB0
          if(usb.control_msg(HandleXu1541->devh,
                             USB_TYPE_CLASS | USB_ENDPOINT_IN,
                             XU1541_GET_RESULT, 0, 0,
                             (char *)rv, sizeof(rv),
                             1000) == sizeof(rv))
#elif HAVE_LIBUSB1
          if (usb.control_transfer(HandleXu1541->devh,
                             LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
                             XU1541_GET_RESULT, 0, 0,
                             rv, sizeof(rv),
                             1000) == sizeof(rv))
#endif
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
#if HAVE_LIBUSB0
      if((nBytes = usb.control_msg(HandleXu1541->devh,
                   USB_TYPE_CLASS | USB_ENDPOINT_IN,
                   cmd, (secaddr << 8) + addr, 0,
                   (char *)ret, sizeof(ret),
                   USB_TIMEOUT)) < 0)
#elif HAVE_LIBUSB1
      if ((nBytes = usb.control_transfer(HandleXu1541->devh,
                   (uint8_t) LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
                   (uint8_t) cmd, (secaddr << 8) + addr, 0,
                   ret, sizeof(ret),
                   USB_TIMEOUT)) < 0)
#endif
      {
          fprintf(stderr, "USB error in xu1541_ioctl(sync): %s\n",
                  usb.error_name(nBytes));
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

/*! \brief write data to the xu1541 device

 \param HandleXu1541
   handle to the xu1541 device

 \param data
    Pointer to buffer which contains the data to be written to the xu1541

 \param len
    The length of the data buffer to be written to the xu1541

 \return
    The number of bytes written
*/
int xu1541_write(struct opencbm_usb_handle *HandleXu1541, const unsigned char *data, size_t len)
{
    int bytesWritten = 0;

    xu1541_dbg(1, "write %d bytes from address %p", len, data);

    while(len)
    {
        int link_ok = 0, err = 0;
        int wr;
        uint16_t bytes2write;
        bytes2write = (len > XU1541_IO_BUFFER_SIZE)?XU1541_IO_BUFFER_SIZE:len;

        /* the write itself moved the data into the buffer, the actual */
        /* iec write is triggered _after_ this USB write is done */
#if HAVE_LIBUSB0
        if ((wr = usb.control_msg(HandleXu1541->devh,
                                 USB_TYPE_CLASS | USB_ENDPOINT_OUT,
                                 XU1541_WRITE, bytes2write, 0,
                                 (char*)data, bytes2write,
                                 USB_TIMEOUT)) < 0)
#elif HAVE_LIBUSB1
        if ((wr = usb.control_transfer(HandleXu1541->devh,
                                 LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
                                 XU1541_WRITE, bytes2write, 0,
                                 (unsigned char*)data, bytes2write,
                                 USB_TIMEOUT)) < 0)
#endif
        {
            fprintf(stderr, "USB error xu1541_write(): %s\n", usb.error_name(wr));
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
#if HAVE_LIBUSB0
            if(usb.control_msg(HandleXu1541->devh,
                               USB_TYPE_CLASS | USB_ENDPOINT_IN,
                               XU1541_GET_RESULT, 0, 0,
                               (char*)rv, sizeof(rv),
                               1000) == sizeof(rv))
#elif HAVE_LIBUSB1
            if (usb.control_transfer(HandleXu1541->devh,
                               LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
                               XU1541_GET_RESULT, 0, 0,
                               rv, sizeof(rv),
                               1000) == sizeof(rv))
#endif
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

/*! \brief read data from the xu1541 device

 \param HandleXu1541
   handle to the xu1541 device

 \param data
    Pointer to a buffer which will contain the data read from the xu1541

 \param len
    The number of bytes to read from the xu1541

 \return
    The number of bytes read
*/
int xu1541_read(struct opencbm_usb_handle *HandleXu1541, unsigned char *data, size_t len)
{
    int bytesRead = 0;

    xu1541_dbg(1, "read %d bytes to address %p", len, data);

    while(len > 0)
    {
        int rd;
        uint16_t bytes2read;
        int link_ok = 0, err = 0;
        unsigned char rv[2];

        /* limit transfer size */
        bytes2read = (len > XU1541_IO_BUFFER_SIZE)?XU1541_IO_BUFFER_SIZE:len;

        /* request async read, ignore errors as they happen due to */
        /* link being disabled */
#if HAVE_LIBUSB0
        rd = usb.control_msg(HandleXu1541->devh,
                        USB_TYPE_CLASS | USB_ENDPOINT_IN,
#elif HAVE_LIBUSB1
        rd = usb.control_transfer(HandleXu1541->devh,
                        LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
#endif
                        XU1541_REQUEST_READ, bytes2read, 0,
                        NULL, 0,
                        1000);

        if(rd < 0) {
            fprintf(stderr, "USB error in xu1541_request_read(): %s\n",
                    usb.error_name(rd));
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
#if HAVE_LIBUSB0
            if((rd = usb.control_msg(HandleXu1541->devh,
                                     USB_TYPE_CLASS | USB_ENDPOINT_IN,
                                     XU1541_GET_RESULT, 0, 0,
                                     (char*)rv, sizeof(rv),
                                     1000)) == sizeof(rv))
#elif HAVE_LIBUSB1
            if ((rd = usb.control_transfer(HandleXu1541->devh,
                                     LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
                                     XU1541_GET_RESULT, 0, 0,
                                     rv, sizeof(rv),
                                     1000)) == sizeof(rv))
#endif
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
#if HAVE_LIBUSB0
        if((rd = usb.control_msg(HandleXu1541->devh,
                                 USB_TYPE_CLASS | USB_ENDPOINT_IN,
                                 XU1541_READ, bytes2read, 0,
                                 (char*)data, bytes2read, 1000)) < 0)
#elif HAVE_LIBUSB1
        if ((rd = usb.control_transfer(HandleXu1541->devh,
                                  LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
                                  XU1541_READ, bytes2read, 0,
                                  data, bytes2read, 1000)) < 0)
#endif
        {
            fprintf(stderr, "USB error in xu1541_read(): %s\n",
                    usb.error_name(rd));
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

/*! \brief "special" write data to the xu1541 device

 \todo
    What is so special?

 \param HandleXu1541
   handle to the xu1541 device

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
int xu1541_special_write(struct opencbm_usb_handle *HandleXu1541, int mode, const unsigned char *data, size_t size)
{
    int bytesWritten = 0;

    xu1541_dbg(1, "special write %d %d bytes from address %p",
               mode, size, data);

    while(size > 0)
    {
        int wr;
        uint16_t bytes2write = (size>128)?128:size;

#if HAVE_LIBUSB0
        if((wr = usb.control_msg(HandleXu1541->devh,
                                 USB_TYPE_CLASS | USB_ENDPOINT_OUT,
                                 mode, XU1541_WRITE, bytes2write,
                                 (char*)data, bytes2write, 1000)) < 0)
#elif HAVE_LIBUSB1
        if ((wr = usb.control_transfer(HandleXu1541->devh,
                                  (uint8_t) LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
                                  (uint8_t) mode, XU1541_WRITE, bytes2write,
                                  (unsigned char*)data, bytes2write, 1000)) < 0)
#endif
        {
            fprintf(stderr, "USB error in xu1541_special_write(): %s\n",
                    usb.error_name(wr));
            return -1;
        }

        xu1541_dbg(2, "special wrote %d bytes", wr);

        size -= wr;
        data += wr;
        bytesWritten += wr;
    }

    return bytesWritten;
}

/*! \brief "special" read data from the xu1541 device

 \todo
    What is so special?

 \param HandleXu1541
   handle to the xu1541 device

 \param mode
    \todo ???

 \param data
    Pointer to a buffer which will contain the data read from the xu1541

 \param size
    The number of bytes to read from the xu1541

 \return
    The number of bytes read
*/
int xu1541_special_read(struct opencbm_usb_handle *HandleXu1541, int mode, unsigned char *data, size_t size)
{
    int bytesRead = 0;

    xu1541_dbg(1, "special read %d %d bytes to address %p",
               mode, size, data);

    while(size > 0)
    {
        int rd;
        uint16_t bytes2read = (size>128)?128:size;

#if HAVE_LIBUSB0
        if((rd = usb.control_msg(HandleXu1541->devh,
                                 USB_TYPE_CLASS | USB_ENDPOINT_IN,
                                 mode, XU1541_READ, bytes2read,
                                 (char*)data, bytes2read,
                                 USB_TIMEOUT)) < 0)
#elif HAVE_LIBUSB1
        if ((rd = usb.control_transfer(HandleXu1541->devh,
                                 LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_IN,
                                 (uint8_t) mode, XU1541_READ, bytes2read,
                                 data, bytes2read,
                                 USB_TIMEOUT)) < 0)
#endif
        {
            fprintf(stderr, "USB error in xu1541_special_read(): %s\n",
                    usb.error_name(rd));
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
