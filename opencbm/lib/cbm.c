/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2004 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 *  Copyright 2001-2004 Spiro Trikaliotis
 *
 *  Parts are Copyright
 *      Jouko Valta <jopi@stekt.oulu.fi>
 *      Andreas Boose <boose@linux.rz.fh-hannover.de>
*/

/*! ************************************************************** 
** \file lib/cbm.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: cbm.c,v 1.1 2004-12-22 14:43:21 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**
****************************************************************/

// #define UNICODE 1

#include <windows.h>
#include <windowsx.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! Mark: We are building the DLL */
#define DBG_DLL

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM.DLL"

/*! This file is "like" debug.c, that is, define some variables */
#define DBG_IS_DEBUG_C

#include "debug.h"

#include <winioctl.h>
#include "cbmioctl.h"

#include <stdlib.h>

//! mark: We are building the DLL */
#define DLL
#include "i_opencbm.h"


// #define DBG_DUMP_RAW_READ
// #define DBG_DUMP_RAW_WRITE

/*-------------------------------------------------------------------*/
/*--------- DEBUGGING FUNCTIONS -------------------------------------*/

#if DBG
static void 
memdump(IN const char *Where, IN const unsigned char *InputBuffer, IN const size_t Count)
{
    unsigned i;
    char outputBufferChars[17];
    char outputBuffer[100];
    char *p;

    p = outputBuffer;

    DBG_PRINT((DBG_PREFIX "%s: (0x%04x)", Where, (unsigned) Count));

    for (i=0; i<Count; i++) 
    {
        p += sprintf(p, "%02x ", (unsigned) InputBuffer[i]);

        if (i % 16 == 7)
        {
            p += sprintf(p, "- ");
        }

        outputBufferChars[i % 16] = isprint(InputBuffer[i]) ? InputBuffer[i] : '.';

        if (i % 16 == 15)
        {
            outputBufferChars[(i % 16) + 1] = 0;
            DBG_PRINT((DBG_PREFIX "%04x: %-50s  %s", (unsigned) i & 0xfff0, outputBuffer, outputBufferChars));
            p = outputBuffer;
        }
    }

    if (i % 16 != 0)
    {
        outputBufferChars[i % 16] = 0;
        DBG_PRINT((DBG_PREFIX "%04x: %-50s  %s", (unsigned) i & 0xfff0, outputBuffer, outputBufferChars));
    }
}
#endif // #if DBG

/*-------------------------------------------------------------------*/
/*--------- DRIVER HANDLING -----------------------------------------*/

/*! \brief Get the name of the driver for a specific parallel port

 Get the name of the driver for a specific parallel port.

 \param PortNumber

   The port number for the driver to open. 0 means "default" driver, while
   values != 0 enumerate each driver.

 \return 
   Returns a pointer to a null-terminated string containing the
   driver name, or NULL if an error occurred.

 \bug
   PortNumber is not allowed to exceed 10. 
*/

const char * CBMAPIDECL
cbm_get_driver_name(int PortNumber)
{
    FUNC_ENTER();

    FUNC_LEAVE_STRING(cbm_i_get_driver_name(PortNumber));
}

/*! \brief Opens the driver

 This function Opens the driver.

 \param HandleDevice
  
   Pointer to a CBM_FILE which will contain the file handle of the driver.

 \param PortNumber

   The port number of the driver to open. 0 means "default" driver, while
   values != 0 enumerate each driver.

 \return 

   ==0: This function completed successfully
   !=0: otherwise

 PortNumber is not allowed to exceed 10. 

 cbm_driver_open() should be balanced with cbm_driver_close().
*/

int CBMAPIDECL 
cbm_driver_open(CBM_FILE *HandleDevice, int PortNumber)
{
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbm_i_driver_open(HandleDevice, PortNumber));
}

/*! \brief Closes the driver

 Closes the driver, which has be opened with cbm_driver_open() before.

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 cbm_driver_close() should be called to balance a previous call to
 cbm_driver_open(). 
 
 If cbm_driver_open() did not succeed, it is illegal to 
 call cbm_driver_close().
*/

void CBMAPIDECL
cbm_driver_close(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    cbm_i_driver_close(HandleDevice);

    FUNC_LEAVE();
}

/*-------------------------------------------------------------------*/
/*--------- BASIC I/O -----------------------------------------------*/

/*! \brief Write data to the IEC serial bus

 This function sends data after a cbm_listen().

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer

   Pointer to a buffer which hold the bytes to write to the bus.

 \param Count

   Number of bytes to be written.

 \return

   >= 0: The actual number of bytes written. 
   <0  indicates an error.

 This function tries to write Count bytes. Anyway, if an error
 occurs, this function can stop prematurely.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_raw_write(CBM_FILE HandleDevice, const void *Buffer, size_t Count)
{
    DWORD BytesWritten;

    FUNC_ENTER();

#ifdef DBG_DUMP_RAW_WRITE
    memdump("cbm_raw_write", Buffer, Count);
#endif

    WriteFile(
        HandleDevice,
        Buffer,
        Count,
        &BytesWritten,
        NULL
        );

    FUNC_LEAVE_INT(BytesWritten);
}


/*! \brief Read data from the IEC serial bus

 This function retrieves data after a cbm_talk().

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer

   Pointer to a buffer which will hold the bytes read.

 \param Count

   Number of bytes to be read at most.

 \return

   >= 0: The actual number of bytes read. 
   <0  indicates an error.

 At most Count bytes are read.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_raw_read(CBM_FILE HandleDevice, void *Buffer, size_t Count)
{
    DWORD bytesToRead = Count;
    DWORD bytesRead;

    FUNC_ENTER();

    ReadFile(
        HandleDevice,
        Buffer,
        bytesToRead,
        &bytesRead,
        NULL
        );

#ifdef DBG_DUMP_RAW_READ
    memdump("cbm_raw_read", Buffer, bytesRead);
#endif

    FUNC_LEAVE_INT(bytesRead);
}

/*! \brief Send a LISTEN on the IEC serial bus

 This function sends a LISTEN on the IEC serial bus.
 This prepares a LISTENer, so that it will wait for our
 bytes we will write in the future.

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param SecondaryAddress

   The secondary address for the device on the IEC serial bus.

 \return

   0 means success, else failure

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_listen(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
{
    CBMT_LISTEN_IN parameter;
    int returnValue;

    FUNC_ENTER();

    parameter.PrimaryAddress = DeviceAddress;
    parameter.SecondaryAddress = SecondaryAddress;

    returnValue = cbm_ioctl(HandleDevice, CBMCTRL(LISTEN), &parameter, sizeof(parameter), NULL, 0)
        ? 0 : 1;

    FUNC_LEAVE_INT(returnValue);
}

/*! \brief Send a TALK on the IEC serial bus

 This function sends a TALK on the IEC serial bus.
 This prepares a TALKer, so that it will prepare to send
 us some bytes in the future.

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param SecondaryAddress

   The secondary address for the device on the IEC serial bus.

 \return

   0 means success, else failure

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_talk(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
{
    CBMT_TALK_IN parameter;
    int returnValue;

    FUNC_ENTER();

    parameter.PrimaryAddress = DeviceAddress;
    parameter.SecondaryAddress = SecondaryAddress;

    returnValue = cbm_ioctl(HandleDevice, CBMCTRL(TALK), &parameter, sizeof(parameter), NULL, 0)
        ? 0 : 1;

    FUNC_LEAVE_INT(returnValue);
}

/*! \brief Open a file on the IEC serial bus

 This function opens a file on the IEC serial bus.

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param SecondaryAddress

   The secondary address for the device on the IEC serial bus.

 \param Filename

   The filename of the file to be opened

 \param FilenameLength

   The size of the Filename. If zero, the Filename has to be
   a null-terminated string.

 \return

   0 means success, else failure

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_open(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress, 
         const void *Filename, size_t FilenameLength)
{
    CBMT_OPEN_IN parameter;
    int returnValue;

    FUNC_ENTER();

    parameter.PrimaryAddress = DeviceAddress;
    parameter.SecondaryAddress = SecondaryAddress;

    if (cbm_ioctl(HandleDevice, CBMCTRL(OPEN), &parameter, sizeof(parameter), NULL, 0))
    {
        returnValue = 0;

        if(Filename != NULL)
        {
            if (FilenameLength == 0)
            {
                DBG_WARN((DBG_PREFIX "*** FilenameLength of 0 encountered!"));
                FilenameLength = strlen(Filename);
            }

            if (FilenameLength > 0)
            {
                returnValue = 
                    (size_t) (cbm_raw_write(HandleDevice, Filename, FilenameLength))
                    != FilenameLength;
            }
            cbm_unlisten(HandleDevice);
        }
    }
    else
    {
        returnValue = -1;
    }

    FUNC_LEAVE_INT(returnValue);
}

/*! \brief Close a file on the IEC serial bus

 This function closes a file on the IEC serial bus.

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param SecondaryAddress

   The secondary address for the device on the IEC serial bus.

 \return

   0 on success, else failure

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbm_close(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
{
    CBMT_CLOSE_IN parameter;
    int returnValue;

    FUNC_ENTER();

    parameter.PrimaryAddress = DeviceAddress;
    parameter.SecondaryAddress = SecondaryAddress;

    returnValue = 
        cbm_ioctl(HandleDevice, CBMCTRL(CLOSE), &parameter, sizeof(parameter), NULL, 0)
        ? 0 : 1;

    FUNC_LEAVE_INT(returnValue);
}

/*! \brief Send an UNLISTEN on the IEC serial bus

 This function sends an UNLISTEN on the IEC serial bus.
 Other than LISTEN and TALK, an UNLISTEN is not directed
 to just one device, but to all devices on that IEC
 serial bus. 

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \return

   0 on success, else failure

 At least on a 1541 floppy drive, an UNLISTEN also undoes
 a previous TALK.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbm_unlisten(CBM_FILE HandleDevice)
{
    int returnValue;

    FUNC_ENTER();

    returnValue = cbm_ioctl(HandleDevice, CBMCTRL(UNLISTEN), NULL, 0, NULL, 0) ? 0 : 1;

    FUNC_LEAVE_INT(returnValue);
}

/*! \brief Send an UNTALK on the IEC serial bus

 This function sends an UNTALK on the IEC serial bus.
 Other than LISTEN and TALK, an UNTALK is not directed
 to just one device, but to all devices on that IEC
 serial bus. 

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \return

   0 on success, else failure

 At least on a 1541 floppy drive, an UNTALK also undoes
 a previous LISTEN.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbm_untalk(CBM_FILE HandleDevice)
{
    int returnValue;

    FUNC_ENTER();

    returnValue = cbm_ioctl(HandleDevice, CBMCTRL(UNTALK), NULL, 0, NULL, 0) ? 0 : 1;

    FUNC_LEAVE_INT(returnValue);
}


/*! \brief Get EOI flag after bus read

 This function gets the EOI ("End of Information") flag 
 after reading the IEC serial bus.

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \return

   != 0 if EOI was signalled, else 0.

 If a previous read returned less than the specified number
 of bytes, there are two possible reasons: Either an error
 occurred on the IEC serial bus, or an EOI was signalled.
 To find out the cause, check with this function.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_get_eoi(CBM_FILE HandleDevice)
{
    CBMT_GET_EOI_OUT result;

    FUNC_ENTER();

    cbm_ioctl(HandleDevice, CBMCTRL(GET_EOI), NULL, 0, &result, sizeof(result));

    FUNC_LEAVE_INT(result.Decision);
}

/*! \brief Reset the EOI flag

 This function resets the EOI ("End of Information") flag
 which might be still set after reading the IEC serial bus.

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \return

   0 on success, != 0 means an error has occured.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL 
cbm_clear_eoi(CBM_FILE HandleDevice)
{
    int returnValue;

    FUNC_ENTER();

    returnValue = cbm_ioctl(HandleDevice, CBMCTRL(CLEAR_EOI), NULL, 0, NULL, 0);

    FUNC_LEAVE_INT(returnValue);
}

/*! \brief RESET all devices

 This function performs a hardware RESET of all devices on
 the IEC serial bus.

 \param HandleDevice
  
   A CBM_FILE which contains the file handle of the driver.

 \return

   0 on success, else failure

 Don't overuse this function! Normally, an initial RESET
 should be enough.

 Control is returned after a delay which ensures that all
 devices are ready again.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbm_reset(CBM_FILE HandleDevice)
{
    USHORT returnValue;

    FUNC_ENTER();

    returnValue = cbm_ioctl(HandleDevice, CBMCTRL(RESET), NULL, 0, NULL, 0) ? 0 : 1;

    FUNC_LEAVE_INT(returnValue);
}


/*-------------------------------------------------------------------*/
/*--------- LOW-LEVEL PORT ACCESS -----------------------------------*/

/*! \brief Read a byte from a XP1541/XP1571 cable

 This function reads a single byte from the parallel portion of 
 an XP1541/1571 cable.

 \param HandleDevice

   A CBM_FILE which contains the file handle of the driver.

 \return

   the byte which was received on the parallel port

 This function reads the current state of the port. No handshaking
 is performed at all.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

__u_char CBMAPIDECL 
cbm_pp_read(CBM_FILE HandleDevice)
{
    CBMT_PP_READ_OUT result;

    FUNC_ENTER();

    cbm_ioctl(HandleDevice, CBMCTRL(PP_READ), NULL, 0, &result, sizeof(result));

    FUNC_LEAVE_UCHAR(result.Byte);
}

/*! \brief Write a byte to a XP1541/XP1571 cable

 This function writes a single byte to the parallel portion of 
 a XP1541/1571 cable.

 \param HandleDevice

   A CBM_FILE which contains the file handle of the driver.

 \param Byte

   the byte to be output on the parallel port

 This function just writes on the port. No handshaking
 is performed at all.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

void CBMAPIDECL 
cbm_pp_write(CBM_FILE HandleDevice, __u_char Byte)
{
    CBMT_PP_WRITE_IN parameter;

    FUNC_ENTER();

    parameter.Byte = (UCHAR) Byte;

    cbm_ioctl(HandleDevice, CBMCTRL(PP_WRITE), &parameter, sizeof(parameter), NULL, 0);

    FUNC_LEAVE();
}

/*! \brief Read status of all bus lines.

 This function reads the state of all lines on the IEC serial bus.

 \param HandleDevice

   A CBM_FILE which contains the file handle of the driver.

 \return

   The state of the lines. The result is an OR between
   the bit flags IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 This function just reads the port. No handshaking
 is performed at all.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

int CBMAPIDECL
cbm_iec_poll(CBM_FILE HandleDevice)
{
    CBMT_IEC_POLL_OUT result;

    FUNC_ENTER();

    cbm_ioctl(HandleDevice, CBMCTRL(IEC_POLL), NULL, 0, &result, sizeof(result));

    FUNC_LEAVE_INT(result.Line);
}


/*! \brief Activate a line on the IEC serial bus

 This function activates (sets to 0V) a line on the IEC serial bus.

 \param HandleDevice

   A CBM_FILE which contains the file handle of the driver.

 \param Line

   The line to be activated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

void CBMAPIDECL
cbm_iec_set(CBM_FILE HandleDevice, int Line)
{
    CBMT_IEC_SET_IN parameter;

    FUNC_ENTER();
 
    parameter.Line = (UCHAR) Line;

    cbm_ioctl(HandleDevice, CBMCTRL(IEC_SET), &parameter, sizeof(parameter), NULL, 0);

    FUNC_LEAVE();
}

/*! \brief Deactivate a line on the IEC serial bus

 This function deactivates (sets to 5V) a line on the IEC serial bus.

 \param HandleDevice

   A CBM_FILE which contains the file handle of the driver.

 \param Line

   The line to be deactivated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

void CBMAPIDECL
cbm_iec_release(CBM_FILE HandleDevice, int Line)
{
    CBMT_IEC_RELEASE_IN parameter;

    FUNC_ENTER();

    parameter.Line = (UCHAR) Line;

    cbm_ioctl(HandleDevice, CBMCTRL(IEC_RELEASE), &parameter, sizeof(parameter), NULL, 0);

    FUNC_LEAVE();
}

/*! \brief Wait for a line to have a specific state

 This function waits for a line to enter a specific state
 on the IEC serial bus.

 \param HandleDevice

   A CBM_FILE which contains the file handle of the driver.

 \param Line
   The line to be deactivated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 \param State
   If zero, then wait for this line to be deactivated. \n
   If not zero, then wait for this line to be activated.

 \return
   The state of the IEC bus on return (like cbm_iec_poll).

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

int CBMAPIDECL
cbm_iec_wait(CBM_FILE HandleDevice, int Line, int State)
{
    CBMT_IEC_WAIT_IN parameter;
    CBMT_IEC_WAIT_OUT result;

    FUNC_ENTER();

    parameter.Line = (UCHAR) Line;
    parameter.State = (UCHAR) State;

    cbm_ioctl(HandleDevice, CBMCTRL(IEC_WAIT), 
        &parameter, sizeof(parameter), 
        &result, sizeof(result));

    FUNC_LEAVE_INT(result.Line);
}

/*! \brief Get the (logical) state of a line on the IEC serial bus

 This function gets the (logical) state of a line on the IEC serial bus.

 \param HandleDevice

   A CBM_FILE which contains the file handle of the driver.

 \param Line

   The line to be tested. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 \return

   1 if the line is set, 0 if it is not

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

int CBMAPIDECL
cbm_iec_get(CBM_FILE HandleDevice, int Line)
{
    FUNC_ENTER();
    FUNC_LEAVE_INT((cbm_iec_poll(HandleDevice)&Line) != 0 ? 1 : 0);
}


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
        cbm_listen(HandleDevice, DeviceAddress, 15);

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

/*! \brief Read the drive status from a floppy

 This function reads the drive status of a connected
 floppy drive.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param Buffer
   Pointer to a buffer which will hold the drive's status after
   successfull calling,

 \param BufferLength
   The length of the buffer pointed to by Buffer in bytes.

 \return
   Returns the int representation of the drive status,
   that is, the numerical value of the first return
   value from the drive. This is the error number.

 This function will never write more than BufferLength bytes.
 Nevertheless, the buffer will always be terminated with
 a trailing zero.

 If an error occurs, this function returns a
 "99, DRIVER ERROR,00,00\r" and the value 99.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbm_device_status(CBM_FILE HandleDevice, __u_char DeviceAddress, 
                  void *Buffer, size_t BufferLength)
{
    int retValue;

    FUNC_ENTER();

    DBG_ASSERT(Buffer && (BufferLength > 0));

    // Pre-occupy return value

    retValue = 99;

    if (Buffer && (BufferLength > 0))
    {
        char *bufferToWrite = Buffer;

        // make sure we have a trailing zero at the end of the buffer:

        bufferToWrite[--BufferLength] = '\0';

        // pre-occupy buffer with the error value

        strncpy(bufferToWrite, "99, DRIVER ERROR,00,00\r", BufferLength);

        // Now, ask the drive for its error status:

        if (cbm_talk(HandleDevice, DeviceAddress, 15) == 0)
        {
            unsigned bytesRead = cbm_raw_read(HandleDevice, bufferToWrite, BufferLength);

            DBG_ASSERT(bytesRead >= 0);
            DBG_ASSERT(bytesRead <= BufferLength);

            // make sure we have a trailing zero at the end of the status:

            bufferToWrite[bytesRead] = '\0';

            cbm_untalk(HandleDevice);
        }

        retValue = atoi(bufferToWrite);
    }

    FUNC_LEAVE_INT(retValue);
}

/*! \brief Executes a command in the floppy drive.

 This function Executes a command in the connected floppy drive.

 \param HandleDevice

   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param Command

   Pointer to a string which holds the command to be executed.

 \param Size

   The length of the command in bytes. If zero, the Command
   has to be a null-terminated string.

 \return

   0 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
cbm_exec_command(CBM_FILE HandleDevice, __u_char DeviceAddress, 
                 const void *Command, size_t Size)
{
    int rv;

    FUNC_ENTER();
    rv = cbm_listen(HandleDevice, DeviceAddress, 15);
    if(rv == 0) {
        if(Size == 0) {
            Size = (USHORT) strlen(Command);
        }
        rv = (size_t) cbm_raw_write(HandleDevice, Command, Size) != Size;
        cbm_unlisten(HandleDevice);
    }

    FUNC_LEAVE_INT(rv);
}
/**/

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

/*-------------------------------------------------------------------*/
/*--------- ASCII <-> PETSCII CONVERSION FUNCTIONS ------------------*/
/*
 * 
 *  These functions are taken from VICE's charset.c,
 *  Copyright
 *      Jouko Valta <jopi@stekt.oulu.fi>
 *      Andreas Boose <boose@linux.rz.fh-hannover.de>
 *
 *  You can get VICE from http://www.viceteam.org/
 */

/*! \brief Convert a PETSCII character to ASCII

 This function converts a character in PETSCII
 to ASCII.

 \param Character
   The character value to be converted in PETSCII

 \return
   Returns the value of character in ASCII if it
   can be displayed, a dot (".") otherwise.
*/

char CBMAPIDECL 
cbm_petscii2ascii_c(char Character)
{
    switch (Character & 0xff) {
      case 0x0a:
      case 0x0d:
          return '\n';
      case 0x40:
      case 0x60:
        return Character;
      case 0xa0:                                /* CBM: Shifted Space */
      case 0xe0:
        return ' ';
      default:
        switch (Character & 0xe0) {
          case 0x40: /* 41 - 7E */
          case 0x60:
            return (Character ^ 0x20);

          case 0xc0: /* C0 - DF */
            return (Character ^ 0x80);

      }
    }

    return ((isprint(Character) ? Character : '.'));
}


/*! \brief Convert an ASCII character to PETSCII

 This function converts a character in ASCII
 to PETSCII.

 \param Character
   The character value to be converted in ASCII

 \return
   Returns the value of character in PETSCII.
*/

char CBMAPIDECL
cbm_ascii2petscii_c(char Character)
{
    if ((Character >= 0x5b) && (Character <= 0x7e))
    {
        return Character ^ 0x20;
    }
    else if ((Character >= 'A') && (Character <= 'Z'))          /* C0 - DF */
    {
        return Character | 0x80;
    }
    return Character;
}


/*! \brief Convert an null-termined PETSCII string to ASCII

 This function converts a string in PETSCII
 to ASCII.

 \param Str
   Pointer to a buffer which holds a null-termined string
   in PETCII.

 \return
   Returns a pointer to the Str itself, converted to ASCII.

 If some character cannot be printer on the PC, they are
 replaced with a dot (".").
*/

char * CBMAPIDECL
cbm_petscii2ascii(char *Str)
{
    unsigned char *p;
    for (p = Str; *p; p++) 
    {
        *p = (unsigned char) cbm_petscii2ascii_c(*p);
    }
    return Str;
}

/*! \brief Convert an null-termined ASCII string to PETSCII

 This function converts a string in ASCII
 to PETSCII.

 \param Str
   Pointer to a buffer which holds a null-termined string
   in ASCII.

 \return
   Returns a pointer to the Str itself, converted to PETSCII.
*/

char * CBMAPIDECL 
cbm_ascii2petscii(char *Str)
{
    unsigned char *p;
    for (p = Str; *p; p++)
    {
        *p = (unsigned char) cbm_ascii2petscii_c(*p);
    }
    return Str;
}

/*! \brief DLL initialization und unloading

 This function is called whenever the DLL is loaded or unloaded.
 It ensures that the driver is loaded to be able to call its
 functions.

 \param Module
   Handle of the module; this is not used.

 \param Reason
   DLL_PROCESS_ATTACH if the DLL is loaded,
   DLL_PROCESS_DETACH if it is unloaded.

 \param Reserved
   Not used.

 \return 
   Returns TRUE on success, else FALSE.

 If this function returns FALSE, windows reports that loading the DLL
 was not successful. If the DLL is linked statically, the executable
 refuses to load with STATUS_DLL_INIT_FAILED (0xC0000142)
*/

BOOL
opencbm_init(IN HANDLE Module, IN DWORD Reason, IN LPVOID Reserved)
{
    static BOOL bIsOpen = FALSE;
    BOOLEAN Status = TRUE;

    FUNC_ENTER();

#if DBG

    if (Reason == DLL_PROCESS_ATTACH)
    {
        // Read the debugging flags from the registry

        cbm_i_get_debugging_flags();
    }

#endif

    /* make sure the definitions in opencbm.h and cbmioctl.h
     * match each other! 
     * Since we are the only instance which includes both files,
     * we are the only one which can ensure this.
     */

    DBG_ASSERT(IEC_LINE_CLOCK == IEC_CLOCK);
    DBG_ASSERT(IEC_LINE_RESET == IEC_RESET);
    DBG_ASSERT(IEC_LINE_DATA == IEC_DATA);
    DBG_ASSERT(IEC_LINE_ATN == IEC_ATN);

    switch (Reason) 
    {
        case DLL_PROCESS_ATTACH:

            if (IsDriverStartedAutomatically())
            {
                // the driver is started automatically, do not try
                // to start it

                Status = TRUE;
                break;
            }

            if (bIsOpen)
            {
                DBG_ERROR((DBG_PREFIX "No multiple instances are allowed!"));
                Status = FALSE;
            }
            else
            {
                Status  = TRUE;
                bIsOpen = cbm_i_driver_start();
            }
            break;

        case DLL_PROCESS_DETACH:

            if (IsDriverStartedAutomatically())
            {
                // the driver is started automatically, do not try
                // to stop it

                Status = TRUE;
                break;
            }

            if (!bIsOpen)
            {
                DBG_ERROR((DBG_PREFIX "Driver is not running!"));
                Status = FALSE; // is ignored anyway, but...
            }
            else
            {
                // it is arguable if the driver should be stopped
                // whenever the DLL is unloaded.

                cbm_i_driver_stop();
                bIsOpen = FALSE;
            }
            break;

        default:
            break;

    }

    FUNC_LEAVE_BOOL(Status);
}

/*! \brief Complete driver installation, "external version"

 This function performs anything that is needed to successfully
 complete the driver installation.

 \param Buffer
   Pointer to a buffer which will return the install information

 \param BufferLen
   The length of the buffer Buffer points to (in bytes).

 \return
   FALSE on success, TRUE on error

 This function is for use of the installation routines only!

 This version of this function is for exporting out of the DLL.
*/

BOOL CBMAPIDECL
cbm_i_driver_install(OUT PULONG Buffer, IN ULONG BufferLen)
{
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbm_i_i_driver_install(Buffer, BufferLen));
}
