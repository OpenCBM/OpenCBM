/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2019 Spiro Trikaliotis
 *
 *  Based on code:
 *  Copyright 1999-2004 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Modifications for cbm4win and general rework Copyright 2001-2007 Spiro Trikaliotis
 *  Additions Copyright 2006,2011 Wolfgang Moser (http://d81.de)
 *  Additions Copyright 2011      Thomas Winkler
 *
*/

/*! **************************************************************
** \file cbmctrl/pport.c \n
** \author Spiro Trikaliotis \n
** \n
** \brief Check XP1541/XP1571 (or similar XUM1541) parallel cable
**
****************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "cbmctrl.h"

#include "arch.h"


/*! \brief \internal Set the PIA back to input mode

 This function sets the parallel port PIA back to input mode.
 This prevents both PC and drive driving the lines, which
 could result in a hardware defect!

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Drive
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param PiaAddress
   The address in the drive's address space where the PIA, VIA
   or CIA output register for the parallel port is suspected.
   This routine assume that at PiaAddress + 2, there is the
   DDR of this port.

 \return
   0 if the drive could be contacted.

 If cbm_driver_open() did not succeed, it is illegal to
 call this function.
*/

static int
pia_to_inputmode(CBM_FILE HandleDevice, unsigned char Drive, unsigned int PiaAddress)
{
    int ret = -1;
    unsigned char byteval = 0x00;

    if (cbm_upload(HandleDevice, Drive, PiaAddress + 2, &byteval, 1) == 1)
        ret = 0;
    else
        ret = 1;

    return ret;
}

/*! \brief Output via floppy PIA and check if that can be read back

 This function outputs some byte via the floppy PIA and checks if
 that value can be read back using the parallel port of the PC.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Drive
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param PiaAddress
   The address in the drive's address space where the PIA, VIA
   or CIA output register for the parallel port is suspected.
   This routine assume that at PiaAddress + 2, there is the
   DDR of this port.

 \param Value
   The value to write into the parallel port for testing.

 \return
   -1 if there was an error setting the vlaue
   Otherwise, the value that was read back.

 If cbm_driver_open() did not succeed, it is illegal to
 call this function.
*/

static int
output_pia(CBM_FILE HandleDevice, unsigned char Drive, unsigned int PiaAddress, unsigned char Value)
{
    int ret = -1;

    do {
        unsigned char byteval = 0xff;

        /*
         * Set the PIA port to output
         */
        if (cbm_upload(HandleDevice, Drive, PiaAddress + 2, &byteval, 1) != 1)
            break;

        /*
         * Output the value via the PIA port
         */
        if (cbm_upload(HandleDevice, Drive, PiaAddress , &Value, 1) != 1)
            break;

        /*
         * Give the drive time to be able to execute this command
         */
        arch_usleep(5000);

        /*
         * Read back the value. Hopefully, it is exactly what we just have written.
         */
        ret = cbm_pp_read(HandleDevice);

    } while (0);

    return ret;
}

// @@@DUMMY
static int
input_pia(CBM_FILE HandleDevice, unsigned char Drive, unsigned int PiaAddress, unsigned char Value)
{
    int ret = -1;

    do {
        unsigned char byteval = 0xff;

        /*
         * Output the value via the parallel port
         */
        cbm_pp_write(HandleDevice, Value);

        /*
         * Give the drive time to be able to execute this command
         */
        arch_usleep(5000);

        /*
         * Read back the value. Hopefully, it is exactly what we just have written.
         */
        if (cbm_download(HandleDevice, Drive, PiaAddress , &byteval, sizeof(byteval)) != sizeof(byteval))
            break;

        ret = byteval;

    } while (0);

    return ret;
}


/*! \brief Check if XP1541 (or similar) cable is working

 This function checks the XP1541 (or similar parallel cable)
 if it works as expected. That is, the connections are tested
 with some test patterns.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param CbmDeviceType
   An enum which holds the type of the device.

 \param CableType
   An enum which holds the cable type of the device

 \param Verbose
   set the != 0 if the output should be verbose

 \remark
   When calling this function, it must be sure that a parallel
   cable exists, or this function will fail.

 \return
   0 if the XP1541 cable works. Othersise, != 0
*/

int
cbm_check_xp1541(CBM_FILE HandleDevice, unsigned char DeviceAddress,
                 enum cbm_device_type_e CbmDeviceType,
                 enum cbm_cable_type_e CableType,
                 int Verbose)
{
    int ret = 0;

    if (CableType != cbm_ct_xp1541)
        return -1;

    do
    {
        unsigned int piaAddress;

        unsigned int pattern_index;
        static const unsigned char testpattern[] =
            {
              0x55, 0xAA, 0x69, 0x96, 0x0f, 0xf0,
              0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
              0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
            };

        /*
         * If needed, determine the type of the drive
         */
        if (CbmDeviceType == cbm_dt_unknown)
        {
            ret = cbm_identify(HandleDevice, DeviceAddress,
                 &CbmDeviceType, NULL);
        }

        if (ret)
        {
            /*
             * We could not even determine the device type;
             * we do not dare trying to determine the cable
             * type, as writing into wrong locations can
             * make the drive hang.
             */
            break;
        }

        piaAddress = cbm_determine_pport_address(CbmDeviceType);
        /*
         * If we do not have a PIA address, we do not know where a parallel
         * cable could be located. This, report "no parallel cable".
         */
        if (piaAddress == 0)
            break;

        /*
         * Set parallel port into input mode.
         * This prevents us (PC) and the drive driving the lines simultaneously.
         */
        cbm_pp_read(HandleDevice);

        ret = 0;

        for (pattern_index = 0; pattern_index < sizeof(testpattern); pattern_index++) {
            /*
             * Try to write some patterns and check if we see them:
             */
            unsigned char value_set = testpattern[pattern_index];
            unsigned int value_read = output_pia(HandleDevice, DeviceAddress, piaAddress, value_set);
            if (value_set != value_read) {
                fprintf(stderr, "error sending FLOPPY -> PC, pattern #%u = 0x%02X, but read back 0x%02X\n",
                        pattern_index, value_set, value_read);
                ++ret;
            }
        }

        /*
         * Set PIA back to input mode.
         * Again, this prevents PC and drive driving the lines simultaneously
         * (in the future).
         */
        if (piaAddress)
        {
            pia_to_inputmode(HandleDevice, DeviceAddress, piaAddress);
        }

        for (pattern_index = 0; pattern_index < sizeof(testpattern); pattern_index++) {
            /*
             * Try to write some patterns and check if we see them:
             */
            unsigned char value_set = testpattern[pattern_index];
            unsigned int value_read = input_pia(HandleDevice, DeviceAddress, piaAddress, value_set);
            if (value_set != value_read) {
                fprintf(stderr, "error sending PC -> FLOPPY, pattern #%u = 0x%02X, but read back 0x%02X\n",
                        pattern_index, value_set, value_read);
                ++ret;
            }
        }

    } while (0);

    return ret;
}
