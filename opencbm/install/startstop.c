/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis <cbm4win@trikaliotis.net>
 *
 */

/*! ************************************************************** 
** \file startstop.c \n
** \author Spiro Trikaliotis \n
** \version $Id: startstop.c,v 1.4 2004-11-17 20:30:12 strik Exp $ \n
** \n
** \brief Functions for starting and stopping the driver
**
****************************************************************/


#include <windows.h>
#include <stdio.h>
#include "cbmioctl.h"

#include "instcbm.h"

#include "i_opencbm.h"

#include "version.h"

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "INSTCBM.EXE"

#include "cbmioctl.h"

#include "debug.h"

/*! \brief Output a path

 \param Text

   Pointer to a string that will be printed before
   the path string.

 \param Path

   Pointer to a string which contains the path.

*/
VOID
OutputPathString(IN PCHAR Text, IN PCHAR Path)
{
    FUNC_ENTER();

    DBG_PRINT((DBG_PREFIX "%s%s", Text, Path));
    printf("%s%s\n", Text, Path);

    FUNC_LEAVE();
}

/*! \brief Output a version string

 \param Text

   Pointer to a string that will be printed before
   the version string.

 \param Version

   A (coded) version information, as defined by
   CBMT_I_INSTALL_OUT_MAKE_VERSION()

*/
VOID
OutputVersionString(IN PCHAR Text, IN ULONG Version)
{
    char buffer[100];

    FUNC_ENTER();

    if (Version != 0)
    {
        _snprintf(buffer, sizeof(buffer)-1,
            CBMT_I_INSTALL_OUT_GET_VERSION_DEVEL(Version) ? "%u.%u.%u.%u (Development)" : "%u.%u.%u",
            (unsigned) CBMT_I_INSTALL_OUT_GET_VERSION_MAJOR(Version),
            (unsigned) CBMT_I_INSTALL_OUT_GET_VERSION_MINOR(Version),
            (unsigned) CBMT_I_INSTALL_OUT_GET_VERSION_SUBMINOR(Version),
            (unsigned) CBMT_I_INSTALL_OUT_GET_VERSION_DEVEL(Version));
    }
    else
    {
        _snprintf(buffer, sizeof(buffer)-1, "COULD NOT DETERMINE VERSION");
    }
    buffer[sizeof(buffer)] = 0;

    OutputPathString(Text, buffer);
//    DBG_PRINT((DBG_PREFIX "%s%s", Text, buffer));
//    printf("%s%s\n", Text, buffer);

    FUNC_LEAVE();
}

/*! \brief Check version information

 This function checks (and outputs) the version
 information for cbm4win.

 \return
    If mixed versions are found, or other errors occurred,
    this function returns TRUE.

*/
static BOOL
CheckVersions(PCBMT_I_INSTALL_OUT InstallOutBuffer)
{
    ULONG instcbmVersion;
    DWORD startMode;
    DWORD lptPort;
    char dllPath[MAX_PATH] = "<unknown>";
    char driverPath[MAX_PATH] = "<unknown>";
    BOOL error;

    FUNC_ENTER();

    error = FALSE;

    startMode = -1;
    lptPort = 0;

    // Try to find out the version and path of the DLL

    {
        HMODULE handleDll;

        // Try to load the DLL. If this succeeds, get the version information
        // from there

        handleDll = LoadLibrary("OPENCBM.DLL");

        if (handleDll)
        {
            P_CBM_I_DRIVER_INSTALL p_cbm_i_driver_install;
            CBMT_I_INSTALL_OUT dllInstallOutBuffer;
            DWORD length;

            memset(&dllInstallOutBuffer, 0, sizeof(dllInstallOutBuffer));

            p_cbm_i_driver_install = 
                (P_CBM_I_DRIVER_INSTALL) GetProcAddress(handleDll, "cbm_i_driver_install");

            if (p_cbm_i_driver_install)
            {
                p_cbm_i_driver_install((PULONG) &dllInstallOutBuffer, sizeof(dllInstallOutBuffer));

                if (InstallOutBuffer->DriverVersion == dllInstallOutBuffer.DriverVersion)
                {
                    InstallOutBuffer->DllVersion = dllInstallOutBuffer.DllVersion;
                }
                else
                {
                    error = TRUE;
                }
            }
            else
            {
                error = TRUE;
            }

            length = GetModuleFileName(handleDll, dllPath, sizeof(dllPath));

            if (length >= sizeof(dllPath))
            {
                dllPath[sizeof(dllPath)] = 0;
            }

            FreeLibrary(handleDll);
        }
        else
        {
            error = TRUE;
        }
    }

    // Try to find the path to the driver

    {
        HKEY regKey;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         CBM_REGKEY_SERVICE,
                         0,
                         KEY_QUERY_VALUE,
                         &regKey)
           )
        {
            DBG_WARN((DBG_PREFIX "RegOpenKeyEx() failed!"));
            error = TRUE;
        }
        else
        {
            DWORD regLength;
            DWORD regReturn;
            DWORD regType;
            char driverPathFromRegistry[MAX_PATH];

            // now, get the number of the port to use

            regLength = sizeof(driverPathFromRegistry);

            regReturn = RegQueryValueEx(regKey, "ImagePath", NULL, &regType, 
                (LPBYTE)&driverPathFromRegistry, &regLength);

            if (regReturn != ERROR_SUCCESS)
            {
                error = TRUE;

                DBG_ERROR((DBG_PREFIX "No HKLM\\" CBM_REGKEY_SERVICE "\\ImagePath"
                    " value: %s", FormatErrorMessage(regReturn)));
            }
            else
            {
                char *pColon;

                // Make sure there is a trailing zero

                driverPathFromRegistry[sizeof(driverPathFromRegistry)] = 0;

                // Find out if there is a colon ":" in the path

                pColon = strchr(driverPathFromRegistry, ':');

                if (pColon)
                {
                    // There is a colon, that is, the path is absolute

                    if (strncmp(driverPathFromRegistry, "\\??\\", sizeof("\\??\\")-1) == 0)
                    {
                        // There is the \??\ prefix, skip that

                        strncpy(driverPath, &driverPathFromRegistry[sizeof("\\??\\")-1],
                            sizeof(driverPath));
                    }
                    else
                    {
                        strncpy(driverPath, driverPathFromRegistry, sizeof(driverPath));
                    }
                }
                else
                {
                    int lengthString;

                    // There is no colon, that is, the path is relative (to the windows directory)
                    // Thus, make sure the windows directory is appended in front of it

                    GetWindowsDirectory(driverPath, sizeof(driverPath));
                    lengthString = strlen(driverPath);

                    if (lengthString < sizeof(driverPath))
                    {
                        strncat(&driverPath[lengthString], "\\", sizeof(driverPath)-lengthString);
                        ++lengthString;

                        strncat(&driverPath[lengthString], driverPathFromRegistry,
                            sizeof(driverPath)-lengthString);
                    }
                }
            }

            // find out the start mode of the driver

            RegGetDWORD(regKey, "Start", &startMode);

            // find out the default lpt port of the driver

            RegGetDWORD(regKey, CBM_REGKEY_SERVICE_DEFAULTLPT, &lptPort);

            // We're done, close the registry handle.

            RegCloseKey(regKey);
        }
    }

    // Print out the configuration we just obtained

    printf("\n\nThe following configuration is used:\n\n");

    instcbmVersion = 
        CBMT_I_INSTALL_OUT_MAKE_VERSION(CBM4WIN_VERSION_MAJOR,
                                        CBM4WIN_VERSION_MINOR,
                                        CBM4WIN_VERSION_SUBMINOR,
                                        CBM4WIN_VERSION_DEVEL);

    OutputVersionString("INSTCBM version: ", instcbmVersion);

    OutputVersionString("Driver version:  ", InstallOutBuffer->DriverVersion);
    OutputPathString   ("Driver path:     ", driverPath);
    OutputVersionString("DLL version:     ", InstallOutBuffer->DllVersion);
    OutputPathString   ("DLL path:        ", dllPath);

    printf("\n");

    if ( (InstallOutBuffer->DriverVersion != InstallOutBuffer->DllVersion)
        || (instcbmVersion != InstallOutBuffer->DllVersion)
        || (instcbmVersion != InstallOutBuffer->DriverVersion))
    {
        error = TRUE;
        printf("There are mixed versions, THIS IS NOT RECOMMENDED!\n\n");
    }

    printf("Driver configuration:\n");
    DBG_PRINT((DBG_PREFIX "Driver configuration:"));

    printf("  Default port: ........ LPT%i\n", lptPort ? lptPort : 1);
    DBG_PRINT((DBG_PREFIX "  Default port: ........ LPT%i", lptPort ? lptPort : 1));

    {
        const char *startModeName;

        switch (startMode)
        {
            case -1:
                startModeName = "NO ENTRY FOUND!";
                break; 

            case SERVICE_BOOT_START:
                startModeName = "boot";
                break;

            case SERVICE_SYSTEM_START:
                startModeName = "system";
                break;

            case SERVICE_AUTO_START:
                startModeName = "auto";
                break;

            case SERVICE_DEMAND_START:
                startModeName = "demand";
                break;

            case SERVICE_DISABLED:
                startModeName = "disabled";
                break;

            default:
                startModeName = "<UNKNOWN>";
                break;
        }

        printf("  Driver start mode: ... %s (%i)\n\n", startModeName, startMode);
        DBG_PRINT((DBG_PREFIX "  Driver start mode: ... %s (%i)", startModeName,
            startMode));
    }

    FUNC_LEAVE_BOOL(error);
}
/*! \brief Check if the driver was correctly installed

 This function checks if the driver was correctly installed.

 \return
   FALSE on success, TRUE on error,

 This function opens the opencbm driver, tests if anything is
 ok - especially, if the interrupt could be obtained - and reports
 this status.

 If there was no interrupt available, it tries to enable it
 on the parallel port.
*/

BOOL
CbmCheckCorrectInstallation(VOID)
{
    CBMT_I_INSTALL_OUT outBuffer;
    BOOL error;
    int tries;

    FUNC_ENTER();

    memset(&outBuffer, 0, sizeof(outBuffer));

    for (tries = 1; tries >= 0; --tries)
    {
        error = cbm_i_driver_start() ? FALSE : TRUE;

        if (error)
        {
            DBG_PRINT((DBG_PREFIX "Driver or DLL not correctly installed."));
            printf("Driver or DLL not correctly installed.\n");
            break;
        }
        else
        {
            error = cbm_i_i_driver_install((PULONG) &outBuffer, sizeof(outBuffer));
            outBuffer.DllVersion = 0;

            cbm_i_driver_stop();

            if (error)
            {
                DBG_PRINT((DBG_PREFIX "Driver problem: Could not check install."));
                printf("Driver problem: Could not check install.\n");
                break;
            }

            // did we fail to gather an interrupt?

            if (outBuffer.ErrorFlags & CBM_I_DRIVER_INSTALL_0M_NO_INTERRUPT)
            {
                if (tries > 0)
                {
                    // No IRQ available: Try to restart the parallel port to enable it.
                    printf("Please wait some seconds...\n");
                    CbmParportRestart();
                }
                else
                {
                    DBG_PRINT((DBG_PREFIX "No interrupt available."));
                    printf("Could not get an interrupt. Please try again after a reboot.\n");
                }
            }
            else
            {
                // no problem, we can stop the loop

                break;
            }
        }
    }

    // If the driver is set to be started automatically, restart it now

    if (IsDriverStartedAutomatically())
    {
        cbm_i_driver_start();
    }

    if (CheckVersions(&outBuffer))
    {
        error = TRUE;
    }

    FUNC_LEAVE_BOOL(error);
}
