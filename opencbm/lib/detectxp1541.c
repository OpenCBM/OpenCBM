/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005 Spiro Trikaliotis
 *  Based on code by Wolfgang Moser
 *
*/

/*! ************************************************************** 
** \file lib/detectxp1541.c \n
** \author Spiro Trikaliotis \n
** \version $Id: detectxp1541.c,v 1.1 2005-07-20 16:37:12 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**        Detect an XP1541/XP1571 parallel cable
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


/**/
static int
output_pia(CBM_FILE HandleDevice, unsigned char Drive, unsigned int PiaAddress, unsigned char Value)
{
    int ret = -1;

    FUNC_ENTER();

    do {
        unsigned char byteval = 0xff;

        if (cbm_upload(HandleDevice, Drive, PiaAddress + 2, &byteval, 1) != 1)
            break;

        if (cbm_upload(HandleDevice, Drive, PiaAddress , &Value, 1) != 1)
            break;

        Sleep(10);

        ret = cbm_pp_read(HandleDevice);

    } while (0);

    FUNC_LEAVE_INT(ret);
}

/*! \brief Identify the cable connected to a specific floppy drive.

 This function tries to identify if the given floppy drive has an
 XP1541 cable connected.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param CbmDeviceType
   Pointer to an enum which holds the type of the device.
   If this pointer is NULL or the device type is set to
   unknown, this function calls cbm_identify itself to find
   out the device type. If this pointer is not set to NULL,
   this function will return the device type there.

 \param CableType
   Pointer to an enum which will hold the cable type of the 
   device on return.

 \return
   0 if the drive could be contacted. It does not mean that
   the device could be identified.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_identify_xp1541(CBM_FILE HandleDevice, __u_char DeviceAddress,
                    enum cbm_device_type_e *CbmDeviceType,
                    enum cbm_cable_type_e *CableType)
{
    int xp15x1 = 0;
    int ret = 0;
    enum device_type_e localDummyDeviceType = cbm_dt_unknown;

    FUNC_ENTER();
 
    do
    {
        unsigned int piaAddress;
        int value;

        if (!CableType)
        {
            ret = 1;
            break;
        }

        *CableType = cbm_ct_none;

        if (!CbmDeviceType)
            CbmDeviceType = &localDummyDeviceType;

        if (*CbmDeviceType == cbm_dt_unknown)
        {
            ret = cbm_identify(HandleDevice, DeviceAddress,
                 CbmDeviceType, NULL);
        }

        if (ret)
        {
            *CableType = cbm_ct_unknown;
            break;
        }

        switch (*CbmDeviceType)
        {
        case cbm_dt_cbm1541:
            piaAddress = 0x1801;
            break;

        case cbm_dt_cbm1570:
        case cbm_dt_cbm1571:
            piaAddress = 0x4001;
            break;

        default:
            piaAddress = 0;
            break;
        }

        if (piaAddress == 0)
            break;

        // Set parallel port into input mode
        value = cbm_pp_read(HandleDevice);
        DBG_PRINT((DBG_PREFIX "Setting Parport into input mode", value));

        value = output_pia(HandleDevice, DeviceAddress, piaAddress, 0x55);
        DBG_PRINT((DBG_PREFIX "Reading after setting $55: $%02x", value));
        if (value != 0x55)
            break;

        value = output_pia(HandleDevice, DeviceAddress, piaAddress, 0xAA);
        DBG_PRINT((DBG_PREFIX "Reading after setting $AA: $%02x", value));
        if (value != 0xaa)
            break;

        *CableType = cbm_ct_xp1541;

    } while (0);

    FUNC_LEAVE_INT(ret);
}
