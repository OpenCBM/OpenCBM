/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005       Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005, 2025 Spiro Trikaliotis
 *  Copyright 2011            Wolfgang Moser (http://d81.de)
 *  Copyright 2011            Thomas Winkler
 *
*/

/*! **************************************************************
** \file lib/detect.c \n
** \author Michael Klein, Spiro Trikaliotis \n
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
#include "opencbm-dos.h"
#include "archlib.h"


/** @{ @ingroup opencbm_dos */

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
cbm_identify(CBM_FILE HandleDevice, unsigned char DeviceAddress,
             enum cbm_device_type_e *CbmDeviceType,
             const char **CbmDeviceString)
{
    enum cbm_device_type_e deviceType = cbm_dt_unknown;
    unsigned short magic;
    int            magic_extra = -1;

    static char unknownDevice[] = "*unknown*, footprint=<....>";
    char *deviceString = unknownDevice;
    int rv = -1;

    FUNC_ENTER();

    FUNC_PARAM((DBG_PREFIX "HandleDevice = %p, DeviceAddress = 0x%02x, "
                            "&CbmDeviceType = %p, &CbmDeviceString = %p",
                            HandleDevice, DeviceAddress,
                            CbmDeviceType, CbmDeviceString));

    do {
        uint8_t buf[2];

        /* get footprint from 0xFF40 */
        if ((rv = cbm_dos_memory_read(HandleDevice, buf, sizeof buf, DeviceAddress, 0xFF40u, 2, NULL, NULL)) < 0) {
            break;
        }

        magic = buf[0] | (buf[1] << 8);

        if (magic == 0xAAAAu) {
            uint16_t magic2;

            if ((rv = cbm_dos_memory_read(HandleDevice, buf, sizeof buf, DeviceAddress, 0xFFFEu, 2, NULL, NULL)) < 0) {
                break;
            }

            magic2 = buf[0] | (buf[1] << 8);

            if (magic2 != 0xFE67u) {
                magic = magic2;
            }
        }

        if (magic == 0x01ba) {
            /* FD2000/4000 and 1581 must be distinguished */

            /* get footprint from 0x8008 */
            if ((rv = cbm_dos_memory_read(HandleDevice, buf, sizeof buf, DeviceAddress, 0x8008u, 2, NULL, NULL)) < 0) {
                break;
            }

            magic_extra = buf[0] | (buf[1] << 8);
        }

        switch(magic)
        {
            default:
                unknownDevice[22] = ((magic >> 12 & 0x0F) | 0x40);
                unknownDevice[24] = ((magic >>  4 & 0x0F) | 0x40);
                magic &= 0x0F0F;
                magic |= 0x4040;
                unknownDevice[23] = magic >> 8;
                unknownDevice[25] = (char)magic;
                break;

            case 0xfeb6:
                deviceType = cbm_dt_cbm2031;
                deviceString = "2031";
                break;

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

            case 0x2710:
                deviceType = cbm_dt_cbm1541;
                deviceString = "ProfessionalDOS 1541";
                break;

            case 0x8085:
                deviceType = cbm_dt_cbm1541;
                deviceString = "JiffyDOS 1541";
                break;

            case 0xaeea:
                deviceType = cbm_dt_cbm1541;
                deviceString = "64'er DOS 1541";
                break;

            case 0x180d:
                deviceType = cbm_dt_cbm1541;
                deviceString = "Turbo Access / Turbo Trans";
                break;

            case 0x094c:
                deviceType = cbm_dt_cbm1541;
                deviceString = "Prologic DOS";
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
                switch (magic_extra) {
                    case 0x4446: /* the "FD" in "CMD FD DOS" */
                        deviceType = cbm_dt_fdx000;
                        deviceString = "FD2000/FD4000";
                        break;

                    default:
                        deviceType = cbm_dt_cbm1581;
                        deviceString = "1581";
                        break;
                }
                break;

            case 0x20f0:
                // A DOS 1 2040
                // Values 0xF0 and 0x20 from ROM 901468-07
                deviceType = cbm_dt_cbm2040;
                deviceString = "2040";
                break;

            case 0x32f0:
                deviceType = cbm_dt_cbm3040;
                deviceString = "3040";
                break;

            case 0xc320:
            case 0x20f8:
                deviceType = cbm_dt_cbm4040;
                deviceString = "4040";
                break;

            case 0xf2e9:
                deviceType = cbm_dt_cbm8050;
                deviceString = "8050 dos2.5";
                break;

            case 0xc866:       /* special dos2.7 ?? Speed-DOS 8250 ?? */
            case 0xc611:
                deviceType = cbm_dt_cbm8250;
                deviceString = "8250 dos2.7";
                break;
        }

        rv = 0;

    } while (0);


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

/** @} */
