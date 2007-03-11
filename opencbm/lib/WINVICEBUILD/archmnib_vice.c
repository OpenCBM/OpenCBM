/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005      Tim Schürmann
 *  Copyright 2005,2007 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINVICEBUILD/archmnib_vice.c \n
** \author Tim Schürmann, Spiro Trikaliotis \n
** \version $Id: archmnib_vice.c,v 1.4.2.1 2007-03-11 13:46:03 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the mnib driver functions, windows specific code for VICE emulation
**
****************************************************************/

#include <windows.h>
#include <windowsx.h>

#include <mmsystem.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM.DLL"

#include "debug.h"

#include <winioctl.h>
#include "cbmioctl.h"

#include <stdlib.h>

//! mark: We are building the DLL */
#define DLL
#include "i_opencbm.h"
#include "archlib.h"


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
cbmarch_parallel_burst_read(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    FUNC_LEAVE_UCHAR(0);
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
cbmarch_parallel_burst_write(CBM_FILE HandleDevice, __u_char Value)
{
    FUNC_ENTER();

    FUNC_LEAVE();
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
   0 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbmarch_parallel_burst_read_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length)
{
    FUNC_ENTER();

    FUNC_LEAVE_INT(0);
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
   0 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbmarch_parallel_burst_write_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length)
{
    FUNC_ENTER();

    FUNC_LEAVE_INT(0);
}
