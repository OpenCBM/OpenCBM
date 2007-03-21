/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2004,2007 Spiro Trikaliotis
 *
 *  Parts are Copyright
 *      Jouko Valta <jopi(at)stekt(dot)oulu(dot)fi>
 *      Andreas Boose <boose(at)linux(dot)rz(dot)fh-hannover(dot)de>
*/

/*! ************************************************************** 
** \file lib/WINBUILD/i_opencbm.c \n
** \author Spiro Trikaliotis \n
** \version $Id: i_opencbm.c,v 1.14.2.2 2007-03-21 16:06:06 strik Exp $ \n
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
    DWORD ret;
    HKEY regKey;

    FUNC_ENTER();

    DBG_PPORT((DBG_PREFIX "cbm_get_default_port()"));

    // Open a registry key to HKLM\<%REGKEY_SERVICE%>

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     CBM_REGKEY_SERVICE,
                     0,
                     KEY_QUERY_VALUE,
                     &regKey)
       )
    {
        DBG_WARN((DBG_PREFIX "RegOpenKeyEx() failed!"));
        FUNC_LEAVE_BOOL(FALSE);
    }

    // now, get the number of the port to use

    if (RegGetDWORD(regKey, CBM_REGKEY_SERVICE_DEFAULTLPT, &ret) != ERROR_SUCCESS)
    {
        DBG_WARN((DBG_PREFIX "No " CBM_REGKEY_SERVICE "\\" CBM_REGKEY_SERVICE_DEFAULTLPT 
            " value, setting 0."));
        ret = 0;
    }

    // We're done, close the registry handle.

    RegCloseKey(regKey);

    DBG_PPORT((DBG_PREFIX "RETURN: cbm_get_default_port() == %u", ret));

    FUNC_LEAVE_INT(ret);
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

/*-------------------------------------------------------------------*/
/*--------- ASYNCHRONOUS IO FUNCTIONS -------------------------------*/

/*! A special "cancel event" which is signalled whenever we want to
 * prematurely cancel an I/O request  */

static HANDLE CancelEvent = NULL;
static HANDLE CancelCallbackEvent = NULL;
static ULONG  InCancellableState  = 0;

/*! \brief Initialize WaitForIoCompletion()

 This function initializes everything needed for the 
 WaitForIoCompletion...() functions. It has to be called
 exactly once for each program start.
*/

VOID
WaitForIoCompletionInit(VOID)
{
    FUNC_ENTER();

    //
    // Create the events for prematurely cancelling I/O request
    //

    CancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    DBG_ASSERT(CancelEvent != NULL);

    CancelCallbackEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    DBG_ASSERT(CancelCallbackEvent != NULL);

    FUNC_LEAVE();
}

/*! \brief Uninitialize WaitForIoCompletion()

 This function uninitializes everything needed for the 
 WaitForIoCompletion...() functions. It has to be called
 exactly once for each program stop.

 \remark
   Make sure there are no outstanding I/O requests when this
   function is called!
*/

VOID
WaitForIoCompletionDeinit(VOID)
{
    FUNC_ENTER();

    //
    // Delete the event which is used for prematurely 
    // cancelling I/O request
    //
    if (CancelEvent != NULL)
        CloseHandle(CancelEvent);

    FUNC_LEAVE();
}

/*! \brief Cancel any running WaitForIoCompletion()

 This function cancels the running WaitForIoCompletion()
 function.
*/

VOID
WaitForIoCompletionCancelAll(VOID)
{
    FUNC_ENTER();

    if (InterlockedExchange(&InCancellableState, 0) != 0)
    {
        //
        // signal the event which is used for prematurely 
        // cancelling I/O request
        //

        SetEvent(CancelEvent);

        //
        // Wait to be signalled that the current I/O request
        // has been cancelled.
        //

        WaitForSingleObject(CancelCallbackEvent, INFINITE);
    }

    FUNC_LEAVE();
}

/*! \brief Boilerplate code for asynchronous I/O requests

 This function initializes 

 \param Overlapped
   Pointer to an OVERLAPPED structure that will be initialized.

 \remark
   This function completely initializes an OVERLAPPED structure
   to be used with ReadFile(), WriteFile, DeviceIoControl()
   later, and waited for with WaitForIoCompletion().
*/

VOID
WaitForIoCompletionConstruct(LPOVERLAPPED Overlapped)
{
    FUNC_ENTER();

    memset(Overlapped, 0, sizeof(*Overlapped));
    Overlapped->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    DBG_ASSERT(Overlapped->hEvent != NULL);

    FUNC_LEAVE();
}

/*! \brief Wait for the completion of an I/O operation

 This function waits until an I/O operation is completed,
 or cancelled.

 \param Result
   The result of the previous I/O operation 
   (ReadFile(), WriteFile(), DeviceIoControl())

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Overlapped
   Pointer to an OVERLAPPED structure that was specified when the
   overlapped operation was started.

 \param BytesTransferred
   Pointer to a DWORD which will contain the number of bytes
   transferred in this asynchronous I/O operation.

 \return 
   FALSE if a failure occurred, TRUE if success.

 \remark
   A cancelled request is considered as a failure, thus, FALSE
   is returned in this case.
*/

BOOL
WaitForIoCompletion(BOOL Result, CBM_FILE HandleDevice, LPOVERLAPPED Overlapped, DWORD *BytesTransferred)
{
    BOOL result = Result;
    HANDLE handleList[2] = { CancelEvent, Overlapped->hEvent };

    FUNC_ENTER();

    DBG_ASSERT(Overlapped != NULL);
    DBG_ASSERT(BytesTransferred != NULL);

    if (!Result)
    {
        // deal with the error code 
        switch (GetLastError()) 
        { 
            case ERROR_IO_PENDING:
            {
                int tmp;

                //
                // Make sure WaitForIoCompletionCancelAll() knows it has to signal us
                //

                tmp = InterlockedExchange(&InCancellableState, 1);
                DBG_ASSERT(tmp == 0);

                //
                // wait for the operation to finish
                //

                if (WaitForMultipleObjects(2, handleList, FALSE, INFINITE) == WAIT_OBJECT_0)
                {
                    CancelIo(HandleDevice);

                    // we are told to cancel this event
                    *BytesTransferred = 0;
                    result = FALSE;
                    SetEvent(CancelCallbackEvent);
                }
                else
                {
                    //
                    // WaitForIoCompletionCancelAll() does not need to alert us anymore
                    //

                    if (InterlockedExchange(&InCancellableState, 0) == 0)
                    {
                        //
                        // In case we were signalled, make sure 
                        // WaitForIoCompletionCancelAll() does not hang
                        //

                        SetEvent(CancelCallbackEvent);
                    }

                    // check on the results of the asynchronous read 
                    result = GetOverlappedResult(HandleDevice, Overlapped, 
                        BytesTransferred, FALSE) ; 
                }
                break;
            }
         }
    }

    CloseHandle(Overlapped->hEvent);

    FUNC_LEAVE_BOOL(result ? TRUE : FALSE);
}

/*-------------------------------------------------------------------*/
/*--------- OPENCBM ARCH FUNCTIONS ----------------------------------*/

/*! \brief Perform an ioctl on the driver

 This function performs an ioctl on the driver. 
 It is used internally only.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param ControlCode
   The ControlCode of the IOCTL to be performed.

 \param TextControlCode
   A string representation of the IOCTL to be performed. This is
   used for debugging purposes, only, and not available in
   free builds.

 \param InBuffer
   Pointer to a buffer which holds the input parameters for the
   IOCTL. Can be NULL if no input buffer is needed.

 \param InBufferSize
   Size of the buffer pointed to by InBuffer. If InBuffer is NULL,
   this has to be zero,

 \param OutBuffer
   Pointer to a buffer which holds the output parameters of the
   IOCTL. Can be NULL if no output buffer is needed.

 \param OutBufferSize
   Size of the buffer pointed to by OutBuffer. If OutBuffer is NULL,
   this has to be zero,

 \return
   TRUE: IOCTL succeeded, else
   FALSE  an error occurred processing the IOCTL

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

BOOL 
cbm_ioctl(IN CBM_FILE HandleDevice, IN DWORD ControlCode, 
#if DBG
          IN char *TextControlCode, 
#endif // #if DBG
          IN PVOID InBuffer, IN ULONG InBufferSize, OUT PVOID OutBuffer, IN ULONG OutBufferSize)
{
    OVERLAPPED overlapped;
    DWORD dwBytesReturned;

    BOOL returnValue;

    FUNC_ENTER();

    WaitForIoCompletionConstruct(&overlapped);

    // Perform the IOCTL

    returnValue = DeviceIoControl(HandleDevice, ControlCode, InBuffer, InBufferSize,
        OutBuffer, OutBufferSize, &dwBytesReturned, &overlapped);

    returnValue = WaitForIoCompletion(returnValue, HandleDevice, &overlapped, &dwBytesReturned);

    // If an error occurred, output it (in the DEBUG version of this file

    if (!returnValue)
    {
        DBG_ERROR((DBG_PREFIX "%s: Error code = %u", TextControlCode, GetLastError()));
    }
    else
    {
        // Check if the number of bytes returned equals the wanted number

        if (dwBytesReturned != OutBufferSize) 
        {
            DBG_WARN((DBG_PREFIX "%s: OutBufferSize = %u, but dwBytesReturned = %u",
                TextControlCode, OutBufferSize, dwBytesReturned));
        }
    }

    FUNC_LEAVE_BOOL(returnValue);
}
