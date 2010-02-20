/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2010 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file libmisc/LINUX/dynlibusb.h \n
** \author Spiro Trikaliotis \n
** \version $Id: dynlibusb.c,v 1.1 2010-02-20 20:50:38 strik Exp $ \n
** \n
** \brief Allow for libusb (0.1) to be loaded dynamically
**        (Currently, this is used on Windows only)
****************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "opencbm.h"

#include "arch.h"
#include "dynlibusb.h"
#include "getpluginaddress.h"

usb_dll_t usb = { 
    usb_open, usb_close, usb_bulk_write, usb_bulk_read, usb_control_msg, usb_set_configuration, usb_claim_interface, usb_release_interface,
    usb_strerror, usb_init, usb_find_busses, usb_find_devices, usb_get_busses
};

int dynlibusb_init(void) {
    int error = 0;

    return error;
}

void dynlibusb_uninit(void) {
}
