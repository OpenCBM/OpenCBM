/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 *  Copyright 2001-2005 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/upload.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: upload.c,v 1.1 2005-02-13 17:58:12 strik Exp $ \n
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


/*-------------------------------------------------------------------*/
/*--------- HELPER FUNCTIONS ----------------------------------------*/


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

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbm_upload(CBM_FILE HandleDevice, __u_char DeviceAddress, 
           int DriveMemAddress, const void *Program, size_t Size)
{
    const char *bufferToProgram = Program;

    USHORT c;
    USHORT i;
    USHORT rv = 0;
    UCHAR command[] = { 'M', '-', 'W', ' ', ' ', ' ' };

    FUNC_ENTER();

    DBG_ASSERT(sizeof(command) == 6);

    for(i = 0; i < Size; i += 32)
    {
        cbmarch_listen(HandleDevice, DeviceAddress, 15);

        // Calculate how much bytes are left

        c = Size - i;

        // Do we have more than 32? Than, restrict to 32

        if (c > 32)
        {
            c = 32;
        }

        // The command M-W consists of:
        // M-W <lowaddress> <highaddress> <count>
        // build that command:

        command[3]=(UCHAR) (DriveMemAddress % 256);
        command[4]=(UCHAR) (DriveMemAddress / 256);
        command[5]=(UCHAR) c; 

        // Write the M-W command to the drive...

        cbmarch_raw_write(HandleDevice, command, sizeof(command));

        // ... as well as the (up to 32) data bytes

        cbmarch_raw_write(HandleDevice, bufferToProgram, c);

        // Now, advance the pointer into drive memory
        // as well to the program in PC's memory in case we
        // might need to use it again for another M-W command

        DriveMemAddress += c;
        bufferToProgram += c;

        // Advance the return value of send bytes, too.

        rv += c;

        // The UNLISTEN is the signal for the drive 
        // to start execution of the command

        cbmarch_unlisten(HandleDevice);
    }

    FUNC_LEAVE_INT(rv);
}
