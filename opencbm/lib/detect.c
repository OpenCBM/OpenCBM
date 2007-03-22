/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/detect.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: detect.c,v 1.3 2007-03-22 12:50:16 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**
****************************************************************/

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM.DLL"

#include "debug.h"

#include <stdlib.h>

//! mark: We are building the DLL */
#define DLL
#include "opencbm.h"
#include "archlib.h"


/*! \brief Identify the connected floppy drive.

 This function tries to identify a connected floppy drive.
 For this, it performs some M-R operations.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param CbmDeviceType
   Pointer to an enum which will hold the type of the device.

 \param CbmDeviceString
   Pointer to a pointer which will point on a string which
   tells the name of the device.

 \return
   0 if the drive could be contacted. It does not mean that
   the device could be identified.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_identify(CBM_FILE HandleDevice, __u_char DeviceAddress,
             enum cbm_device_type_e *CbmDeviceType, 
             const char **CbmDeviceString)
{
    enum cbm_device_type_e deviceType = cbm_dt_unknown;
    unsigned short magic;
    unsigned char buf[3];
    const char command[] = { 'M', '-', 'R', (char) 0x40, (char) 0xff, (char) 0x02 };
    char *deviceString = "*unknown*";
    int rv = -1;

    FUNC_ENTER();

    if (cbm_exec_command(HandleDevice, DeviceAddress, command, sizeof(command)) == 0 
        && cbm_talk(HandleDevice, DeviceAddress, 15) == 0)
    {
        if (cbm_raw_read(HandleDevice, buf, 3) == 3)
        {
            magic = buf[0] | (buf[1] << 8);

            switch(magic)
            {
                case 0xaaaa:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "1540 or 1541"; 
                    break;

                case 0xf00f:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "1541-II";
                    break;

                case 0xcd18:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "1541C";
                    break;

                case 0x10ca:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "DolphinDOS 1541";
                    break;

                case 0x6f10:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "SpeedDOS 1541";
                    break;

                case 0x8085:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "JiffyDOS 1541";
                    break;

                case 0xaeea:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "64'er DOS 1541";
                    break;

                case 0xfed7:
                    deviceType = cbm_dt_cbm1570;
                    deviceString = "1570";
                    break;

                case 0x02ac:
                    deviceType = cbm_dt_cbm1571;
                    deviceString = "1571";
                    break;

                case 0x01ba:
                    deviceType = cbm_dt_cbm1581;
                    deviceString = "1581";
                    break;
            }
            rv = 0;
        }
        cbm_untalk(HandleDevice);
    }

    if(CbmDeviceType)
    {
        *CbmDeviceType = deviceType;
    }

    if(CbmDeviceString) 
    {
        *CbmDeviceString = deviceString;
    }

    FUNC_LEAVE_INT(rv);
}
