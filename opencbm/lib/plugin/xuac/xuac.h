/*
 * Copyright (c) 2009-2010 Nate Lawson <nate@root.org>
 * Copyright (c) 2017      Arnd Menge
 *
 * Incorporates content from the xum1541 driver by:
 * Copyright (c) 2009-2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _XUAC_H
#define _XUAC_H

#include <usb.h>

#include "opencbm.h"
#include "xuac_types.h"

/*
 * Compile-time assert to make sure CBM_FILE is large enough.
 * Perhaps this should be in the global opencbm.h
 */
#ifndef CTASSERT
#define CTASSERT(x)         _CTASSERT(x, __LINE__)
#define _CTASSERT(x, y)     __CTASSERT(x, y)
#define __CTASSERT(x, y)    typedef char __assert ## y[(x) ? 1 : -1]
#endif

CTASSERT(sizeof(CBM_FILE) >= sizeof(usb_dev_handle *));

/*
 * Make our control transfer timeout 10% later than the device itself
 * times out. This is used for both the INIT and RESET messages since
 * INIT can do its own reset if it finds the drive in the middle of
 * a previous aborted transaction.
 */
#define USB_TIMEOUT                 ((int)(XUAC_TIMEOUT * 1100))

// libusb value for "wait forever" (signed int)
#define LIBUSB_NO_TIMEOUT           0x7fffffff

// the maximum value for all allowed xuac serial numbers
#define MAX_ALLOWED_XUAC_SERIALNUM  255

const char *xuac_device_path (int PortNumber);
int         xuac_init        (usb_dev_handle **hDev, int PortNumber);
void        xuac_close       (usb_dev_handle *hDev);
int         xuac_control_msg (usb_dev_handle *hDev, unsigned int cmd);

// Send bulk command+subcommand+data to USB device, optionally send bulk data,
// optionally receive bulk data, optionally receive status+data
int Device_comm_bulk(
    usb_dev_handle *hDev,
    unsigned char cmd, unsigned char subcmd, unsigned __int32 cmd_data,
    const unsigned char *SendBuffer, size_t SendLength, int *BytesWritten,
          unsigned char *RecvBuffer, size_t RecvLength, int *BytesRead,
    int *Status, unsigned __int32 *answer_data);

// Send tape operations abort command to USB device
int Device_tap_break(usb_dev_handle *hDev);

#endif // _XUAC_H
