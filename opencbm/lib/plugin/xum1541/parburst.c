/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2009      Nate Lawson
 *
 */

/*! ************************************************************** 
** \file lib/plugin/xum1541/WINDOWS/parburst.c \n
** \author Nate Lawson \n
** \version $Id: parburst.c,v 1.3 2010-04-26 04:10:40 natelawson Exp $ \n
** \n
** \brief Shared library / DLL for accessing the mnib driver functions, windows specific code
**
****************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define DBG_USERMODE
#define DBG_PROGNAME "OPENCBM-XUM1541.DLL"
#include "debug.h"

#define OPENCBM_PLUGIN
#include "archlib.h"

#include "xum1541.h"


/*! \brief PARBURST: Read from the parallel port

 This function is a helper function for parallel burst:
 It reads from the parallel port.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \return
   The value read from the parallel port

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

__u_char CBMAPIDECL
opencbm_plugin_parallel_burst_read(CBM_FILE HandleDevice)
{
    __u_char result;

    result = (__u_char)xum1541_ioctl(XUM1541_PARBURST_READ, 0, 0);
    //printf("parburst read: %x\n", result);
    return result;
}

/*! \brief PARBURST: Write to the parallel port

 This function is a helper function for parallel burst:
 It writes to the parallel port.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Value
   The value to be written to the parallel port

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

void CBMAPIDECL
opencbm_plugin_parallel_burst_write(CBM_FILE HandleDevice, __u_char Value)
{
    int result;

    result = xum1541_ioctl(XUM1541_PARBURST_WRITE, Value, 0);
    //printf("parburst write: %x, res %x\n", Value, result);
}

int CBMAPIDECL
opencbm_plugin_parallel_burst_read_n(CBM_FILE HandleDevice, __u_char *Buffer,
    unsigned int Length)
{
    int result;

    result = xum1541_special_read(XUM1541_NIB_READ_N, Buffer, Length);
    if (result != Length) {
        DBG_WARN((DBG_PREFIX "parallel_burst_read_n: returned with error %d", result));
    }

    return result;
}

int CBMAPIDECL
opencbm_plugin_parallel_burst_write_n(CBM_FILE HandleDevice, __u_char *Buffer,
    unsigned int Length)
{
    int result;

    result = xum1541_special_write(XUM1541_NIB_WRITE_N, Buffer, Length);
    if (result != Length) {
        DBG_WARN((DBG_PREFIX "parallel_burst_write_n: returned with error %d", result));
    }

    return result;
}

/*! \brief PARBURST: Read a complete track

 This function is a helper function for parallel burst:
 It reads a complete track from the disk

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer
   Pointer to a buffer which will hold the bytes read.

 \param Length
   The length of the Buffer.

 \return
   != 0 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_parallel_burst_read_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length)
{
    int result;

    result = xum1541_special_read(XUM1541_NIB, Buffer, Length);
    if (result != Length) {
        DBG_WARN((DBG_PREFIX "parallel_burst_read_track: returned with error %d", result));
    }

    return result;
}

/*! \brief PARBURST: Write a complete track

 This function is a helper function for parallel burst:
 It writes a complete track to the disk

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer
   Pointer to a buffer which hold the bytes to be written.

 \param Length
   The length of the Buffer.

 \return
   != 0 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_parallel_burst_write_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length)
{
    int result;

    result = xum1541_special_write(XUM1541_NIB, Buffer, Length);
    if (result != Length) {
        DBG_WARN((DBG_PREFIX "parallel_burst_write_track: returned with error %d", result));
    }

    return result;
}
