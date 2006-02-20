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
** \file lib/cbm.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: cbm.c,v 1.11 2006-02-20 12:11:16 strik Exp $ \n
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
#include <string.h>

//! mark: We are building the DLL */
#define DLL
#include "opencbm.h"
#include "archlib.h"


// #define DBG_DUMP_RAW_READ
// #define DBG_DUMP_RAW_WRITE

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

    FUNC_LEAVE_STRING(cbmarch_get_driver_name(PortNumber));
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

    FUNC_LEAVE_INT(cbmarch_driver_open(HandleDevice, PortNumber));
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

    cbmarch_driver_close(HandleDevice);

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
    FUNC_ENTER();

#ifdef DBG_DUMP_RAW_WRITE
    DBG_MEMDUMP("cbm_raw_write", Buffer, Count);
#endif

    FUNC_LEAVE_INT(cbmarch_raw_write(HandleDevice,Buffer, Count));
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
    int bytesRead;

    FUNC_ENTER();

    bytesRead = cbmarch_raw_read(HandleDevice, Buffer, Count);

#ifdef DBG_DUMP_RAW_READ
    DBG_MEMDUMP("cbm_raw_read", Buffer, bytesRead);
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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_listen(HandleDevice, DeviceAddress, SecondaryAddress));
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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_talk(HandleDevice, DeviceAddress, SecondaryAddress));
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
    int returnValue;

    FUNC_ENTER();

    if (cbmarch_open(HandleDevice, DeviceAddress, SecondaryAddress) == 0)
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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_close(HandleDevice, DeviceAddress, SecondaryAddress));
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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_unlisten(HandleDevice));
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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_untalk(HandleDevice));
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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_get_eoi(HandleDevice));
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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_clear_eoi(HandleDevice));
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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_reset(HandleDevice));
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
    FUNC_ENTER();

    FUNC_LEAVE_UCHAR(cbmarch_pp_read(HandleDevice));
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
    FUNC_ENTER();

    cbmarch_pp_write(HandleDevice, Byte);

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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_iec_poll(HandleDevice));
}


/*! \brief Activate a line on the IEC serial bus

 This function activates (sets to 0V) a line on the IEC serial bus.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Line
   The line to be activated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, or IEC_RESET.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

void CBMAPIDECL
cbm_iec_set(CBM_FILE HandleDevice, int Line)
{
    FUNC_ENTER();
 
    cbmarch_iec_set(HandleDevice, Line);

    FUNC_LEAVE();
}

/*! \brief Deactivate a line on the IEC serial bus

 This function deactivates (sets to 5V) a line on the IEC serial bus.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Line
   The line to be deactivated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, or IEC_RESET.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

void CBMAPIDECL
cbm_iec_release(CBM_FILE HandleDevice, int Line)
{
    FUNC_ENTER();
 
    cbmarch_iec_release(HandleDevice, Line);

    FUNC_LEAVE();
}

/*! \brief Activate and deactive a line on the IEC serial bus

 This function activates (sets to 0V, L) and deactivates 
 (set to 5V, H) lines on the IEC serial bus.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Set
   The mask of which lines should be set. This has to be a bitwise OR
   between the constants IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET

 \param Release
   The mask of which lines should be released. This has to be a bitwise
   OR between the constants IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!

 \remark
   If a bit is specified in the Set as well as in the Release mask, the
   effect is undefined.
*/

void CBMAPIDECL
cbm_iec_setrelease(CBM_FILE HandleDevice, int Set, int Release)
{
    FUNC_ENTER();
 
    cbmarch_iec_setrelease(HandleDevice, Set, Release);

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
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_iec_wait(HandleDevice, Line, State));
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
    FUNC_LEAVE_INT((cbmarch_iec_poll(HandleDevice)&Line) != 0 ? 1 : 0);
}


/*-------------------------------------------------------------------*/
/*--------- HELPER FUNCTIONS ----------------------------------------*/


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

        if (cbmarch_talk(HandleDevice, DeviceAddress, 15) == 0)
        {
            int bytesRead = cbm_raw_read(HandleDevice, bufferToWrite, BufferLength);

            DBG_ASSERT(bytesRead >= 0);
            DBG_ASSERT(((unsigned)bytesRead) <= BufferLength);

            // make sure we have a trailing zero at the end of the status:

            bufferToWrite[bytesRead] = '\0';

            cbmarch_untalk(HandleDevice);
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
    rv = cbmarch_listen(HandleDevice, DeviceAddress, 15);
    if(rv == 0) {
        if(Size == 0) {
            Size = (size_t) strlen(Command);
        }
        rv = (size_t) cbmarch_raw_write(HandleDevice, Command, Size) != Size;
        cbmarch_unlisten(HandleDevice);
    }

    FUNC_LEAVE_INT(rv);
}

/*! \brief MNIB: Read from the parallel port

 This function is a helper function for mnib:
 It reads from the parallel port.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \return
   The value read from the parallel port

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

__u_char CBMAPIDECL
cbm_mnib_par_read(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    FUNC_LEAVE_UCHAR(cbmarch_mnib_par_read(HandleDevice));
}

/*! \brief MNIB: Write to the parallel port

 This function is a helper function for mnib:
 It writes to the parallel port.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Value
   The value to be written to the parallel port

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

void CBMAPIDECL
cbm_mnib_par_write(CBM_FILE HandleDevice, __u_char Value)
{
    FUNC_ENTER();

    cbmarch_mnib_par_write(HandleDevice, Value);

    FUNC_LEAVE();
}

/*! \brief MNIB: Read a complete track

 This function is a helper function for mnib:
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
cbm_mnib_read_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length)
{
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_mnib_read_track(HandleDevice, Buffer, Length));
}

/*! \brief MNIB: Write a complete track

 This function is a helper function for mnib:
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
cbm_mnib_write_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length)
{
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbmarch_mnib_write_track(HandleDevice, Buffer, Length));
}
