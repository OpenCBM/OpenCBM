/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005,2007 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/cbm.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: cbm.c,v 1.17.2.5 2007-03-14 11:32:09 strik Exp $ \n
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

#ifdef ENABLE_XU1541
#include "xu1541.h"
#endif
#include "opencbm-plugin.h"

#include "getpluginaddress.h"

struct plugin_information_s {
    SHARED_OBJECT_HANDLE Library;
    opencbm_plugin_t     Plugin;
};
typedef struct plugin_information_s plugin_information_t;

static
struct plugin_information_s Plugin_information = { 0 };

static int
initialize_plugin_pointer(plugin_information_t *Plugin_information)
{
    int ret = 1;

    do {
        memset(&Plugin_information->Plugin, 0, sizeof(Plugin_information->Plugin));

#ifdef WIN32
        Plugin_information->Library = plugin_load("opencbm-xu1541.dll");
#else
        Plugin_information->Library = plugin_load("/usr/local/lib/libopencbm-xu1541.so");
#endif

        DBG_PRINT((DBG_PREFIX "LoadLibrary returned %p", Plugin_information->Library));

        if (!Plugin_information->Library)
            break;

        Plugin_information->Plugin.cbm_plugin_get_driver_name            = plugin_get_address(Plugin_information->Library, "cbmarch_get_driver_name");
        Plugin_information->Plugin.cbm_plugin_driver_open                = plugin_get_address(Plugin_information->Library, "cbmarch_driver_open");
        Plugin_information->Plugin.cbm_plugin_driver_close               = plugin_get_address(Plugin_information->Library, "cbmarch_driver_close");
        Plugin_information->Plugin.cbm_plugin_lock                       = plugin_get_address(Plugin_information->Library, "cbmarch_lock");
        Plugin_information->Plugin.cbm_plugin_unlock                     = plugin_get_address(Plugin_information->Library, "cbmarch_unlock");
        Plugin_information->Plugin.cbm_plugin_raw_write                  = plugin_get_address(Plugin_information->Library, "cbmarch_raw_write");
        Plugin_information->Plugin.cbm_plugin_raw_read                   = plugin_get_address(Plugin_information->Library, "cbmarch_raw_read");
        Plugin_information->Plugin.cbm_plugin_open                       = plugin_get_address(Plugin_information->Library, "cbmarch_open");
        Plugin_information->Plugin.cbm_plugin_close                      = plugin_get_address(Plugin_information->Library, "cbmarch_close");
        Plugin_information->Plugin.cbm_plugin_listen                     = plugin_get_address(Plugin_information->Library, "cbmarch_listen");
        Plugin_information->Plugin.cbm_plugin_talk                       = plugin_get_address(Plugin_information->Library, "cbmarch_talk");
        Plugin_information->Plugin.cbm_plugin_unlisten                   = plugin_get_address(Plugin_information->Library, "cbmarch_unlisten");
        Plugin_information->Plugin.cbm_plugin_untalk                     = plugin_get_address(Plugin_information->Library, "cbmarch_untalk");
        Plugin_information->Plugin.cbm_plugin_get_eoi                    = plugin_get_address(Plugin_information->Library, "cbmarch_get_eoi");
        Plugin_information->Plugin.cbm_plugin_clear_eoi                  = plugin_get_address(Plugin_information->Library, "cbmarch_clear_eoi");
        Plugin_information->Plugin.cbm_plugin_reset                      = plugin_get_address(Plugin_information->Library, "cbmarch_reset");
        Plugin_information->Plugin.cbm_plugin_pp_read                    = plugin_get_address(Plugin_information->Library, "cbmarch_pp_read");
        Plugin_information->Plugin.cbm_plugin_pp_write                   = plugin_get_address(Plugin_information->Library, "cbmarch_pp_write");
        Plugin_information->Plugin.cbm_plugin_iec_poll                   = plugin_get_address(Plugin_information->Library, "cbmarch_iec_poll");
        Plugin_information->Plugin.cbm_plugin_iec_set                    = plugin_get_address(Plugin_information->Library, "cbmarch_iec_set");
        Plugin_information->Plugin.cbm_plugin_iec_release                = plugin_get_address(Plugin_information->Library, "cbmarch_iec_release");
        Plugin_information->Plugin.cbm_plugin_iec_setrelease             = plugin_get_address(Plugin_information->Library, "cbmarch_iec_setrelease");
        Plugin_information->Plugin.cbm_plugin_iec_wait                   = plugin_get_address(Plugin_information->Library, "cbmarch_iec_wait");
        Plugin_information->Plugin.cbm_plugin_parallel_burst_read        = plugin_get_address(Plugin_information->Library, "cbmarch_parallel_burst_read");
        Plugin_information->Plugin.cbm_plugin_parallel_burst_write       = plugin_get_address(Plugin_information->Library, "cbmarch_parallel_burst_write");
        Plugin_information->Plugin.cbm_plugin_parallel_burst_read_track  = plugin_get_address(Plugin_information->Library, "cbmarch_parallel_burst_read_track");
        Plugin_information->Plugin.cbm_plugin_parallel_burst_write_track = plugin_get_address(Plugin_information->Library, "cbmarch_parallel_burst_write_track");

        ret = 0;

    } while (0);

    return ret;
}

static void
initialize_plugin(void)
{
    if (Plugin_information.Library == NULL)
    {
        initialize_plugin_pointer(&Plugin_information);
    }
}

static void
uninitialize_plugin(void)
{
    if (Plugin_information.Library != NULL)
    {
        plugin_unload(Plugin_information.Library);

        Plugin_information.Library = NULL;
    }
}

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

    initialize_plugin();

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        static const char usb[]="libusb/xu1541";
        FUNC_LEAVE_STRING(usb); 
    }
#endif

    FUNC_LEAVE_STRING(Plugin_information.Plugin.cbm_plugin_get_driver_name(PortNumber));
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

    initialize_plugin();

#ifdef ENABLE_XU1541
    if(xu1541_init() == 0)
        FUNC_LEAVE_INT(0); 
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_driver_open(HandleDevice, PortNumber));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle != 0) 
    {
        xu1541_close();
	FUNC_LEAVE();
    }
#endif

    Plugin_information.Plugin.cbm_plugin_driver_close(HandleDevice);

    uninitialize_plugin();

    FUNC_LEAVE();
}

/*! \brief Lock the parallel port for the driver

 This function locks the driver onto the parallel port. This way,
 no other program or driver can allocate the parallel port and
 interfere with the communication.

 \param HandleDevice  
   A CBM_FILE which contains the file handle of the driver.

 If cbm_driver_open() did not succeed, it is illegal to 
 call cbm_driver_close().

 \remark
 A call to cbm_lock() is undone with a call to cbm_unlock().

 Note that it is *not* necessary to call this function
 (or cbm_unlock()) when all communication is done with
 the handle to opencbm open (that is, between 
 cbm_driver_open() and cbm_driver_close(). You only
 need this function to pin the driver to the port even
 when cbm_driver_close() is to be executed (for example,
 because the program terminates).
*/

void CBMAPIDECL
cbm_lock(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

#ifdef ENABLE_XU1541
    if(xu1541_handle) return;
#endif

    Plugin_information.Plugin.cbm_plugin_lock(HandleDevice);

    FUNC_LEAVE();
}

/*! \brief Unlock the parallel port for the driver

 This function unlocks the driver from the parallel port.
 This way, other programs and drivers can allocate the
 parallel port and do their own communication with
 whatever device they use.

 \param HandleDevice  
   A CBM_FILE which contains the file handle of the driver.

 If cbm_driver_open() did not succeed, it is illegal to 
 call cbm_driver_close().

 \remark
 Look at cbm_lock() for an explanation of this function.
*/

void CBMAPIDECL
cbm_unlock(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

#ifdef ENABLE_XU1541
    if(xu1541_handle) return;
#endif

    Plugin_information.Plugin.cbm_plugin_unlock(HandleDevice);

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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT(xu1541_write(Buffer, Count));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_raw_write(HandleDevice,Buffer, Count));
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
    int bytesRead = 0;

    FUNC_ENTER();

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        bytesRead = xu1541_read(Buffer, Count);
    } 
    else
#endif
        bytesRead = Plugin_information.Plugin.cbm_plugin_raw_read(HandleDevice, Buffer, Count);

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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT(xu1541_ioctl(XU1541_LISTEN, DeviceAddress, SecondaryAddress));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_listen(HandleDevice, DeviceAddress, SecondaryAddress));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT(xu1541_ioctl(XU1541_TALK, DeviceAddress, SecondaryAddress));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_talk(HandleDevice, DeviceAddress, SecondaryAddress));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle)
	returnValue = xu1541_ioctl(XU1541_OPEN, DeviceAddress, SecondaryAddress);
    else
#endif
        returnValue = Plugin_information.Plugin.cbm_plugin_open(HandleDevice, DeviceAddress, SecondaryAddress);

    if (returnValue == 0)
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	FUNC_LEAVE_INT(xu1541_ioctl(XU1541_CLOSE, DeviceAddress, SecondaryAddress));
    }
#endif
 
    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_close(HandleDevice, DeviceAddress, SecondaryAddress));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT(xu1541_ioctl(XU1541_UNLISTEN, 0, 0));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_unlisten(HandleDevice));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT(xu1541_ioctl(XU1541_UNTALK, 0, 0));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_untalk(HandleDevice));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT(xu1541_ioctl(XU1541_GET_EOI, 0, 0));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_get_eoi(HandleDevice));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT(xu1541_ioctl(XU1541_CLEAR_EOI, 0, 0));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_clear_eoi(HandleDevice));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT(xu1541_ioctl(XU1541_RESET, 0, 0));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_reset(HandleDevice));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	FUNC_LEAVE_UCHAR((__u_char)xu1541_ioctl(XU1541_PP_READ, 0, 0));
    }
#endif

    FUNC_LEAVE_UCHAR(Plugin_information.Plugin.cbm_plugin_pp_read(HandleDevice));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	xu1541_ioctl(XU1541_PP_WRITE, Byte, 0);
	FUNC_LEAVE();
    }
#endif

    Plugin_information.Plugin.cbm_plugin_pp_write(HandleDevice, Byte);

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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	FUNC_LEAVE_INT(xu1541_ioctl(XU1541_IEC_POLL, 0, 0));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_iec_poll(HandleDevice));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	xu1541_ioctl(XU1541_IEC_SET, Line, 0);
	FUNC_LEAVE();
    }
#endif

    Plugin_information.Plugin.cbm_plugin_iec_set(HandleDevice, Line);

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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	xu1541_ioctl(XU1541_IEC_RELEASE, Line, 0);
	FUNC_LEAVE();
    }
#endif

    Plugin_information.Plugin.cbm_plugin_iec_release(HandleDevice, Line);

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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	xu1541_ioctl(XU1541_IEC_SET, Set, Release);
	FUNC_LEAVE();
    }
#endif

    Plugin_information.Plugin.cbm_plugin_iec_setrelease(HandleDevice, Set, Release);

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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	FUNC_LEAVE_INT(xu1541_ioctl(XU1541_IEC_WAIT, Line, State));
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_iec_wait(HandleDevice, Line, State));
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

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
        FUNC_LEAVE_INT((xu1541_ioctl(XU1541_IEC_POLL,0,0)&Line) != 0 ? 1 : 0);
    }
#endif

    FUNC_LEAVE_INT((Plugin_information.Plugin.cbm_plugin_iec_poll(HandleDevice)&Line) != 0 ? 1 : 0);
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

        if (cbm_talk(HandleDevice, DeviceAddress, 15) == 0)
        {
            int bytesRead = cbm_raw_read(HandleDevice, bufferToWrite, BufferLength);

            DBG_ASSERT(bytesRead >= 0);
            DBG_ASSERT(((unsigned int)bytesRead) <= BufferLength);

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
            Size = (size_t) strlen(Command);
        }
        rv = (size_t) cbm_raw_write(HandleDevice, Command, Size) != Size;
        cbm_unlisten(HandleDevice);
    }

    FUNC_LEAVE_INT(rv);
}

/*! \brief PARBURST: Read from the parallel port

 This function is a helper function for parallel burst:
 It reads from the parallel port.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \return
   The value read from the parallel port

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

__u_char CBMAPIDECL
cbm_parallel_burst_read(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	fprintf(stderr, "cbm_parallel_burst_read(): Not supported by xu1541\n");
	exit(-1);
    }
#endif

    FUNC_LEAVE_UCHAR(Plugin_information.Plugin.cbm_plugin_parallel_burst_read(HandleDevice));
}

/*! \brief PARBURST: Write to the parallel port

 This function is a helper function for parallel burst:
 It writes to the parallel port.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Value
   The value to be written to the parallel port

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

void CBMAPIDECL
cbm_parallel_burst_write(CBM_FILE HandleDevice, __u_char Value)
{
    FUNC_ENTER();

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	fprintf(stderr, "cbm_parallel_burst_write(): Not supported by xu1541\n");
	exit(-1);
    }
#endif

    Plugin_information.Plugin.cbm_plugin_parallel_burst_write(HandleDevice, Value);

    FUNC_LEAVE();
}

/*! \brief PARBURST: Read a complete track

 This function is a helper function for parallel burst:
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
cbm_parallel_burst_read_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length)
{
    FUNC_ENTER();

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	fprintf(stderr, "cbm_parallel_burst_read_track(): Not supported by xu1541\n");
	exit(-1);
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_parallel_burst_read_track(HandleDevice, Buffer, Length));
}

/*! \brief PARBURST: Write a complete track

 This function is a helper function for parallel burst:
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
cbm_parallel_burst_write_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length)
{
    FUNC_ENTER();

#ifdef ENABLE_XU1541
    if(xu1541_handle) 
    {
	fprintf(stderr, "cbm_parallel_burst_write_track(): Not supported by xu1541\n");
	exit(-1);
    }
#endif

    FUNC_LEAVE_INT(Plugin_information.Plugin.cbm_plugin_parallel_burst_write_track(HandleDevice, Buffer, Length));
}
