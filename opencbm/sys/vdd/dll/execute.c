/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis
*/

/*! ************************************************************** 
** \file sys/vdd/dll/execute.c \n
** \author Spiro Trikaliotis \n
** \version $Id: execute.c,v 1.2 2004-12-22 14:57:04 strik Exp $ \n
** \n
** \brief Execution functions of the VDD
**
****************************************************************/

#include <windows.h>
#include <vddsvc.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! Mark: We are building the DLL */
#define DBG_DLL

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBMVDD.DLL"

#include "debug.h"

#include "vdd.h"
#include "opencbm.h"


/*-------------------------------------------------------------------*/
/*--------- HELPER FUNCTIONS ----------------------------------------*/

static PVOID
get_vdm_address(WORD Offset, WORD Length)
{
    PBYTE buffer;
    ULONG addressInVdm;

    addressInVdm = (ULONG) (getES()<<16 | Offset);

    buffer = GetVDMPointer (addressInVdm, Length, FALSE);

    return buffer;
}

static VOID
release_vdm_address(WORD Offset, WORD Length, PVOID Buffer)
{
    ULONG addressInVdm;

    addressInVdm = (ULONG) (getES()<<16 | Offset);

    FreeVDMPointer(addressInVdm, Length, Length, FALSE);
}


#define retAX( _x_ )  setAX((WORD) _x_)

#define FUNC_CHECKEDBUFFERACCESS(_offset_, _length_) \
\
    BOOLEAN error; \
    PVOID buffer; \
    WORD length; \
    WORD offset; \
\
    FUNC_ENTER(); \
\
    offset = _offset_; \
    length = _length_;

#define CHECKEDBUFFERACCESS_PROLOG()  \
\
    buffer = get_vdm_address(offset, length); \
\
    if (!buffer) \
    { \
        error = TRUE; \
    } \
    else \
    { \
        int ret; \
\
        error = FALSE;

#define CHECKEDBUFFERACCESS_EPILOG() \
        release_vdm_address(offset, length, buffer); \
\
        retAX(ret); \
    } \
    FUNC_LEAVE_BOOL(error);

#define CHECKEDBUFFERACCESS(_func_) \
        CHECKEDBUFFERACCESS_PROLOG(); \
        ret = _func_; \
        CHECKEDBUFFERACCESS_EPILOG();


/*-------------------------------------------------------------------*/
/*--------- DRIVER HANDLING -----------------------------------------*/

/*! \brief Opens the driver

 This function Opens the driver.

 \param PortNumber (CX)

   The port number of the driver to open. 0 means "default" driver, while
   values != 0 enumerate each driver.

 \return HandleDevice (BX)
  
   Handle to the driver

 \return AX: 

   ==0: This function completed successfully
   !=0: otherwise

 vdd_driver_open() should be balanced with vdd_driver_close().
*/

BOOLEAN
vdd_driver_open(VOID)
{
    CBM_FILE cbmfile;
    USHORT translatedhandle;
    int retValue;
 
    FUNC_ENTER();

    retValue = cbm_driver_open(&cbmfile, getCX());

    if (retValue == 0)
    {
        translatedhandle = vdd_cbmfile_store(cbmfile);
        if (translatedhandle == (WORD) -1)
	{
            cbm_driver_close(cbmfile);
            translatedhandle = -1;
        }
    }
    else
    {
        translatedhandle = -1;
    }

    setBX(translatedhandle);
    retAX(retValue ? 1 : 0);

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Closes the driver

 Closes the driver, which has be opened with vdd_driver_open() before.

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 vdd_driver_close() should be called to balance a previous call to
 vdd_driver_open(). 
 
 If vdd_driver_open() did not succeed, it is illegal to 
 call vdd_driver_close().
*/

BOOLEAN
vdd_driver_close(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    cbm_driver_close(HandleDevice);

    FUNC_LEAVE_BOOL(FALSE);
}

/*-------------------------------------------------------------------*/
/*--------- BASIC I/O -----------------------------------------------*/

/*! \brief Write data to the IEC serial bus

 This function sends data after a vdd_listen().

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer (ES:DI)

   Pointer to a buffer which hold the bytes to write to the bus.

 \param Count (CX)

   Number of bytes to be written.

 \return (AX)

   >= 0: The actual number of bytes written. 
   <0  indicates an error.

 This function tries to write Count bytes. Anyway, if an error
 occurs, this function can stop prematurely.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_raw_write(CBM_FILE HandleDevice)
{
    FUNC_CHECKEDBUFFERACCESS(getDI(), getCX());

    CHECKEDBUFFERACCESS(cbm_raw_write(HandleDevice, buffer, length));
}


/*! \brief Read data from the IEC serial bus

 This function retrieves data after a vdd_talk().

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer (ES:DI)

   Pointer to a buffer which will hold the bytes read.

 \param Count (CX)

   Number of bytes to be read at most.

 \return (AX)

   >= 0: The actual number of bytes read. 
   <0  indicates an error.

 At most Count bytes are read.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_raw_read(CBM_FILE HandleDevice)
{
    FUNC_CHECKEDBUFFERACCESS(getDI(), getCX());

    CHECKEDBUFFERACCESS(cbm_raw_read(HandleDevice, buffer, length));
}

/*! \brief Send a LISTEN on the IEC serial bus

 This function sends a LISTEN on the IEC serial bus.
 This prepares a LISTENer, so that it will wait for our
 bytes we will write in the future.

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (DH)

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param SecondaryAddress (CL)

   The secondary address for the device on the IEC serial bus.

 \return (AX)

   0 means success, else failure

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_listen(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_listen(HandleDevice, getDH(), getCL()));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Send a TALK on the IEC serial bus

 This function sends a TALK on the IEC serial bus.
 This prepares a TALKer, so that it will prepare to send
 us some bytes in the future.

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (DH)

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param SecondaryAddress (CL)

   The secondary address for the device on the IEC serial bus.

 \return (AX)

   0 means success, else failure

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_talk(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_talk(HandleDevice, getDH(), getCL()));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Open a file on the IEC serial bus

 This function opens a file on the IEC serial bus.

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (DH)

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param SecondaryAddress (CL)

   The secondary address for the device on the IEC serial bus.

 \param Filename (ES:SI)

   Pointer to the filename of the file to be opened

 \param FilenameLength (DI)

   The size of the Filename. If zero, the Filename has to be
   a null-terminated string.

 \return (AX)

   0 means success, else failure

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_open(CBM_FILE HandleDevice)
{
    FUNC_CHECKEDBUFFERACCESS(getSI(), getDI());

    CHECKEDBUFFERACCESS(cbm_open(HandleDevice, getDH(), getCL(), buffer, length));
}

/*! \brief Close a file on the IEC serial bus

 This function closes a file on the IEC serial bus.

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (DH)

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param SecondaryAddress (CL)

   The secondary address for the device on the IEC serial bus.

 \return (AX)

   0 on success, else failure

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_close(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_close(HandleDevice, getDH(), getCL()));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Send an UNLISTEN on the IEC serial bus

 This function sends an UNLISTEN on the IEC serial bus.
 Other than LISTEN and TALK, an UNLISTEN is not directed
 to just one device, but to all devices on that IEC
 serial bus. 

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \return (AX)

   0 on success, else failure

 At least on a 1541 floppy drive, an UNLISTEN also undoes
 a previous TALK.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_unlisten(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_unlisten(HandleDevice));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Send an UNTALK on the IEC serial bus

 This function sends an UNTALK on the IEC serial bus.
 Other than LISTEN and TALK, an UNTALK is not directed
 to just one device, but to all devices on that IEC
 serial bus. 

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \return (AX)

   0 on success, else failure

 At least on a 1541 floppy drive, an UNTALK also undoes
 a previous LISTEN.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_untalk(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_untalk(HandleDevice));

    FUNC_LEAVE_BOOL(FALSE);
}


/*! \brief Get EOI flag after bus read

 This function gets the EOI ("End of Information") flag 
 after reading the IEC serial bus.

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \return (AX)

   != 0 if EOI was signalled, else 0.

 If a previous read returned less than the specified number
 of bytes, there are two possible reasons: Either an error
 occurred on the IEC serial bus, or an EOI was signalled.
 To find out the cause, check with this function.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_get_eoi(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_get_eoi(HandleDevice));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Reset the EOI flag

 This function resets the EOI ("End of Information") flag
 which might be still set after reading the IEC serial bus.

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \return (AX)

   0 on success, != 0 means an error has occured.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_clear_eoi(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_clear_eoi(HandleDevice));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief RESET all devices

 This function performs a hardware RESET of all devices on
 the IEC serial bus.

 \param HandleDevice (BX)
  
   A CBM_FILE which contains the file handle of the driver.

 \return (AX)

   0 on success, else failure

 Don't overuse this function! Normally, an initial RESET
 should be enough.

 Control is returned after a delay which ensures that all
 devices are ready again.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_reset(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_reset(HandleDevice));

    FUNC_LEAVE_BOOL(FALSE);
}


/*-------------------------------------------------------------------*/
/*--------- LOW-LEVEL PORT ACCESS -----------------------------------*/

/*! \brief Read a byte from a XP1541/XP1571 cable

 This function reads a single byte from the parallel portion of 
 an XP1541/1571 cable.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \return (AL)

   the byte which was received on the parallel port

 This function reads the current state of the port. No handshaking
 is performed at all.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_pp_read(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    setAL(cbm_pp_read(HandleDevice));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Write a byte to a XP1541/XP1571 cable

 This function writes a single byte to the parallel portion of 
 a XP1541/1571 cable.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \param Byte (DH)

   the byte to be output on the parallel port

 This function just writes on the port. No handshaking
 is performed at all.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_pp_write(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    cbm_pp_write(HandleDevice, getDH());

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Read status of all bus lines.

 This function reads the state of all lines on the IEC serial bus.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \return (AX)

   The state of the lines. The result is an OR between
   the bit flags IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 This function just reads the port. No handshaking
 is performed at all.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_iec_poll(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_iec_poll(HandleDevice));

    FUNC_LEAVE_BOOL(FALSE);
}


/*! \brief Activate a line on the IEC serial bus

 This function activates (sets to 0V) a line on the IEC serial bus.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \param Line (DH)

   The line to be activated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_iec_set(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    cbm_iec_set(HandleDevice, getDH());

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Deactivate a line on the IEC serial bus

 This function deactivates (sets to 5V) a line on the IEC serial bus.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \param Line (DH)

   The line to be deactivated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_iec_release(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    cbm_iec_release(HandleDevice, getDH());

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Wait for a line to have a specific state

 This function waits for a line to enter a specific state
 on the IEC serial bus.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \param Line (DH)
   The line to be deactivated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 \param State (CL)
   If zero, then wait for this line to be deactivated. \n
   If not zero, then wait for this line to be activated.

 \return (AX)
   The state of the IEC bus on return (like vdd_iec_poll).

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_iec_wait(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_iec_wait(HandleDevice, getDH(), getCL()));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Get the (logical) state of a line on the IEC serial bus

 This function gets the (logical) state of a line on the IEC serial bus.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \param Line (DH)

   The line to be tested. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 \return (AX)

   1 if the line is set, 0 if it is not

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_iec_get(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    retAX(cbm_iec_get(HandleDevice, getDH()));

    FUNC_LEAVE_BOOL(FALSE);
}


/*-------------------------------------------------------------------*/
/*--------- HELPER FUNCTIONS ----------------------------------------*/


/*! \brief Upload a program into a floppy's drive memory.

 This function writes a program into the drive's memory
 via use of "M-W" commands.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (DH)

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param DriveMemAddress (DI)

   The address in the drive's memory where the program is to be
   stored.
   
 \param Program (ES:SI)

   Pointer to a byte buffer which holds the program in the 
   caller's address space.

 \param Size (CX)

   The size of the program to be stored, in bytes.

 \return (AX)

   Returns the number of bytes written into program memory.
   If it does not equal Size, than an error occurred.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_upload(CBM_FILE HandleDevice)
{
    FUNC_CHECKEDBUFFERACCESS(getSI(), getCX());

    CHECKEDBUFFERACCESS(cbm_upload(HandleDevice, getDH(), getDI(), buffer, length));
}

/*! \brief Read the drive status from a floppy

 This function reads the drive status of a connected
 floppy drive.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (DH)
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param Buffer (ES:DI)
   Pointer to a buffer which will hold the drive's status after
   successfull calling,

 \param BufferLength (CX)
   The length of the buffer pointed to by Buffer in bytes.

 \return (AX)
   Returns the int representation of the drive status,
   that is, the numerical value of the first return
   value from the drive. This is the error number.

 This function will never write more than BufferLength bytes.
 Nevertheless, the buffer will always be terminated with
 a trailing zero.

 If an error occurs, this function returns a
 "99, DRIVER ERROR,00,00\r" and the value 99.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_device_status(CBM_FILE HandleDevice)
{
    FUNC_CHECKEDBUFFERACCESS(getDI(), getCX());

    CHECKEDBUFFERACCESS(cbm_device_status(HandleDevice, getDH(), buffer, length));
}

/*! \brief Executes a command in the floppy drive.

 This function Executes a command in the connected floppy drive.

 \param HandleDevice (BX)

   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (DH)

   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param Command (ES:SI)

   Pointer to a string which holds the command to be executed.

 \param Size (CX)

   The length of the command in bytes. If zero, the Command
   has to be a null-terminated string.

 \return (AX)

   0 on success.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_exec_command(CBM_FILE HandleDevice)
{
    FUNC_CHECKEDBUFFERACCESS(getSI(), getCX());

    CHECKEDBUFFERACCESS(cbm_exec_command(HandleDevice, getDH(), buffer, length));
}

/*! \brief Identify the connected floppy drive.

 This function tries to identify a connected floppy drive.
 For this, it performs some M-R operations.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (DH)
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param CbmDeviceString (ES:SI)
   Buffer which will hold a string which tells the name 
   of the device.
 
 \param Length (CX)
   Length of the buffer at ES:SI

 \return CbmDeviceType (DI)
   Pointer to an enum which will hold the type of the device.

 \return (AX)
   0 if the drive could be contacted. It does not mean that
   the device could be identified.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_identify(CBM_FILE HandleDevice)
{
    enum cbm_devicetype_e devicetype;
    char *string;

    FUNC_CHECKEDBUFFERACCESS(getSI(), getCX());

    CHECKEDBUFFERACCESS_PROLOG();
    
    ret = cbm_identify(HandleDevice, getDH(), &devicetype, &string);

    setDI(devicetype);
    strncpy(buffer, string, length);

    CHECKEDBUFFERACCESS_EPILOG();
}
