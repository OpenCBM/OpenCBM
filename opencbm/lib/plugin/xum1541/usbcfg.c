/*
 * USB configuration application for the xum1541
 * Copyright 2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

/*! **************************************************************
** \file lib/plugin/xum1541/usbcfg.c \n
** \author Nate Lawson \n
** \version $Id: usbcfg.c,v 1.2 2010-08-09 19:35:34 wmsr Exp $ \n
** \n
** \brief USB configuration application for the xum1541
****************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "opencbm.h"

#include "arch.h"
#include "dynlibusb.h"
#include "xum1541.h"

/*! \brief Initialize the xum1541 device
  This function tries to find and identify the xum1541 device.

  \return
    0 on success, 1 on error.
*/
int ARCH_MAINDECL
main(int argc, char **argv)
{
    int ret;
    XUM1541_HANDLE HandleXum1541 = NULL;

    ret = dynlibusb_init();
    if (ret == 0) {
        ret = xum1541_init(&HandleXum1541);
    }
    if (ret != 0) {
        fprintf(stderr, "initialization error, aborting\n");
        exit(1);
    }

    if (xum1541_control_msg(HandleXum1541, XUM1541_ENTER_BOOTLOADER) == 0) {
        fprintf(stderr, "Success. xum1541 now in bootloader mode.\n");
        fprintf(stderr, "Run your firmware update program now.\n");
    } else {
        fprintf(stderr, "error entering bootloader mode, aborting\n");
    }

    xum1541_close(&HandleXum1541);
    dynlibusb_uninit();
    exit(0);
}
