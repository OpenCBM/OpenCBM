/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2006 Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file sys/libcommon/lockunlock.c \n
** \author Spiro Trikaliotis \n
** \version $Id: lockunlock.c,v 1.1 2006-03-08 17:27:24 strik Exp $ \n
** \n
** \brief Functions for locking und unlocking the driver onto the parallel port
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"

#include "version.h"

/*! \brief Lock the parallel port for the driver

 This functions locks the driver onto the parallel port. This way,
 no other program or driver can allocate the parallel port and
 interfere with the communication.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
 
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

NTSTATUS
cbm_lock(IN PDEVICE_EXTENSION Pdx)
{
    FUNC_ENTER();

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}

/*! \brief Unlock the parallel port for the driver

 This functions unlocks the driver from the parallel port.
 This way, other programs and drivers can allocate the
 parallel port and do their own communication with
 whatever device they use.

 \param Pdx
   Pointer to the device extension.

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. Otherwise, it
   returns one of the error status values.
 
 \remark
 Look at cbm_lock() for an explanation of this function.
*/

NTSTATUS
cbm_unlock(IN PDEVICE_EXTENSION Pdx)
{
    FUNC_ENTER();

    FUNC_LEAVE_NTSTATUS_CONST(STATUS_SUCCESS);
}
