/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2001 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 *  Copyright 2001-2005 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINBUILD/archlib.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: archlib.c,v 1.1 2005-02-13 17:58:12 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver, windows specific code
**
****************************************************************/

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
#include "archlib.h"


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

int
cbmarch_raw_write(CBM_FILE HandleDevice, const void *Buffer, size_t Count)
{
    DWORD BytesWritten;

    FUNC_ENTER();

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

int
cbmarch_raw_read(CBM_FILE HandleDevice, void *Buffer, size_t Count)
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

int
cbmarch_listen(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
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

int
cbmarch_talk(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
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

 \return
   0 means success, else failure

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int
cbmarch_open(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
{
    CBMT_OPEN_IN parameter;
    int returnValue;

    FUNC_ENTER();

    parameter.PrimaryAddress = DeviceAddress;
    parameter.SecondaryAddress = SecondaryAddress;

    if (cbm_ioctl(HandleDevice, CBMCTRL(OPEN), &parameter, sizeof(parameter), NULL, 0))
    {
        returnValue = 0;
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

int
cbmarch_close(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
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

int
cbmarch_unlisten(CBM_FILE HandleDevice)
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

int
cbmarch_untalk(CBM_FILE HandleDevice)
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

int
cbmarch_get_eoi(CBM_FILE HandleDevice)
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

int
cbmarch_clear_eoi(CBM_FILE HandleDevice)
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

int
cbmarch_reset(CBM_FILE HandleDevice)
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

__u_char
cbmarch_pp_read(CBM_FILE HandleDevice)
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

void
cbmarch_pp_write(CBM_FILE HandleDevice, __u_char Byte)
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

int
cbmarch_iec_poll(CBM_FILE HandleDevice)
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
   IEC_DATA, IEC_CLOCK, IEC_ATN, or IEC_RESET.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

void
cbmarch_iec_set(CBM_FILE HandleDevice, int Line)
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
   IEC_DATA, IEC_CLOCK, IEC_ATN, or IEC_RESET.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

void
cbmarch_iec_release(CBM_FILE HandleDevice, int Line)
{
    CBMT_IEC_RELEASE_IN parameter;

    FUNC_ENTER();

    parameter.Line = (UCHAR) Line;

    cbm_ioctl(HandleDevice, CBMCTRL(IEC_RELEASE), &parameter, sizeof(parameter), NULL, 0);

    FUNC_LEAVE();
}

/*! \brief Activate a line on the IEC serial bus

 This function activates (sets to 0V) and deactivates 
 lines on the IEC serial bus in one call.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Mask
   The mask of which lines have to be altered at all. Any line
   not mentioned here is left untouched. This has to be a bitwise
   OR between the constants IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET

 \param Line
   If a line has been set in Mask, the corresponding bit here decides
   if that line is to be set (in this case, it is ORed to this value)
   or released (in this case, the corresponding bit here is 0).

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

void
cbmarch_iec_setrelease(CBM_FILE HandleDevice, int Mask, int Line)
{
    CBMT_IEC_SETRELEASE_IN parameter;

    FUNC_ENTER();
 
    parameter.State = (UCHAR) Mask;
    parameter.Line = (UCHAR) Line;

    cbm_ioctl(HandleDevice, CBMCTRL(IEC_SETRELEASE), &parameter, sizeof(parameter), NULL, 0);

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

int
cbmarch_iec_wait(CBM_FILE HandleDevice, int Line, int State)
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
