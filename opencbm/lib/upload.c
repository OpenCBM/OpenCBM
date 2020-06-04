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
#include "archlib.h"

enum { RETRIES_UPLOAD   = 5 }; //!< \brief how many retries to do when communication errors occur on upload
enum { RETRIES_DOWNLOAD = 5 }; //!< \brief how many retries to do when communication errors occur on download

/*! \brief maximum number of byte to upload via M-W (due to floppy buffer size restrictions)
 * The floppy input buffer is located at $0200 - $0229. Thus, the maximum command size is $2A byte.
 * This includes the M-W plus address (2 byte) plus size (1 byte).
 *
 * While one might be tempted do increase the number in order to reduce the number of M-W commands,
 * this is contraproductive: As the 154x/157x cannot cross a page boundary, the number of commands
 * would be the same, but the calculation is more complex.
 *
 * Thus, leave it at 32.
 */
enum { MAX_BYTE_UPLOAD = 32 };

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

/*! \internal \brief Invalidate M-W command

 When an M-W command is sent, but there is a transfer error,
 we cannot just repeat the command, or there will be erroneous
 data written.

 Because of this, this function is used to "invalidate" the
 command in the Floppy RAM before it is executed.

 This is done by flooding the input buffer, which is from $0200-$0229,
 with nonsense data. Because of this, the floppy will reject the command.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \return
   0  on success.
   -1 if the command could not execute because there were subsequent
      transfer errors.

 This function assumes that the cbm_raw_write() failed, but the
 cbm_unlisten() was not executed yet. The unlisten is the trigger for
 the floppy to execute this command. Afterwards, there is nothing we can
 done.
*/

static int
invalidateMWCommand(CBM_FILE HandleDevice)
{
    /* the M-W command was aborted because of an error.
     * It is critical to "invalidate" it, or we will write garbage into
     * the RAM.
     * Because of this, we "flood" the 1541 input buffer, making sure
     * the command is bigger than 0x29 byte, which will result in an error from
     * the floppy, before trying to continue.
     */

    static const unsigned char dummydata[41] = { 0 };
    int retrycounter = RETRIES_UPLOAD;
    int rv = 0;

    do {
        if ( cbm_raw_write(HandleDevice, dummydata, sizeof dummydata) != sizeof dummydata) {
            if (retrycounter-- > 0) {
                continue;
            }
            rv = -1;
        }
        break;
    } while (1);

    retrycounter = RETRIES_UPLOAD;

    if (rv == 0) do {
        if ( cbm_unlisten(HandleDevice) ) {
            if (retrycounter-- > 0) {
                continue;
            }
            rv = -1;
            break;
        }
        break;
    } while (1);

    return rv;
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
   Specifically, -1 is returned on transfer errors.

 If cbm_driver_open() did not succeed, it is illegal to
 call this function.
*/

int CBMAPIDECL
cbm_upload(CBM_FILE HandleDevice, unsigned char DeviceAddress,
           int DriveMemAddress, const void *Program, size_t Size)
{
    const char *bufferToProgram = Program;

    unsigned char command[] = { 'M', '-', 'W', ' ', ' ', ' ' };
    size_t i;
    int rv = 0;
    int c;

    int retrycounter = RETRIES_UPLOAD;

    FUNC_ENTER();

    DBG_ASSERT(sizeof(command) == 6);

    for(i = 0; i < Size; i += c)
    {
        if ( cbm_listen(HandleDevice, DeviceAddress, 15) ) {
            if (retrycounter-- > 0) {
                c = 0;
                continue;
            }
            return -1;
        }

        // Calculate how many bytes are left

        c = Size - i;

        // Do we have more than the maximum number? Then, restrict to maximum

        if (c > MAX_BYTE_UPLOAD)
        {
            c = MAX_BYTE_UPLOAD;
        }

        // The command M-W consists of:
        // M-W <lowaddress> <highaddress> <count>
        // build that command:

        StoreAddressAndCount(&command[3], DriveMemAddress, c);

        // Write the M-W command to the drive...

        if ( cbm_raw_write(HandleDevice, command, sizeof(command)) != sizeof command) {
            if (retrycounter-- > 0) {
                if ( ! invalidateMWCommand(HandleDevice) ) {
                    c = 0;
                    continue;
                }
            }
            rv = -1;
            break;
        }


        // ... as well as the (up to MAX_BYTE_UPLOAD) data bytes

        if ( cbm_raw_write(HandleDevice, bufferToProgram, c) != c ) {
            if (retrycounter-- > 0) {
                if ( ! invalidateMWCommand(HandleDevice) ) {
                    c = 0;
                    continue;
                }
            }
            rv = -1;
            break;
        }

        // The UNLISTEN is the signal for the drive
        // to start execution of the command

        if ( cbm_unlisten(HandleDevice) ) {
            if (retrycounter-- > 0) {
                c = 0;
                continue;
            }
            rv = -1;
            break;
        }
        // Now, advance the pointer into drive memory
        // as well to the program in PC's memory in case we
        // might need to use it again for another M-W command

        DriveMemAddress += c;
        bufferToProgram += c;

        // Advance the return value of send bytes, too.

        rv += c;

        retrycounter = RETRIES_UPLOAD;
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
    unsigned char command[] = { 'M', '-', 'R', ' ', ' ', '\0', '\r' };
    unsigned char *StoreBuffer = Buffer;

    size_t i;
    int rv = 0;
    int readbytes = 0;
    int c;
    int page2workaround = 0;
    int retrycounter = RETRIES_DOWNLOAD;

    FUNC_ENTER();

    DBG_ASSERT(sizeof(command) == 7);

    for(i = 0; i < Size; i += c)
    {
        char dummy;

        // Calculate how much bytes are left
        c = Size - i;

        // Do we have more than 256? Then, restrict to 256
        if (c > TRANSFER_SIZE_DOWNLOAD)
        {
            c = TRANSFER_SIZE_DOWNLOAD;
        }

        /*
         * Workaround: The 154x/157x/1581 drives (and possibly others, too)
         * cannot cross a page boundary on M-R. Thus, make sure we do not
         * cross the boundary.
         */
        if (c + (DriveMemAddress & 0xFF) > 0x100) {
            c = 0x100 - (DriveMemAddress & 0xFF);
        }

        // The command M-R consists of:
        // M-R <lowaddress> <highaddress> <count> '\r'
        // build that command:

        StoreAddressAndCount(&command[3], DriveMemAddress, c);

        // Write the M-R command to the drive...
        if ( cbm_exec_command(HandleDevice, DeviceAddress, command, sizeof(command)) ) {
            if (retrycounter-- > 0) {
                c = 0;
                continue;
            }
            else {
                rv = -1;
                break;
            }
        }


        if ( cbm_talk(HandleDevice, DeviceAddress, 15) ) {
            if (retrycounter-- > 0) {
                c = 0;
                continue;
            }
            else {
                rv = -1;
                break;
            }
        }

        // now read the (up to 256) data bytes
        // and advance the return value of send bytes, too.
        readbytes = cbm_raw_read(HandleDevice, StoreBuffer, c);

        if (readbytes != c) {
            if (retrycounter-- > 0) {
                c = 0;
                continue;
            }
            else {
                rv = -1;
                break;
            }
        }

        // Now, advance the pointer into drive memory
        // as well to the program in PC's memory in case we
        // might need to use it again for another M-W command
        DriveMemAddress += readbytes;
        StoreBuffer     += readbytes;

        rv              += readbytes;

        // skip the trailing CR
        if ( cbm_raw_read(HandleDevice, &dummy, 1) != 1 ) {
            /*
             * if there is no CR, there can be two reasons:
             *
             * 1. an unexpected error occurred. In this case, we
             *    want to quit and report an error
             *
             * 2. we are reading page 2 of the floppy drive's memory
             *    In this case, this error is due to the special
             *    handling of the $02D4 address in the 154x/157x
             *    ($02CF in the 1581). It returns a CR and aborts the
             *    transfer.
             *    If this is the case, restart the transmission for
             *    the rest as a work-around.
             */

            if ( ( (DriveMemAddress & 0xFF00) >> 8 == 2) && page2workaround == 0 ) {
                /* we are on page 2, try the workaround once */
                page2workaround = 1;
                c = readbytes - 1;
                --rv;
                --DriveMemAddress;
                --StoreBuffer;
                continue;
            }
            else {
                if (retrycounter-- > 0) {
                    continue;
                }
                else {
                    rv = -1;
                    break;
                }
            }
        }

        // The UNTALK is the signal for end of transmission
        if ( cbm_untalk(HandleDevice) ) {
            if (retrycounter-- > 0) {
                /*
                 * do NOT set c to 0! We already incremented that 
                 * pointers. Thus, setting it to 0 and re-reading
                 * would result in a buffer overflow!
                 */
                continue;
            }
            else {
                rv = -1;
                break;
            }
        }

        retrycounter = RETRIES_DOWNLOAD;
    }

    FUNC_LEAVE_INT(rv);
}
