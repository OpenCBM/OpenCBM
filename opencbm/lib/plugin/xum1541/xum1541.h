/*
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef XUM1541_H
#define XUM1541_H

#include <usb.h>

#include "opencbm.h"
#include "xum1541_types.h"

/* time out 10% after device itself times out */
/* so make sure we should normally never time out on usb */
#define USB_RESET_TIMEOUT (XUM1541_RESET_TIMEOUT * 1100)

// libusb value for "wait forever"
#define LIBUSB_NO_TIMEOUT   -1

/* vendor and product id (donated by ftdi) */
#define XUM1541_VID  0x0403
#define XUM1541_PID  0xc632

/* calls required for standard io */
extern int xu1541_init(void);
extern void xu1541_close(void);
extern int xu1541_ioctl(unsigned int cmd, unsigned int addr, unsigned int secaddr);
extern int xu1541_write(const __u_char *data, size_t len);
extern int xu1541_read(__u_char *data, size_t len);

/* calls for speeder supported modes */
extern int xu1541_special_write(__u_char mode, const __u_char *data, size_t size);
extern int xu1541_special_read(__u_char mode, __u_char *data, size_t size);

#endif // XUM1541_H
