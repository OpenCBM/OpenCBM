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

#if HAVE_LIBUSB0
const char * libusb0_dummy_error_name(int error_code);
#endif

usb_dll_t usb = {
    .shared_object_handle = NULL,
#if HAVE_LIBUSB1
    .open = libusb_open,
    .close = libusb_close,
    .bulk_transfer = libusb_bulk_transfer,
    .control_transfer = libusb_control_transfer,
    .set_configuration = libusb_set_configuration,
    .get_configuration = libusb_get_configuration,
    .claim_interface = libusb_claim_interface,
    .release_interface = libusb_release_interface,
    .set_interface_alt_setting = libusb_set_interface_alt_setting,
    .clear_halt = libusb_clear_halt,
    .error_name = libusb_error_name,
    .init = libusb_init,
    .exit = libusb_exit,
    .get_device_descriptor = libusb_get_device_descriptor,
    .get_string_descriptor_ascii = libusb_get_string_descriptor_ascii,
    .get_device = libusb_get_device,
    .get_device_list = libusb_get_device_list,
    .free_device_list = libusb_free_device_list,
    .get_bus_number = libusb_get_bus_number,
    .get_device_address = libusb_get_device_address,
#elif HAVE_LIBUSB0
    .open = usb_open,
    .close = usb_close,
    .bulk_write = usb_bulk_write,
    .bulk_read = usb_bulk_read,
    .control_msg = usb_control_msg,
    .set_configuration = usb_set_configuration,
    .claim_interface = usb_claim_interface,
    .release_interface = usb_release_interface,
    .set_altinterface = usb_set_altinterface,
    .clear_halt = usb_clear_halt,
    .strerror = usb_strerror,
    .error_name = libusb0_dummy_error_name,
    .init = usb_init,
    .find_busses = usb_find_busses,
    .find_devices = usb_find_devices,
    .device = usb_device,
    .get_busses = usb_get_busses
#endif
};

#if HAVE_LIBUSB0
const char * libusb0_dummy_error_name(int error_code)
{
    return usb.strerror();
}
#endif


int dynlibusb_init(void) {
    int error = 0;

    return error;
}

void dynlibusb_uninit(void) {
}
