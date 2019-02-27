/*
 * Utility routines for working with xum1541 devices
 *
 * Copyright 2011 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "usb.h"

#if HAVE_LIBUSB0
int xum1541_get_model_version(usb_dev_handle *handle, int *model,
    int *version);
usb_dev_handle *
#elif HAVE_LIBUSB1
int xum1541_get_model_version(libusb_device_handle *handle, libusb_device *dev,
    int *model, int *version);
libusb_device_handle *
#endif
#if HAVE_LIBUSB0
xum1541_find_device(int PortNumber, char *devNameBuf,
    int devNameBufSize);
#elif HAVE_LIBUSB1
xum1541_find_device(libusb_device **usbDevice, int PortNumber, char *devNameBuf,
    int devNameBufSize);
#endif
void verbose_print(char *msg, ...);

extern int verbose;
extern int debug;  // Used by dfu-programmer

// XXX should be from xum1541_types.h
#define XUM1541_ENTER_BOOTLOADER    4
