/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005,2007 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/plugin/xa1541/WINDOWS/archlib.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: archlib.c,v 1.1.2.2 2007-03-14 17:12:30 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver, windows specific code
**
****************************************************************/

#include <windows.h>
#include <windowsx.h>

#include <mmsystem.h>

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


/*! \internal \brief Start fast scheduling

 The cbm4win driver is very timing sensitive. Because of this, it is
 very important that the scheduling is done as fast as possible.

 Unfortunately, Windows has a varying scheduling granularity. Normally,
 it is in the order of 5 us to 20 us, but it can even grow much larger,
 for example, on an NT4 system or an an SMP or HT machine.

 fastschedule_start() uses a function from mmsystem.h which tells
 Windows to schedule with a much lower granularity. In this case, we ask
 Windows to use a granularity of 1 us.

 Note that this has a negative impact on the overall performance of the
 system, as more scheduling decisions have to be taken by the system.
 Anyway, it has a very positive impact for cbm4win.
 
 Note: Every call to fastschedule_start() has to be balanced with an
 appropriate call to fastschedule_stop() 
*/

static void
fastschedule_start(void)
{
    FUNC_ENTER();

    if (timeBeginPeriod(1) != TIMERR_NOERROR)
    {
        DBG_WARN((DBG_PREFIX "Unable to decrease scheduling period."));
    }

    FUNC_LEAVE();
}

/*! \internal \brief End fast scheduling

 For an explanation of this function, see fastschedule_start().
*/

static void
fastschedule_stop(void)
{
    FUNC_ENTER();

    if (timeEndPeriod(1) != TIMERR_NOERROR)
    {
        DBG_WARN((DBG_PREFIX "Unable to restore scheduling period."));
    }

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
opencbm_xa1541_init(IN HANDLE Module, IN DWORD Reason, IN LPVOID Reserved)
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
            }
            else
            {
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
            }

            WaitForIoCompletionInit();

            /* If the DLL loaded successfully, ask for fast scheduling */
            if (Status)
            {
                fastschedule_start();
            }
            break;

        case DLL_PROCESS_DETACH:

            if (IsDriverStartedAutomatically())
            {
                // the driver is started automatically, do not try
                // to stop it

                Status = TRUE;
            }
            else
            {
                if (!bIsOpen)
                {
                    DBG_ERROR((DBG_PREFIX "Driver is not running!"));
                    Status = FALSE;
                }
                else
                {
                    // it is arguable if the driver should be stopped
                    // whenever the DLL is unloaded.

                    cbm_i_driver_stop();
                    bIsOpen = FALSE;
                }
            }

            /* If the DLL unloaded successfully, we do not need fast scheduling anymore. */
            if (Status)
            {
                fastschedule_stop();
            }

            WaitForIoCompletionDeinit();
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


#if DBG

/*! \brief Output contents of the debugging buffer

 This function outputs the contents of the kernel-mode
 debugging buffer to the screen.

 This function is for use of the installation routines only!
*/

int CBMAPIDECL
cbm_get_debugging_buffer(CBM_FILE HandleDevice, char *buffer, size_t len)
{
    FUNC_ENTER();

    cbm_ioctl(HandleDevice, CBMCTRL(I_READDBG), NULL, 0, buffer, len);

    FUNC_LEAVE_INT(0);
}

#endif // #if DBG
