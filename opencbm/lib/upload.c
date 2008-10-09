/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005,2008 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/upload.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: upload.c,v 1.8 2008-10-09 17:14:26 strik Exp $ \n
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

/*! \internal \brief Write a 8 bit value into the buffer

 This function writes a 8 bit value into the given buffer.

 \param Buffer
   A pointer to the memory address where the address and the byte
   count are to be written to.

 \param Value
   The 8 bit value to be written.
*/

static __inline void StoreInt8IntoBuffer(unsigned char * Buffer, unsigned int Value)
{
    DBG_ASSERT(Buffer != NULL);
    *Buffer = (unsigned char) Value;
}

/*! \internal \brief Write a 16 bit value into the buffer

 This function writes a 16 bit value into the given buffer.
 The value is written in low endian, that is, the low byte first.

 \param Buffer
   A pointer to the memory address where the address and the byte
   count are to be written to.

 \param Value
   The 16 bit value to be written.
*/

static __inline void StoreInt16IntoBuffer(unsigned char * Buffer, unsigned int Value)
{
    DBG_ASSERT(Buffer != NULL);
    StoreInt8IntoBuffer(Buffer++, Value % 256);
    StoreInt8IntoBuffer(Buffer,   Value / 256);
}

/*! \internal \brief Write an address and a count number into memory

 This function is used to write the drive address and the byte count
 for the "M-W" and "M-R" command.

 \param Buffer
   A pointer to the memory address where the address and the byte
   count are to be written to.

 \param DriveMemAddress
   The address in the drive's memory where the program is to be
   stored.
   
 \param ByteCount
   The number of bytes to be transferred.
*/

static __inline void StoreAddressAndCount(unsigned char * Buffer, unsigned int DriveMemAddress, unsigned int ByteCount)
{
    StoreInt16IntoBuffer(Buffer, DriveMemAddress);
    StoreInt8IntoBuffer(Buffer + 2, ByteCount);
}

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

    unsigned char command[] = { 'M', '-', 'W', ' ', ' ', ' ' };
    size_t i;
    int rv = 0;
    int c;

    FUNC_ENTER();

    DBG_ASSERT(sizeof(command) == 6);

    for(i = 0; i < Size; i += 32)
    {
        cbm_listen(HandleDevice, DeviceAddress, 15);

        // Calculate how many bytes are left

        c = Size - i;

        // Do we have more than 32? Then, restrict to 32

        if (c > 32)
        {
            c = 32;
        }

        // The command M-W consists of:
        // M-W <lowaddress> <highaddress> <count>
        // build that command:

        StoreAddressAndCount(&command[3], DriveMemAddress, c);

        // Write the M-W command to the drive...

        cbm_raw_write(HandleDevice, command, sizeof(command));

        // ... as well as the (up to 32) data bytes

        cbm_raw_write(HandleDevice, bufferToProgram, c);

        // Now, advance the pointer into drive memory
        // as well to the program in PC's memory in case we
        // might need to use it again for another M-W command

        DriveMemAddress += c;
        bufferToProgram += c;

        // Advance the return value of send bytes, too.

        rv += c;

        // The UNLISTEN is the signal for the drive 
        // to start execution of the command

        cbm_unlisten(HandleDevice);
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

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbm_download(CBM_FILE HandleDevice, __u_char DeviceAddress, 
             int DriveMemAddress, void *const Buffer, size_t Size)
{
    unsigned char command[] = { 'M', '-', 'R', ' ', ' ', '\0', '\r' };
    unsigned char *StoreBuffer = Buffer;

    size_t i;
    int rv = 0;
    int c;

    FUNC_ENTER();

    DBG_ASSERT(sizeof(command) == 7);

    for(i = 0; i < Size; i += 0x100)
    {
        char dummy;

        // Calculate how much bytes are left
        c = Size - i;

        // Do we have more than 256? Then, restrict to 256
        if (c > 0x100)
        {
            c = 0x100;
        }

        // The command M-R consists of:
        // M-R <lowaddress> <highaddress> <count> '\r'
        // build that command:

        StoreAddressAndCount(&command[3], DriveMemAddress, c);

        // Write the M-R command to the drive...
        cbm_exec_command(HandleDevice, DeviceAddress, command, sizeof(command));

        cbm_talk(HandleDevice, DeviceAddress, 15);

        // now read the (up to 256) data bytes
        // and advance the return value of send bytes, too.
        rv += cbm_raw_read(HandleDevice, StoreBuffer, c);

        // Now, advance the pointer into drive memory
        // as well to the program in PC's memory in case we
        // might need to use it again for another M-W command
        DriveMemAddress += c;
        StoreBuffer     += c;

        // skip the trailing CR
        cbm_raw_read(HandleDevice, &dummy, 1);

        // The UNTALK is the signal for end of transmission
        cbm_untalk(HandleDevice);
    }

    FUNC_LEAVE_INT(rv);
}
