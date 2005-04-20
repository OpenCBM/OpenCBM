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
** \file sys/libcommon/openclose.c \n
** \author Spiro Trikaliotis \n
** \version $Id: openclose.c,v 1.3 2005-04-20 14:24:07 strik Exp $ \n
** \n
** \brief Functions for opening and closing the driver
**
****************************************************************/

#include <wdm.h>
#include "cbm_driver.h"
#include "iec.h"

/*! \brief Services IRPs containing the IRP_MJ_CREATE or IRP_MJ_CLOSE I/O function code.

 Services IRPs containing the IRP_MJ_CREATE or IRP_MJ_CLOSE I/O function code.

 \param Fdo
   Pointer to a DEVICE_OBJECT structure. 
   This is the device object for the target device, 
   previously created by the driver's AddDevice routine.

 \param Irp
   Pointer to an IRP structure that describes the requested I/O operation. 

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. 
   Otherwise, it returns one of the error status values.

 The driver's DriverEntry routine has stored this routine's address 
 in DriverObject->MajorFunction[IRP_MJ_CREATE] and
 DriverObject->MajorFunction[IRP_MJ_CLOSE].

 Generally, all Dispatch routines execute in an arbitrary thread 
 context at IRQL PASSIVE_LEVEL, but there are exceptions.
*/
NTSTATUS
cbm_createopenclose(IN PDEVICE_OBJECT Fdo, IN PIRP Irp)
{
    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION pdx;
    NTSTATUS ntStatus;

    FUNC_ENTER();

    // get the device extension

    pdx = Fdo->DeviceExtension;

    DBG_IRPPATH_PROCESS("create/open");

    // get the current IRP stack location

    irpSp = IoGetCurrentIrpStackLocation(Irp);


    if (irpSp->MajorFunction == IRP_MJ_CREATE
        && irpSp->Parameters.Create.Options & FILE_DIRECTORY_FILE) 
    {
        // The caller wants to open a directory.
        // As we do not support directories, fail that request!

        ntStatus = STATUS_NOT_A_DIRECTORY;
        DBG_IRPPATH_COMPLETE("create/open");
        QueueCompleteIrp(NULL, Irp, ntStatus, 0);
    }
    else
    {
        // Initialize the settings from the registry. This allows
        // us to change the registry settings

        /*! \todo This should not be needed if the driver can be
         * reloaded!
         */

        cbm_init_registry(NULL, pdx);

        switch (irpSp->MajorFunction)
        {
        case IRP_MJ_CREATE:
            PERF_EVENT_OPEN_QUEUE();
            break;

        case IRP_MJ_CLOSE:
            PERF_EVENT_CLOSE_QUEUE();
            break;

        default:
            DBG_ERROR((DBG_PREFIX "UNKNOWN IRP->MAJORFUNCTION: %08x",
                irpSp->MajorFunction));
            break;
        }

        // queue the IRP to be processed

        ntStatus = QueueStartPacket(&pdx->IrpQueue, Irp, FALSE, Fdo);
    }
    
    FUNC_LEAVE_NTSTATUS(ntStatus);
}

/*! \brief Execute IRPs containing the IRP_MJ_CREATEOPEN I/O function code.

 Executes IRPs containing the IRP_MJ_CREATEOPEN I/O function code.

 \param Pdx
   Pointer to a DEVICE_EXTENSION structure.

 \param Irp
   Pointer to an IRP structure that describes the requested I/O operation. 

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. 
   Otherwise, it returns one of the error status values.
*/
NTSTATUS
cbm_execute_createopen(IN PDEVICE_EXTENSION Pdx, IN PIRP Irp)
{
    NTSTATUS ntStatus;

    FUNC_ENTER();

    DBG_ASSERT(Pdx != NULL);

    PERF_EVENT_OPEN_EXECUTE();

    DBG_IRPPATH_EXECUTE("create/open");

    // Try to allocate the parallel port

    ntStatus = ParPortAllocate(Pdx);

    // Set the appropriate mode of the parallel port
    // Normally, this will be either BYTE MODE, or SPP

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = ParPortSetMode(Pdx);
    }

    // Try to allocate the interrupt

    if (NT_SUCCESS(ntStatus))
    {
        /*! \todo
         * As we will try to cope without interrupt,
         * do not handle it as an open failure if we
         * do not succeed!
         */

        // ntStatus =
        ParPortAllocInterrupt(Pdx, cbm_isr);
    }

    // Initialize the IEC serial port

    if (NT_SUCCESS(ntStatus))
    {
        cbmiec_init(Pdx);
    }

    // Did we fail any call? If yes, free and release
    // any resource we might happen to have allocated
    // before we failed


    if (!NT_SUCCESS(ntStatus))
    {
        // The functions themselves test if the resource
        // is allocated, thus, we do not need to protect
        // against freeing non-allocated resources here.

        ParPortFreeInterrupt(Pdx);
        ParPortUnsetMode(Pdx);
        ParPortFree(Pdx);
    }

    // release the bus (to be able to share it with other
    // controllers

    cbmiec_release_bus(Pdx);

    // We're done, complete the IRP

    DBG_IRPPATH_COMPLETE("create/open");

    QueueCompleteIrp(&Pdx->IrpQueue, Irp, ntStatus, 0);

    FUNC_LEAVE_NTSTATUS(ntStatus);
}

/*! \brief Execute IRPs containing the IRP_MJ_CLOSE I/O function code.

 Executes IRPs containing the IRP_MJ_CLOSE I/O function code.

 \param Pdx
   Pointer to a DEVICE_EXTENSION structure.

 \param Irp
   Pointer to an IRP structure that describes the requested I/O operation. 

 \return 
   If the routine succeeds, it returns STATUS_SUCCESS. 
   Otherwise, it returns one of the error status values.
*/
NTSTATUS
cbm_execute_close(IN PDEVICE_EXTENSION Pdx, IN PIRP Irp)
{
    NTSTATUS ntStatus;

    FUNC_ENTER();

    DBG_ASSERT(Pdx != NULL);

    PERF_EVENT_CLOSE_EXECUTE();

    DBG_IRPPATH_PROCESS("close");

    ntStatus = STATUS_SUCCESS;

    // release the bus (to be able to share it with other controllers)

    cbmiec_release_bus(Pdx);

    // release all resources we have previously allocated

    ParPortFreeInterrupt(Pdx);
    ParPortUnsetMode(Pdx);
    ParPortFree(Pdx);

    // Now, complete the IRP

    DBG_IRPPATH_COMPLETE("close");
    QueueCompleteIrp(&Pdx->IrpQueue, Irp, ntStatus, 0);

    FUNC_LEAVE_NTSTATUS(ntStatus);
}
