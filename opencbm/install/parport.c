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
** \file parport.c \n
** \author Spiro Trikaliotis \n
** \version $Id: parport.c,v 1.1 2004-11-07 11:05:12 strik Exp $ \n
** \n
** \brief Program to handle the parallel port for the OPENCBM driver
**
****************************************************************/


#include <windows.h>
#include <stdio.h>

#include "instcbm.h"

#include <setupapi.h>
#include <cfgmgr32.h>

#include <initguid.h>
#include <ntddpar.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "INSTCBM.EXE"

#include "debug.h"


/*! \internal \brief Output the error message for the last error
*/
static void
output_error(void)
{
    DWORD error = GetLastError();

    if (error)
    {
        DBG_PRINT((DBG_PREFIX "error: (0x%x) '%s'", error, FormatErrorMessage(error)));
        printf("error: (0x%x) '%s'\n", error, FormatErrorMessage(error));
    }
}

/*! Pointer to the CM_Get_Device_ID_Ex function.
 * As this function is not supported on NT4, we have to dynamically get its address,
 * because our driver would refuse to load on NT4 else.
 */
typedef CMAPI CONFIGRET (*P_CM_Get_Device_ID_ExA)(
    IN DEVINST dnDevInst, OUT PTCHAR Buffer, IN ULONG BufferLen,
    IN ULONG ulFlags, IN HMACHINE hMachine);

typedef WINSETUPAPI BOOL (*P_SetupDiGetDeviceInfoListDetailA)(
    IN HDEVINFO DeviceInfoSet, OUT PSP_DEVINFO_LIST_DETAIL_DATA  DeviceInfoSetDetailData);


typedef
struct {
    HMODULE                           HandleSetupApiDll;
    P_CM_Get_Device_ID_ExA            CM_Get_Device_ID_ExA_p;
    P_SetupDiGetDeviceInfoListDetailA SetupDiGetDeviceInfoListDetailA_p;
} SETUPAPI, *PSETUPAPI;

static VOID
FreeDynamicalAddresses(PSETUPAPI SetupApi)
{
    FUNC_ENTER();

    DBG_ASSERT(SetupApi->HandleSetupApiDll != NULL);

    FreeLibrary(SetupApi->HandleSetupApiDll);

    DBGDO(memset(SetupApi, 0, sizeof(*SetupApi));)

    FUNC_LEAVE();
}

#define GET_PROC_ADDRESS(_xxx) \
    if (SetupApi && SetupApi->HandleSetupApiDll) \
    { \
        SetupApi->_xxx##_p = (P_##_xxx) GetProcAddress(SetupApi->HandleSetupApiDll, #_xxx); \
        \
        DBG_PRINT((DBG_PREFIX "p_CM_Get_Device_ID_Ex = %p", SetupApi->_xxx##_p)); \
        \
        if (SetupApi->_xxx##_p == NULL) \
        { \
            DBG_PRINT((DBG_PREFIX "GetProcAddress(\"" #_xxx "\") FAILED!")); \
            FreeDynamicalAddresses(SetupApi); \
        } \
     }

static BOOLEAN
GetDynamicalAddresses(PSETUPAPI SetupApi)
{
    FUNC_ENTER();

    DBG_ASSERT(SetupApi->HandleSetupApiDll == NULL);
    DBG_ASSERT(SetupApi->CM_Get_Device_ID_ExA_p == NULL);

    SetupApi->HandleSetupApiDll = LoadLibrary("SETUPAPI.DLL");

    DBG_PRINT((DBG_PREFIX "SetupApi->HandleSetupApiDll = %p",
        SetupApi->HandleSetupApiDll));

    GET_PROC_ADDRESS(CM_Get_Device_ID_ExA);
    GET_PROC_ADDRESS(SetupDiGetDeviceInfoListDetailA);

    FUNC_LEAVE_BOOL(SetupApi->HandleSetupApiDll != NULL ? TRUE : FALSE);
}

/*! \brief Restart the parallel port

 This function tries to restart the parallel port, so that registry 
 changes can take effect.
*/
VOID
CbmParportRestart(VOID)
{
    HDEVINFO hdevInfo;
    SETUPAPI setupApi;

    FUNC_ENTER();

    DBGDO(memset(&setupApi, 0, sizeof(setupApi));)

    if (GetDynamicalAddresses(&setupApi))
    {
        DBG_ASSERT(setupApi.HandleSetupApiDll);
        DBG_ASSERT(setupApi.CM_Get_Device_ID_ExA_p);
        DBG_ASSERT(setupApi.SetupDiGetDeviceInfoListDetailA_p);

        // open all devices with a parallel port interface

        hdevInfo = SetupDiGetClassDevs(&GUID_PARALLEL_DEVICE, NULL, NULL,
                                       DIGCF_DEVICEINTERFACE | DIGCF_PRESENT | DIGCF_PROFILE);

        if (hdevInfo != INVALID_HANDLE_VALUE)
        {
            SP_DEVINFO_DATA sdd;
            unsigned i;

            // now, enumerate all parallel devices
            for (i=0; ; i++)
            {
                char deviceId[MAX_DEVICE_ID_LEN];

                SP_DEVINFO_LIST_DETAIL_DATA sdld;

                sdld.cbSize = sizeof(sdld);

                if (!setupApi.SetupDiGetDeviceInfoListDetailA_p(hdevInfo, &sdld))
                {
                    DBG_PRINT((DBG_PREFIX "SetupDiGetDeviceInfoListDetail FAILED!"));
                    printf("SetupDiGetDeviceInfoListDetail FAILED!\n");
                }       
            

                sdd.cbSize = sizeof(sdd);

                if (SetupDiEnumDeviceInfo(hdevInfo, i, &sdd))
                {
                    SP_DEVINSTALL_PARAMS devParams;
                    SP_PROPCHANGE_PARAMS spp;
                    HMACHINE machineHandle = { 0 };

                    if (setupApi.CM_Get_Device_ID_ExA_p(sdd.DevInst, deviceId, MAX_DEVICE_ID_LEN,
                        0, machineHandle) != CR_SUCCESS)
                    {
                        deviceId[0] = 0;
                    }
                    DBG_PRINT((DBG_PREFIX "No. %u: Returned deviceId = '%s'", i, deviceId));

                    spp.ClassInstallHeader.cbSize = sizeof(spp.ClassInstallHeader);
                    spp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
                    spp.StateChange = DICS_PROPCHANGE;
                    spp.Scope = DICS_FLAG_GLOBAL; // ignored
                    spp.HwProfile = 0;

                    if (SetupDiSetClassInstallParams(hdevInfo, &sdd,
                        (PSP_CLASSINSTALL_HEADER) &spp, sizeof(spp)))
                    {
                        DBG_PRINT((DBG_PREFIX "SET CLASS INSTALL PARAMS WAS SUCCESSFULL!"));
                    }
                    else
                    {
                        DBG_PRINT((DBG_PREFIX "SET CLASS INSTALL PARAMS NOT SUCCESSFULL"));
                        printf("SET CLASS INSTALL PARAMS NOT SUCCESSFULL!\n");
                        output_error();
                    }

                    if (SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hdevInfo, &sdd))
                    {
                        DBG_PRINT((DBG_PREFIX "CALL CLASS INSTALLER WAS SUCCESSFULL!"));
                    }
                    else
                    {
                        DBG_PRINT((DBG_PREFIX "CALL CLASS INSTALLER NOT SUCCESSFULL"));
                        printf("CALL CLASS INSTALLER NOT SUCCESSFULL!\n");
                        output_error();
                    }

                    if (SetupDiGetDeviceInstallParams(hdevInfo, &sdd, &devParams))
                    {
                        if (devParams.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))
                        {
                            DBG_PRINT((DBG_PREFIX "NEED REBOOT TO TAKE EFFECT!"));
                            printf("NEED REBOOT TO TAKE EFFECT!\n");
                        }
                    }
                }
                else
                {
                    if (GetLastError() == ERROR_NO_MORE_ITEMS)
                    {
                        break;
                    }
                }
            }
        }

        SetupDiDestroyDeviceInfoList(hdevInfo);

        FreeDynamicalAddresses(&setupApi);
    }
    else
    {
        DBG_PRINT((DBG_PREFIX "SetupDiGetClassDevs FAILED!"));
        printf("SetupDiGetClassDevs FAILED!");
        output_error();
    }

    FUNC_LEAVE();
}
