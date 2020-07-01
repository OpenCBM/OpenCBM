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
** \file libmisc/WINDOWS/dynlibusb.h \n
** \author Spiro Trikaliotis \n
** \n
** \brief Allow for libusb (0.1 or 1.0) to be loaded dynamically
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

usb_dll_t usb = { NULL };

#if HAVE_LIBUSB1
  #define LIBUSB_DLLNAME "libusb-1.0.dll"
  #define LIBUSB_DLLFUNCPREFIX "libusb"
#elif HAVE_LIBUSB0
  #define LIBUSB_DLLNAME "libusb0.dll"
  #define LIBUSB_DLLFUNCPREFIX "usb"
#else
  #error Neither HAVE_LIBUSB0 nor HAVE_LIBUSB1 defined!
#endif

int dynlibusb_init(void) {
    int error = 1;

    do {
        usb.shared_object_handle = plugin_load(LIBUSB_DLLNAME);
        if ( ! usb.shared_object_handle ) {
            break;
        }

#define READ(_x) \
    usb._x = plugin_get_address(usb.shared_object_handle, LIBUSB_DLLFUNCPREFIX "_" ## #_x); \
    if (usb._x == NULL) { \
        break; \
    }

#if HAVE_LIBUSB1
        READ(open);
        READ(close);
//        READ(get_string);
//        READ(get_string_simple);
//        READ(get_descriptor_by_endpoint);
//        READ(get_descriptor);
        READ(bulk_transfer);
//        READ(interrupt_write);
//        READ(interrupt_read);
        READ(control_transfer);
        READ(set_configuration);
        READ(get_configuration);
        READ(claim_interface);
        READ(release_interface);
        READ(set_interface_alt_setting);
//        READ(resetep);
        READ(clear_halt);
//        READ(reset);
        READ(error_name);
        READ(init);
        READ(exit);
        READ(get_device_descriptor);
        READ(get_string_descriptor_ascii);
//        READ(set_debug);
        READ(get_device);
        READ(get_device_list);
        READ(free_device_list);
        READ(get_bus_number);
        READ(get_device_address);
#elif HAVE_LIBUSB0
        READ(open);
        READ(close);
//        READ(get_string);
//        READ(get_string_simple);
//        READ(get_descriptor_by_endpoint);
//        READ(get_descriptor);
        READ(bulk_write);
        READ(bulk_read);
//        READ(interrupt_write);
//        READ(interrupt_read);
        READ(control_msg);
        READ(set_configuration);
        READ(claim_interface);
        READ(release_interface);
        READ(set_altinterface);
        READ(resetep);
        READ(clear_halt);
//        READ(reset);
        READ(strerror);
        READ(init);
//        READ(set_debug);
        READ(find_busses);
        READ(find_devices);
        READ(device);
        READ(get_busses);
#endif

        error = 0;
    } while (0);

    return error;
}

void dynlibusb_uninit(void) {

    do {
        if (usb.shared_object_handle == NULL) {
            break;
        }

        plugin_unload(usb.shared_object_handle);

        memset(&usb, 0, sizeof usb);

    } while (0);

}
