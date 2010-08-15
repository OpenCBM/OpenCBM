/*
 * Copyright (c) 2009-2010 Nate Lawson <nate@root.org>
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

/* 
 * Make our control transfer timeout 10% later than the device itself
 * times out. This is used for both the INIT and RESET messages since
 * INIT can do its own reset if it finds the drive in the middle of
 * a previous aborted transaction.
 */
#define USB_TIMEOUT         ((int)(XUM1541_TIMEOUT * 1100))

// libusb value for "wait forever"
#define LIBUSB_NO_TIMEOUT   -1

const char *xum1541_device_path(int PortNumber);
int xum1541_init               (usb_dev_handle **HandleXum1541, int PortNumber);
void xum1541_close             (usb_dev_handle  *HandleXum1541);
int xum1541_control_msg        (usb_dev_handle  *HandleXum1541, unsigned int cmd);
int xum1541_ioctl              (usb_dev_handle  *HandleXum1541, unsigned int cmd, unsigned int addr, unsigned int secaddr);

// Read/write data in normal CBM and speeder protocol modes
int xum1541_write              (usb_dev_handle  *HandleXum1541, __u_char mode, const __u_char *data, size_t size);
int xum1541_read               (usb_dev_handle  *HandleXum1541, __u_char mode,       __u_char *data, size_t size);

#endif // XUM1541_H
