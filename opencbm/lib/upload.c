/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005,2008,2020 Spiro Trikaliotis
 *
*/

/*! **************************************************************
** \file lib/upload.c \n
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

/*! \brief Upload a program into a floppy's drive memory.

 This function writes a program into the drive's memory
 via use of "M-W" commands.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param DriveMemAddress
   The address in the drive's memory where the program is to be
   stored.

 \param Program
   Pointer to a byte buffer which holds the program in the
   caller's address space.

 \param Size
   The size of the program to be stored, in bytes.

 \return
   Returns the number of bytes written into program memory.
   If it does not equal Size, than an error occurred.
   Specifically, -1 is returned on transfer errors.

 If cbm_driver_open() did not succeed, it is illegal to
 call this function.
*/

int CBMAPIDECL
cbm_upload(CBM_FILE HandleDevice, unsigned char DeviceAddress,
           int DriveMemAddress, const void *Program, size_t Size)
{
    int rv;

    FUNC_ENTER();

    rv = cbm_dos_memory_write(HandleDevice, DeviceAddress, DriveMemAddress, Size, Program, NULL, NULL);

    if (rv == 0) {
        rv = Size;
    }

    FUNC_LEAVE_INT(rv);
}

/*! \brief Download data from a floppy's drive memory.

 This function reads data from the drive's memory via
 use of "M-R" commands.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param DriveMemAddress
   The address in the drive's memory where the program is to be
   stored.

 \param Buffer
   Pointer to a byte buffer where the data from the drive's
   memory is stored.

 \param Size
   The size of the data block to be stored, in bytes.

 \return
   Returns the number of bytes written into the storage buffer.
   If it does not equal Size, than an error occurred.
   Specifically, -1 is returned on transfer errors.

 If cbm_driver_open() did not succeed, it is illegal to
 call this function.
*/

enum { TRANSFER_SIZE_DOWNLOAD = 0x100u };

int CBMAPIDECL
cbm_download(CBM_FILE HandleDevice, unsigned char DeviceAddress,
             int DriveMemAddress, void *const Buffer, size_t Size)
{
    int rv;

    FUNC_ENTER();

    rv = cbm_dos_memory_read(HandleDevice, Buffer, Size, DeviceAddress, DriveMemAddress, Size, NULL, NULL);

    if (rv == 0) {
        rv = Size;
    }

    FUNC_LEAVE_INT(rv);
}
