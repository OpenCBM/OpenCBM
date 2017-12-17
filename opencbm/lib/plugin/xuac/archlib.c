/*
 *  xuac plugin interface, derived from the xu1541 file of the same name
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2009-2010 Nate Lawson
 *  Copyright 1999-2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005, 2007, 2010-2011 Spiro Trikaliotis
 *  Copyright 2010 Wolfgang Moser
 *  Copyright 2017 Arnd Menge
 *
*/

/*! ************************************************************** 
** \file lib/plugin/xuac/archlib.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \n
** \brief Shared library / DLL for accessing the driver, windows specific code
**
****************************************************************/

#ifdef WIN32
#include <windows.h>
#include <windowsx.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! Mark: We are building the DLL */
// #define DBG_DLL

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM-XUAC.DLL"

/*! This file is "like" debug.c, that is, define some variables */
// #define DBG_IS_DEBUG_C

#include "debug.h"
#endif

#include <stdlib.h>

//! mark: We are building the DLL */
#define OPENCBM_PLUGIN
#include "archlib_ex.h"

#include "xuac.h"


/*-------------------------------------------------------------------*/
/*--------- OPENCBM ARCH FUNCTIONS ----------------------------------*/

/*! \brief Get the name of the driver for a specific parallel port

 Get the name of the driver for a specific parallel port.

 \param Port
   The port specification for the driver to open. If not set (== NULL),
   the "default" driver is used. The exact meaning depends upon the plugin.

 \return 
   Returns a pointer to a null-terminated string containing the
   driver name, or NULL if an error occurred.

 \bug
   PortNumber is not allowed to exceed 255. 
*/

const char * CBMAPIDECL
opencbm_plugin_get_driver_name(const char * const Port)
{
    int portNumber = 0;
    
    if(Port != NULL) {
        portNumber = strtoul(Port, NULL, 10);
    }

    return xuac_device_path(portNumber);
}

/*! \brief Opens the driver

 This function Opens the driver.

 \param HandleDevice  
   Pointer to a CBM_FILE which will contain the file handle of the driver.

 \param Port
   The port specification for the driver to open. If not set (== NULL),
   the "default" driver is used. The exact meaning depends upon the plugin.

 \return 
   ==0: This function completed successfully
   !=0: otherwise

 PortNumber is not allowed to exceed 10. 

 cbm_driver_open() should be balanced with cbm_driver_close().
*/

int CBMAPIDECL
opencbm_plugin_driver_open(CBM_FILE *HandleDevice, const char * const Port)
{
    int portNumber = 0;
    
    if(Port != NULL) {
        portNumber = strtoul(Port, NULL, 10);
    }

    return xuac_init((usb_dev_handle **)HandleDevice, portNumber);
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
opencbm_plugin_driver_close(CBM_FILE HandleDevice)
{
    xuac_close((usb_dev_handle *)HandleDevice);
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
opencbm_plugin_reset(CBM_FILE HandleDevice)
{
    return xuac_control_msg((usb_dev_handle *)HandleDevice, XUAC_RESET);
}

/*! \brief Sends a command to the xuac device

 This function sends a control message respectively a command to the xuac device.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param cmd
   The command to run at the xuac device.

 \return
   Returns the value the USB device sent back.
*/

int CBMAPIDECL
xuac_plugin_control_msg(CBM_FILE HandleDevice, unsigned int cmd)
{
    return xuac_control_msg((usb_dev_handle *)HandleDevice, cmd);
}
