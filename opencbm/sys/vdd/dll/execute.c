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
** \version $Id: execute.c,v 1.4 2005-01-06 21:00:16 strik Exp $ \n
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

 \param PortNumber (DH)
   The port number of the driver to open. 0 means "default" driver, while
   values != 0 enumerate each driver.

 \return AX: 
   ==0: This function completed successfully\n
   !=0: otherwise

 \return HandleDevice (BX)
   Handle to the driver

 vdd_driver_open() should be balanced with vdd_driver_close().
*/

BOOLEAN
vdd_driver_open(VOID)
{
    CBM_FILE cbmfile;
    USHORT translatedhandle;
    int retValue;
 
    FUNC_ENTER();

    retValue = cbm_driver_open(&cbmfile, getDH());

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


/*! \brief Get the name of the driver for a specific parallel port

 Get the name of the driver for a specific parallel port.

 \param PortNumber (DH)
   The port number for the driver to open. 0 means "default" driver, while
   values != 0 enumerate each driver.

 \param Buffer (ES:SI)
   Buffer which will hold a string which tells the name 
   of the device.
 
 \param Length (CX)
   Length of the buffer at ES:SI

 \return 
   Returns a pointer to a null-terminated string containing the
   driver name, or NULL if an error occurred.

 \bug
   PortNumber is not allowed to exceed 10. 
*/

BOOLEAN
vdd_get_driver_name(VOID)
{
    const char *returned_string;

    FUNC_CHECKEDBUFFERACCESS(getSI(), getCX());

    CHECKEDBUFFERACCESS_PROLOG();
    
    returned_string = cbm_get_driver_name(getDH());

    strncpy(buffer, returned_string, length);

    ret = FALSE;

    CHECKEDBUFFERACCESS_EPILOG();
}


/*-------------------------------------------------------------------*/
/*--------- BASIC I/O -----------------------------------------------*/

/*! \brief Write data to the IEC serial bus

 This function sends data after a vdd_listen().

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer (ES:SI)
   Pointer to a buffer which hold the bytes to write to the bus.

 \param Count (CX)
   Number of bytes to be written.

 \return (AX)
   >= 0: The actual number of bytes written.\n
   <0  indicates an error.

 This function tries to write Count bytes. Anyway, if an error
 occurs, this function can stop prematurely.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_raw_write(CBM_FILE HandleDevice)
{
    FUNC_CHECKEDBUFFERACCESS(getSI(), getCX());

    CHECKEDBUFFERACCESS(cbm_raw_write(HandleDevice, buffer, length));
}


/*! \brief Read data from the IEC serial bus

 This function retrieves data after a vdd_talk().

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer (ES:SI)
   Pointer to a buffer which will hold the bytes read.

 \param Count (CX)
   Number of bytes to be read at most.

 \return (AX)
   >= 0: The actual number of bytes read.\n
   <0  indicates an error.

 At most Count bytes are read.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_raw_read(CBM_FILE HandleDevice)
{
    FUNC_CHECKEDBUFFERACCESS(getSI(), getCX());

    CHECKEDBUFFERACCESS(cbm_raw_read(HandleDevice, buffer, length));
}

/*! \brief Send a LISTEN on the IEC serial bus

 This function sends a LISTEN on the IEC serial bus.
 This prepares a LISTENer, so that it will wait for our
 bytes we will write in the future.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (CH)
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

    retAX(cbm_listen(HandleDevice, getCH(), getCL()));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Send a TALK on the IEC serial bus

 This function sends a TALK on the IEC serial bus.
 This prepares a TALKer, so that it will prepare to send
 us some bytes in the future.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (CH)
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

    retAX(cbm_talk(HandleDevice, getCH(), getCL()));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Open a file on the IEC serial bus

 This function opens a file on the IEC serial bus.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (CH)
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

    CHECKEDBUFFERACCESS(cbm_open(HandleDevice, getCH(), getCL(), buffer, length));
}

/*! \brief Close a file on the IEC serial bus

 This function closes a file on the IEC serial bus.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress (CH)
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

    retAX(cbm_close(HandleDevice, getCH(), getCL()));

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

 \return (AX)
   the byte which was received on the parallel port. AL contains
   the byte, AH is zeroed.

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

    setAX(cbm_pp_read(HandleDevice));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Write a byte to a XP1541/XP1571 cable

 This function writes a single byte to the parallel portion of 
 a XP1541/1571 cable.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param Byte (CL)
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

    cbm_pp_write(HandleDevice, getCL());

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

 \param Line (CL)
   The line to be activated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, or IEC_ATN.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_iec_set(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    cbm_iec_set(HandleDevice, getCL());

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Deactivate a line on the IEC serial bus

 This function deactivates (sets to 5V) a line on the IEC serial bus.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param Line (CL)
   The line to be deactivated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, or IEC_ATN.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.

 \bug
   This function can't signal an error, thus, be careful!
*/

BOOLEAN
vdd_iec_release(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    cbm_iec_release(HandleDevice, getCL());

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Wait for a line to have a specific state

 This function waits for a line to enter a specific state
 on the IEC serial bus.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param Line (CL)
   The line to be deactivated. This must be exactly one of
   IEC_DATA, IEC_CLOCK, IEC_ATN, and IEC_RESET.

 \param State (CH)
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

    retAX(cbm_iec_wait(HandleDevice, getCL(), getCH()));

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Get the (logical) state of a line on the IEC serial bus

 This function gets the (logical) state of a line on the IEC serial bus.

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param Line (CL)
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

    retAX(cbm_iec_get(HandleDevice, getCL()));

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

 \param Buffer (ES:SI)
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
    FUNC_CHECKEDBUFFERACCESS(getSI(), getCX());

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

 \return (AX)
   0 if the drive could be contacted. It does not mean that
   the device could be identified.

 \return CbmDeviceType (DI)
   Pointer to an enum which will hold the type of the device.

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

/*-------------------------------------------------------------------*/
/*--------- FUNCTIONS FOR ACCESS IN THE I/O ADDRESS SPACE -----------*/

static CBM_FILE VddCbmFileForIoHook;

static BOOLEAN VddIoHookInstalled = FALSE;
static VDD_IO_PORTRANGE VddIoPortRange;
static VDD_IO_HANDLERS VddIoPortHandlers;

static BYTE vdd_iohook_lastwrittencontrolregister = 0xFF;


/* lpt output lines for XE cables */
#define PP_XE_ATN_OUT    0x01 //!< The ATN OUT bit
#define PP_XE_CLK_OUT    0x02 //!< The CLOCK OUT bit
#define PP_XE_RESET_OUT  0x04 //!< The RESET OUT bit
#define PP_XE_DATA_OUT   0x08 //!< The DATA OUT bit

/* lpt input lines for XE cables */
#define PP_XE_ATN_IN     0x10 //!< The ATN IN bit
#define PP_XE_CLK_IN     0x20 //!< The CLOCK IN bit
#define PP_XE_RESET_IN   0x40 //!< The RESET IN bit
#define PP_XE_DATA_IN    0x80 //!< The DATA IN bit

/*
 The following accesses are supported:
 - +0: Data register (read and write)
 - +1: STATUS register (read only, for input)
 - +2: Contrlol register (write only, for output)

 XA: 0xcb  11001011
 XM: 0xc4  11000100
*/
static VOID
vdd_iohook_inb(WORD iport,BYTE *data)
{
    if (iport == VddIoPortRange.First)
    {
        // read the data port (XP15x1 part)
        *data = cbm_pp_read(VddCbmFileForIoHook);
    }
    else if (iport == VddIoPortRange.First + 1)
    {
        UINT value;
        BYTE ret;

        // read the status register (for Input)
        value = cbm_iec_poll(VddCbmFileForIoHook);

        ret = 0;

        if (value & IEC_DATA)
        {
            ret |= PP_XE_DATA_IN;
        }
        if (value & IEC_CLOCK)
        {
            ret |= PP_XE_CLK_IN;
        }
        if (value & IEC_ATN)
        {
            ret |= PP_XE_ATN_IN;
        }

        // as the one line is inverted by the parallel port, behave the same
        *data = ret ^ 0x04;
    }
    else if (iport == VddIoPortRange.First + 2)
    {
        // reading the control register: Gives the last written state
        *data = 0x04 ^ vdd_iohook_lastwrittencontrolregister;
    }
    else DBG_ERROR((DBG_PREFIX "Access to unknown address %08x", iport));
}

static VOID
vdd_iohook_outb(WORD iport,BYTE data)
{
    if (iport == VddIoPortRange.First)
    {
        // write the data port (XP15x1 part)
        cbm_pp_write(VddCbmFileForIoHook, data);
    }
    else if (iport == VddIoPortRange.First + 1)
    {
        DBG_ERROR((DBG_PREFIX "Writing the status register: UNSUPPORTED!"));
    }
    else if (iport == VddIoPortRange.First + 2)
    {
        // writing the control register

        BYTE ret = data ^ vdd_iohook_lastwrittencontrolregister;

        vdd_iohook_lastwrittencontrolregister = data;

        data ^= 0x04;

        if (ret & PP_XE_ATN_OUT)
        {
            // ATN was changed
            ((data & PP_XE_ATN_OUT) ? cbm_iec_set : cbm_iec_release)
                (VddCbmFileForIoHook, IEC_ATN);
        }
        if (ret & PP_XE_CLK_OUT)
        {
            // CLOCK was changed
            ((data & PP_XE_CLK_OUT) ? cbm_iec_set : cbm_iec_release)
                (VddCbmFileForIoHook, IEC_CLOCK);
        }
        if (ret & PP_XE_DATA_OUT)
        {
            // DATA was changed
            ((data & PP_XE_DATA_OUT) ? cbm_iec_set : cbm_iec_release)
                (VddCbmFileForIoHook, IEC_DATA);
        }
        if (ret & PP_XE_RESET_OUT)
        {
            // RESET was changed: Only process it if it was set
            if (data & PP_XE_RESET_OUT)
            {
                cbm_reset(VddCbmFileForIoHook);
            }
        }
    }
    else DBG_ERROR((DBG_PREFIX "Access to unknown address %08x", iport));
}


/*! \brief Install the I/O hook

 This function installs the I/O hook for accessing the VDD via IN
 and OUT assembler instructions

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \param IoBaseAddress (CX)
   The I/O base address on which to install the I/O hook

 \return (AX)
   Returns the I/O base address on which the I/O hook has been installed.\n
   If the hook could not be installed, this return value is zero.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_install_iohook(CBM_FILE HandleDevice)
{
    UINT where = getCX();

    FUNC_ENTER();

    DBG_PRINT((DBG_PREFIX "Entering vdd_install_iohook, installing at 0x%08X", where));

    /* assume that the hook is already installed, thus,
     * we cannot install the hook again
     */
    setAX(0);

    if (!VddIoHookInstalled)
    {
        VddIoPortRange.First = getCX();
        VddIoPortRange.Last = VddIoPortRange.First + 2;

        DBG_PRINT((DBG_PREFIX "trying to install from 0x%08x to 0x%08x, Handle = 0x%08x",
            VddIoPortRange.First, VddIoPortRange.Last, vdd_handle));

        RtlZeroMemory(&VddIoPortHandlers, sizeof(VddIoPortHandlers));
        VddIoPortHandlers.inb_handler = vdd_iohook_inb;
        VddIoPortHandlers.outb_handler = vdd_iohook_outb;

        if (VDDInstallIOHook(vdd_handle, 1, &VddIoPortRange, &VddIoPortHandlers))
        {
            DBG_PRINT((DBG_PREFIX "SUCCESS!"));
            // we had success
            VddIoHookInstalled = TRUE;
            setAX(VddIoPortRange.First);

            VddCbmFileForIoHook = HandleDevice;
        }
        else
        {
            DBG_PRINT((DBG_PREFIX "FAILED!"));
        }
    }

    FUNC_LEAVE_BOOL(FALSE);
}

/*! \brief Uninstall the I/O hook

 This function uninstalls the I/O hook for accessing the VDD via IN
 and OUT assembler instructions

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \return (AX)
   Returns the I/O base address on which the I/O hook was installed
   before.\n
   If no hook was installed, this return value is zero.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

USHORT
vdd_uninstall_iohook_internal(VOID)
{
    USHORT ret;

    FUNC_ENTER();

    if (VddIoHookInstalled)
    {
        VDDDeInstallIOHook(vdd_handle, 1, &VddIoPortRange);

        VddIoHookInstalled = FALSE;
        ret = VddIoPortRange.First;
    }
    else
    {
        ret = 0;
    }

    FUNC_LEAVE_INT(ret);
}

/*! \brief Uninstall the I/O hook

 This function uninstalls the I/O hook for accessing the VDD via IN
 and OUT assembler instructions

 \param HandleDevice (BX)
   A CBM_FILE which contains the file handle of the driver.

 \return (AX)
   Returns the I/O base address on which the I/O hook was installed
   before.\n
   If no hook was installed, this return value is zero.

 If vdd_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOLEAN
vdd_uninstall_iohook(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    DBG_PRINT((DBG_PREFIX "Entering vdd_uninstall_iohook"));

    setAX(vdd_uninstall_iohook_internal());

    FUNC_LEAVE_BOOL(FALSE);
}
