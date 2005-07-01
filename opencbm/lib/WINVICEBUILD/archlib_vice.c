/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINVICEBUILD/archlib_vice.c \n
** \author Spiro Trikaliotis \n
** \version $Id: archlib_vice.c,v 1.1 2005-07-01 12:22:16 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**        This variant is for accessing VICE instead of a real device
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

#include "arch.h"
#include "vice_comm.h"

/**/
static void
DbgOut(const char * const Format, ...)
{
    va_list arg_ptr;
    int n;
    

    // Get a pointer to the current position in the given DebugBuffer

    char buffer[2048];

    va_start(arg_ptr, Format);

    // If there is some space left in the DebugBuffer, append the contents

    // Output at most the number of bytes which are left in the DebugBuffer

    n = _vsnprintf(buffer, sizeof(buffer) - 1, Format, arg_ptr);

    // Was the buffer too small? If yes, the number of bytes
    // inserted is the size of the remaining buffer, so, set
    // this value

    if (n<0) 
        n = sizeof(buffer);

    if (n < sizeof(buffer))
        buffer[n++] = '\n';

    buffer[n] = 0;

    va_end(arg_ptr);

    OutputDebugString(buffer);
}

static void
DbgLineStatus(const unsigned char Value)
{
    DbgOut("ATN OUT = %u, CLOCK OUT = %u, DATA OUT = %u, CLOCK IN = %u, DATA IN = %u",
        (Value & 0x08 ? 1 : 0),
        (Value & 0x10 ? 1 : 0),
        (Value & 0x20 ? 1 : 0),
        (Value & 0x40 ? 1 : 0),
        (Value & 0x80 ? 1 : 0));
}

/**/
static void
send_and_wait(const unsigned int addr, const unsigned char *buffer, int size)
{
//    unsigned int address;

    FUNC_ENTER();

    vicewritememory(addr, size, buffer);
    vicewriteregister(reg_pc, addr);
    vicewriteregister_when_at(0x2000);
    vicetrap(0x2000);
    viceresume();

    vicewaittrap();

    FUNC_LEAVE();
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

            Status = TRUE;
            break;

        case DLL_PROCESS_DETACH:

            vicerelease();
            Status = TRUE;
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
    FUNC_LEAVE_INT(FALSE);
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

static unsigned char data_rawwrite[] = { 
    0xA2, 0x00,       // 2100 LDX #$00
    0xBD, 0x00, 0x40, // 2102 LDA $4000,X
    0x20, 0xA8, 0xFF, // 2105 JSR $FFA8
    0xE8,             // 2108 INX
    0x88,             // 2109 DEY
    0xD0, 0xF6,       // 210A BNE $2102
//    0x20, 0x85, 0xEE, // 210C JSR $EE85 ; CLK = 1
//    0x20, 0x8E, 0xEE, // 210C JSR $EE8E ; CLK = 0
//    0x20, 0xA0, 0xEE, // 210F JSR $EEA0 ; DATA = 0
    0x4C, 0x00, 0x20  // 210C JMP $2000
};
static const addr_rawwrite = 0x2100;

int
cbmarch_raw_write(CBM_FILE HandleDevice, const void *Buffer, size_t Count)
{
    unsigned char byteswritten = 0;

    FUNC_ENTER();

    DBG_ASSERT(Count < 256 && Count >= 0);

    vicepause();
    vicewritememory(0x4000, Count, Buffer);
    vicewriteregister(reg_y, Count);
    viceresume();
    vicewaittrap();

    vicepause();
    send_and_wait(addr_rawwrite, data_rawwrite, sizeof(data_rawwrite));

    byteswritten = (unsigned char) vicereadregister(reg_y);

    FUNC_LEAVE_INT(Count - byteswritten);
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

static unsigned char data_rawread[] = { 
    0xA9, 0x00,       // 2180 LDA #$00
    0x85, 0x90,       // 2182 STA $90
    0xA2, 0x00,       // 2184 LDX #$00
    0x20, 0xA5, 0xFF, // 2186 JSR $FFA5
    0x9D, 0x00, 0x40, // 2189 STA $4000,X

    0xea, 0xea, 0xea, // 218C NOP; NOP; NOP; 
    // 0x20, 0xd2, 0xff, // 218C JSR $FFD2

    0xE8,             // 218F INX
    0x20, 0xB7, 0xFF, // 2190 JSR $FFB7
    0xC9, 0x40,       // 2193 CMP #$40
    0xF0, 0x08,       // 2195 BEQ $219F
    0x29, 0x40,       // 2197 AND #$40
    0xd0, 0x05,       // 2199 BNE $21A0
    0x88,             // 219B DEY
    0xD0, 0xE8,       // 219C BNE $2186
    0x24,             // 219E BIT (8 bit argument)
    0x88,             // 219F DEY
    0x4C, 0x00, 0x20  // 21A0 JMP $2000
};
static const addr_rawread = 0x2180;

int
cbmarch_raw_read(CBM_FILE HandleDevice, void *Buffer, size_t Count)
{
    unsigned char bytesread = 0;

    FUNC_ENTER();

    DBG_ASSERT(Count < 256 && Count >= 0);

    vicepause();
    vicewriteregister(reg_y, Count);
    vicepreparereadmemory(0x4000, Count);
    send_and_wait(addr_rawread, data_rawread, sizeof(data_rawread));

    vicereadmemory(0x4000, Count, Buffer);
    bytesread = (unsigned char) vicereadregister(reg_y);

    FUNC_LEAVE_INT(Count - bytesread);
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

static unsigned char data_listen[] = { 
    0xA0, 0x00,       // 2200 LDY #$00
    0x84, 0x90,       // 2202 STY $90
    0x20, 0xb1, 0xff, // 2204 JSR $FFB1 ; LISTEN
    0x8a,             // 2207 TXA
    0x20, 0x93, 0xff, // 2208 JSR $FF93 ; SECONDARY address after listen
    0xa5, 0x90,       // 220B LDA $90
    0x29, 0x80,       // 220D AND #$80
    0x4c, 0x00, 0x20  // 220F JMP $2000
};

static unsigned int addr_listen = 0x2200;
int
cbmarch_listen(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
{
    unsigned char status;

    FUNC_ENTER();

    vicepause();
    vicewriteregister(reg_a, DeviceAddress);
    vicewriteregister(reg_x, SecondaryAddress | 0x60);

    send_and_wait(addr_listen, data_listen, sizeof(data_listen));

    status = (unsigned char) vicereadregister(reg_a);

    FUNC_LEAVE_INT(status ? 1 : 0);
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

static unsigned char data_talk[] = { 
    0xA0, 0x00,       // 2280 LDY #$00
    0x84, 0x90,       // 2282 STY $90
    0x20, 0xb4, 0xff, // 2284 JSR $FFB4 ; TALK
    0x8a,             // 2287 TXA
    0x20, 0x96, 0xff, // 2288 JSR $FF96 ; TKSA, SECONDARY address after talk
    0xa5, 0x90,       // 228B LDA $90
    0x29, 0x80,       // 228D AND #$80
    0x4c, 0x00, 0x20  // 228F JMP $2000
};

static unsigned int addr_talk = 0x2280;

int
cbmarch_talk(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress)
{
    unsigned char status = 0;

    FUNC_ENTER();

    do
    {
        if (cbmarch_listen(HandleDevice, DeviceAddress, SecondaryAddress))
        {
            status = 1;
            break;
        }

        if (cbmarch_unlisten(HandleDevice))
        {
            status = 1;
            break;
        }

        vicepause();
        vicewriteregister(reg_a, DeviceAddress);
        vicewriteregister(reg_x, SecondaryAddress | 0x60);
        send_and_wait(addr_talk, data_talk, sizeof(data_talk));

        status = (unsigned char) vicereadregister(reg_a);

    } while (0);

    FUNC_LEAVE_INT(status ? 1 : 0);
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
    FUNC_ENTER();
    FUNC_LEAVE_INT(cbmarch_listen(HandleDevice, DeviceAddress, (__u_char)(SecondaryAddress | 0xf0)));
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
    FUNC_ENTER();
    cbmarch_listen(HandleDevice, DeviceAddress, (__u_char)((SecondaryAddress & 0x0f) | 0xe0));
    FUNC_LEAVE_INT(cbmarch_unlisten(HandleDevice));
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

static unsigned char data_unlisten[] = { 
    0x20, 0xae, 0xff, // 2300 JSR $FFAE ; UNLISTEN
    0x4c, 0x00, 0x20  // 2303 JMP $2000
};

static unsigned int addr_unlisten = 0x2300;

int
cbmarch_unlisten(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    vicepause();
    send_and_wait(addr_unlisten, data_unlisten, sizeof(data_unlisten));

    FUNC_LEAVE_INT(0);
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

static unsigned char data_untalk[] = { 
    0x20, 0xab, 0xff, // 2380 JSR $FFAB ; UNTALK
    0x4c, 0x00, 0x20  // 2383 JMP $2000
};

static unsigned int addr_untalk = 0x2380;

int
cbmarch_untalk(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    vicepause();
    send_and_wait(addr_untalk, data_untalk, sizeof(data_untalk));

    FUNC_LEAVE_INT(0);
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

static unsigned char data_geteoi[] = { 
    0x20, 0xb7, 0xff, // 2400 JSR $FFB7 ; get status
    0x4c, 0x00, 0x20  // 2403 JMP $2000
};

static unsigned int addr_geteoi = 0x2400;

int
cbmarch_get_eoi(CBM_FILE HandleDevice)
{
    unsigned char status = 0;

    FUNC_ENTER();

    vicepause();
    send_and_wait(addr_geteoi, data_geteoi, sizeof(data_geteoi));

    status = (unsigned char) vicereadregister(reg_a);

    FUNC_LEAVE_INT(status & 0x40 ? 1 : 0);
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

static unsigned char data_cleareoi[] = { 
    0xa5, 0x90,       // 2480 LDA $90
    0x29, 0xBF,       // 2482 AND #$BF
    0x85, 0x90,       // 2484 STA $90
    0x4c, 0x00, 0x20  // 2486 JMP $2000
};

static unsigned int addr_cleareoi = 0x2480;

int
cbmarch_clear_eoi(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    vicepause();
    send_and_wait(addr_cleareoi, data_cleareoi, sizeof(data_cleareoi));

    FUNC_LEAVE_INT(0);
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
    FUNC_ENTER();
    vicereset();
    FUNC_LEAVE_INT(0);
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

static unsigned char data_pp_read[] = { 
    0xA9, 0x00,       // 2500 LDA #$00
    0x8D, 0x03, 0xDD, // 2502 STA $DD03
    0xAD, 0x01, 0xDD, // 2505 LDA $DD01
    0x4C, 0x00, 0x20  // 2508 JMP $2000
};

static unsigned int addr_pp_read = 0x2500;

__u_char
cbmarch_pp_read(CBM_FILE HandleDevice)
{
    __u_char read;

    FUNC_ENTER();

    vicepause();
    send_and_wait(addr_pp_read, data_pp_read, sizeof(data_pp_read));

    read = (__u_char) vicereadregister(reg_a);

    FUNC_LEAVE_UCHAR(read);
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

static unsigned char data_pp_write[] = { 
    0xA0, 0xFF,       // 2580 LDY #$FF
    0x8C, 0x03, 0xDD, // 2582 STY $DD03
    0x8D, 0x01, 0xDD, // 2585 STA $DD01
    0x4C, 0x00, 0x20  // 2588 JMP $2000
};

static unsigned int addr_pp_write = 0x2580;

void
cbmarch_pp_write(CBM_FILE HandleDevice, __u_char Byte)
{
    FUNC_ENTER();

    vicepause();
    vicewriteregister(reg_a, Byte);
    send_and_wait(addr_pp_write, data_pp_write, sizeof(data_pp_write));

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

static unsigned char data_iec_poll[] = { 
    0xAD, 0x00, 0xDD, // 2600 LDA $DD00
    0x4C, 0x00, 0x20  // 2603 JMP $2000
};

static unsigned int addr_iec_poll = 0x2600;

#define VICE_ATN_OUT  0x08
#define VICE_CLK_IN   0x40
#define VICE_CLK_OUT  0x10
#define VICE_DATA_IN  0x80
#define VICE_DATA_OUT 0x20

int
cbmarch_iec_poll(CBM_FILE HandleDevice)
{
    int line = 0;
    int result = 0;

    FUNC_ENTER();

    vicepause();
    send_and_wait(addr_iec_poll, data_iec_poll, sizeof(data_iec_poll));

    line = vicereadregister(reg_a);

    DbgOut("iec_poll = $%02x", line);
    DbgLineStatus((unsigned char)(line ^ 0xc0));

    if (!(line & VICE_CLK_IN))  result |= IEC_CLOCK;
    if (!(line & VICE_DATA_IN)) result |= IEC_DATA;

    FUNC_LEAVE_INT(result);
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
    FUNC_ENTER();

    cbmarch_iec_setrelease(HandleDevice, Line, Line);

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
    FUNC_ENTER();

    cbmarch_iec_setrelease(HandleDevice, Line, 0);

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

static unsigned char data_iec_setrelease[] = { 
    0x8E, 0x87, 0x26, // 2680 STX $2687
    0x2D, 0x00, 0xDD, // 2683 AND $DD00
    0x09, 0x00,       // 2686 ORA #$00
    0x8D, 0x00, 0xDD, // 2688 STA $DD00
    0x4C, 0x00, 0x20, // 268B JMP $2000
};

static unsigned int addr_iec_setrelease = 0x2680;

void
cbmarch_iec_setrelease(CBM_FILE HandleDevice, int Mask, int Line)
{
    __u_char line = 0;
    __u_char mask = 0;

    FUNC_ENTER();

    DbgOut("");
    DbgOut("setrelease: Mask = %u, Line = %u", Mask, Line);
    cbmarch_iec_poll(HandleDevice);

    if (Mask & IEC_DATA)  mask |= VICE_DATA_OUT;
    if (Mask & IEC_CLOCK) mask |= VICE_CLK_OUT;
    if (Mask & IEC_ATN)   mask |= VICE_ATN_OUT;

    if (Line & IEC_DATA)  line |= VICE_DATA_OUT;
    if (Line & IEC_CLOCK) line |= VICE_CLK_OUT;
    if (Line & IEC_ATN)   line |= VICE_ATN_OUT;

    if (mask)
    {
        DbgOut("iec_setrelease = mask = $%02x, line = $%02x", mask, line);

        vicepause();
        vicewriteregister(reg_a, mask ^ 0xff);
        vicewriteregister(reg_x, line ^ mask);
        send_and_wait(addr_iec_setrelease, data_iec_setrelease, sizeof(data_iec_setrelease));
    }

    cbmarch_iec_poll(HandleDevice);

    FUNC_LEAVE();
}

/*! \brief Wait for a line to have a specific state

 This function waits for a line to enter a specific state
 on the IEC serial bus.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Line
   The line to be waited for. This must be exactly one of
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
    FUNC_ENTER();

    if (State)
    {
        while(!cbm_iec_get(HandleDevice, Line))
            arch_usleep(10);
    }
    else
    {
        while(cbm_iec_get(HandleDevice, Line))
            arch_usleep(10);
    }

    FUNC_LEAVE_INT(cbmarch_iec_poll(HandleDevice));
}
