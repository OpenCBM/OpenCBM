/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2019 Spiro Trikaliotis
 */

/*! **************************************************************
** \file include/usbcommon.h \n
** \author Spiro Trikaliotis \n
** \n
** \brief Some functions for USB handling
**
****************************************************************/

#ifndef CBM_USBCOMMON_H
#define CBM_USBCOMMON_H

#include <stddef.h>

#if HAVE_LIBUSB1
#include <libusb.h>
#elif HAVE_LIBUSB0
#include <usb.h>
#endif

struct opencbm_usb_handle {
#if HAVE_LIBUSB1
        libusb_context *ctx;
        libusb_device_handle *devh;
#elif HAVE_LIBUSB0
        usb_dev_handle *devh; /*!< \internal \brief handle to the xu1541 device */
#else
#error Nothing defined!
#endif
};

#if HAVE_LIBUSB0
extern int usbGetStringAscii(struct opencbm_usb_handle *dev, int index, int langid, unsigned char *buf, int buflen);
#endif

#if HAVE_LIBUSB0
#define LIBUSB_ENDPOINT_IN USB_ENDPOINT_IN

#define LIBUSB_SUCCESS 0
#endif

#endif /* #ifndef CBM_USBCOMMON_H */
