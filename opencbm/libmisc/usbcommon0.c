/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2019 Spiro Trikaliotis
 */

/*! **************************************************************
** \file libmisc/usbcommon0.c \n
** \author Spiro Trikaliotis \n
** \n
** \brief Helper function for USB handling, using libusb0
**
****************************************************************/

#include "arch.h"

#include "usbcommon.h"

#include "dynlibusb.h"

#include <stdlib.h>

#if HAVE_LIBUSB0

/*! \internal \brief Get a char* string from the device's Unicode descriptors
    Some data will be lost in this conversion, but we are ok with that.

 \param dev
    libusb device handle

 \param index
    Descriptor string index

 \param langid
    Language code

 \param buf
    Where to store the string. The result is nul-terminated.

 \param buflen
    Length of the output buffer.

 \return
    Returns the length of the string read or 0 on error.
*/
int
usbGetStringAscii(struct opencbm_usb_handle *Xum1541Handle, int index, int langid,
    unsigned char *buf, int buflen)
{
    char buffer[256];
    int rval, i;
    usb_dev_handle *dev = Xum1541Handle->devh;

    rval = usb.control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR,
        (USB_DT_STRING << 8) + index, langid,
        buffer, sizeof(buffer), 1000);
    if (rval < 0)
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

#endif /* #if HAVE_LIBUSB0 */
