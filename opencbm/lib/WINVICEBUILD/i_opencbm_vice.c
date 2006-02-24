/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005 Spiro Trikaliotis
 *
 *  Parts are Copyright
 *      Jouko Valta <jopi(at)stekt(dot)oulu(dot)fi>
 *      Andreas Boose <boose(at)linux(dot)rz(dot)fh-hannover(dot)de>
*/

/*! ************************************************************** 
** \file lib/WINVICEBUILD/i_opencbm_vice.c \n
** \author Spiro Trikaliotis \n
** \version $Id: i_opencbm_vice.c,v 1.2 2006-02-24 12:21:41 strik Exp $ \n
** \authors Based on code from
**    Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
** \n
** \brief Helper functions for the DLL for accessing the driver,
**        and the install functions
**
****************************************************************/

#include <windows.h>
#include <windowsx.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#ifndef DBG_PROGNAME
    #define DBG_PROGNAME "OPENCBM.DLL"
#endif // #ifndef DBG_PROGNAME

#include "debug.h"

#include <winioctl.h>
#include "cbmioctl.h"

#include <stdlib.h>
#include <stddef.h>

#include "i_opencbm.h"

#include "version.h"
#include "archlib.h"

#include "vice_comm.h"

/*-------------------------------------------------------------------*/
/*--------- REGISTRY FUNCTIONS --------------------------------------*/

/*! \brief Get a DWORD value from the registry

 This function gets a DWORD value in the registry. It is a simple
 wrapper for convenience.

 \param RegKey
   A handle to an already opened registry key.

 \param SubKey
   Pointer to a null-terminiated string which holds the name
   of the value to be created or changed.

 \param Value
   Pointer to a variable which will contain the value from the registry

 \return
   ERROR_SUCCESS on success, -1 otherwise

 If this function returns -1, the given Value will not be changed at all!
*/

LONG
RegGetDWORD(IN HKEY RegKey, IN char *SubKey, OUT LPDWORD Value)
{
    DWORD valueLen;
    DWORD lpType;
    DWORD value;
    DWORD rc;

    FUNC_ENTER();

    FUNC_PARAM((DBG_PREFIX "Subkey = '%s'", SubKey));

    valueLen = sizeof(value);

    rc = RegQueryValueEx(RegKey, SubKey, NULL, &lpType, (LPBYTE)&value, &valueLen);

    DBG_ASSERT(valueLen == 4);

    if ((rc == ERROR_SUCCESS) && (valueLen == 4))
    {
        DBG_SUCCESS((DBG_PREFIX "RegGetDWORD"));
        *Value = value;
    }
    else
    {
        DBG_ERROR((DBG_PREFIX "RegGetDWORD failed, returning -1"));
        rc = -1;
    }

    FUNC_LEAVE_INT(rc);
}


/*! \internal \brief Get the number of the parallel port to open

 This function checks the registry for the number of the parallel port 
 to be opened as default.

 \return 
   Returns the number of the parallel port to be opened as default,
   starting with 0.

 If the registry entry does not exist, this function returns 0, which
 is also the default after installing the driver.
*/

static int
cbm_get_default_port(VOID)
{
    FUNC_ENTER();
    FUNC_LEAVE_INT(0);
}

#if DBG

/*! \brief Set the debugging flags

 This function gets the debugging flags from the registry. If there
 are any, it sets the flags to that value.
*/

VOID
cbm_i_get_debugging_flags(VOID)
{
    DWORD ret;
    HKEY RegKey;

    FUNC_ENTER();

    // Open a registry key to HKLM\<%REGKEY_SERVICE%>

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     CBM_REGKEY_SERVICE,
                     0,
                     KEY_QUERY_VALUE,
                     &RegKey)
       )
    {
        DBG_WARN((DBG_PREFIX "RegOpenKeyEx() failed!"));
        FUNC_LEAVE();
    }

    // now, get the number of the port to use

    if (RegGetDWORD(RegKey, CBM_REGKEY_SERVICE_DLL_DEBUGFLAGS, &ret) != ERROR_SUCCESS)
    {
        DBG_WARN((DBG_PREFIX "No " CBM_REGKEY_SERVICE "\\" CBM_REGKEY_SERVICE_DLL_DEBUGFLAGS
            " value, leaving default."));
    }
    else
    {
        DbgFlags = ret;
    }

    // We're done, close the registry handle.

    RegCloseKey(RegKey);

    FUNC_LEAVE();
}

#endif // #if DBG

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

const char *
cbmarch_get_driver_name(int PortNumber)
{
    //! \todo do not hard-code the driver name
    static char driverName[] = "opencbm-vice";

    FUNC_ENTER();

    FUNC_LEAVE_STRING(driverName);
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

int
cbmarch_driver_open(CBM_FILE *HandleDevice, int PortNumber)
{
    const unsigned char sys[] = "SYS8192\r";
    const unsigned char syslen = sizeof(sys)-1;

    const unsigned char buffer[] = { 
        0x4c, 0x00, 0x20    // 2000 JMP $2000
    };

    int success;

    FUNC_ENTER();

    success = viceinit();

    if (success)
    {
        vicepause();
        vicewritememory(0x2000, sizeof(buffer), buffer);
        viceresume();
        vicewaittrap();

        vicepause();
        vicewritememory(631, syslen, sys);
        viceresume();
        vicewaittrap();

        vicepause();
        vicewritememory(198, 1, &syslen);
        viceresume();
        vicewaittrap();
    }

    FUNC_LEAVE_INT(success ? 0 : 1);
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

void
cbmarch_driver_close(CBM_FILE HandleDevice)
{
    FUNC_ENTER();

    vicerelease();

    FUNC_LEAVE();
}
