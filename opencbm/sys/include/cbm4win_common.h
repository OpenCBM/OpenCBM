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
** \file sys/include/cbm4win_common.h \n
** \author Spiro Trikaliotis \n
** \version $Id: cbm4win_common.h,v 1.2.2.1 2005-04-09 15:10:56 strik Exp $ \n
** \n
** \brief Definitions for the cbm4win driver
**
****************************************************************/

/* Include any configuration definitions */
#include "config.h"

/*! make sure that ECR_OFFSET and the like use 0x02, and not 0x0402
    in parallel.h */
#define DVRH_USE_PARPORT_ECP_ADDR 1

#include <parallel.h>

/*! Debugging is for the kernel-mode */
#define DBG_KERNELMODE

/*! Name of the executable to be debugged */
#define DBG_PROGNAME "CBM4WIN.SYS"

#include "debug.h"

#include "cbmioctl.h"
#include "memtags.h"
#include "util.h"
#include "debug.h"

#include "queue.h"

#include "cbmlog.h"

#include "perfeval.h"

/* The following defines are used if performance evaluation is selected. */

/*! Performance evaluation: StartIo() is performed */
#define PERF_EVENT_STARTIO(_x_)         PERF_EVENT(0x10, (ULONG) _x_)
/*! Performance evaluation: An IRP is completed */
#define PERF_EVENT_COMPLETEIRP(_x_)     PERF_EVENT(0x11, (ULONG) _x_)
/*! Performance evaluation: An IRP is cancelled */
#define PERF_EVENT_CANCELIRP(_x_)       PERF_EVENT(0x12, (ULONG) _x_)

/*! Performance evaluation: An IOCTL is queued */
#define PERF_EVENT_IOCTL_QUEUE(_x_)     PERF_EVENT(0x20, _x_)
/*! Performance evaluation: An IOCTL is executed */
#define PERF_EVENT_IOCTL_EXECUTE(_x_)   PERF_EVENT(0x21, _x_)
/*! Performance evaluation: An OPEN request is queued */
#define PERF_EVENT_OPEN_QUEUE()         PERF_EVENT(0x30, 0)
/*! Performance evaluation: An OPEN request is executed */
#define PERF_EVENT_OPEN_EXECUTE()       PERF_EVENT(0x31, 0)
/*! Performance evaluation: An CLOSE request is queued */
#define PERF_EVENT_CLOSE_QUEUE()        PERF_EVENT(0x40, 0)
/*! Performance evaluation: An CLOSE request is executed */
#define PERF_EVENT_CLOSE_EXECUTE()      PERF_EVENT(0x41, 0)
/*! Performance evaluation: A READ request is queued */
#define PERF_EVENT_READ_QUEUE(_x_)      PERF_EVENT(0x50, _x_)
/*! Performance evaluation: A READ request is executed */
#define PERF_EVENT_READ_EXECUTE(_x_)    PERF_EVENT(0x51, _x_)
/*! Performance evaluation: A WRITE request is queued */
#define PERF_EVENT_WRITE_QUEUE(_x_)     PERF_EVENT(0x60, _x_)
/*! Performance evaluation: A WRITE request is executed */
#define PERF_EVENT_WRITE_EXECUTE(_x_)   PERF_EVENT(0x61, _x_)

/*! Performance evaluation: Schedule a start of the thread */
#define PERF_EVENT_THREAD_START_SCHED() PERF_EVENT(0x100, 0)
/*! Performance evaluation: Thread is actually started */
#define PERF_EVENT_THREAD_START_EXEC()  PERF_EVENT(0x100, 1)
/*! Performance evaluation: Thread polls for IRPs */
#define PERF_EVENT_THREAD_POLL()        PERF_EVENT(0x100, 2)
/*! Performance evaluation: Schedule a stop of the thread */
#define PERF_EVENT_THREAD_STOP_SCHED()  PERF_EVENT(0x100, 3)
/*! Performance evaluation: Thread is actually stopped */
#define PERF_EVENT_THREAD_STOP_EXEC()   PERF_EVENT(0x100, 4)

/*! Performance evaluation: Reading byte no _x_ */
#define PERF_EVENT_READ_BYTE_NO(_x_)    PERF_EVENT(0x200, _x_)
/*! Performance evaluation: Read a byte _x_ */
#define PERF_EVENT_READ_BYTE(_x_)       PERF_EVENT(0x201, _x_)
/*! Performance evaluation: Reading bit no. _x_ */
#define PERF_EVENT_READ_BIT_NO(_x_)     PERF_EVENT(0x202, _x_)
/*! Performance evaluation: Writing byte no _x_ */
#define PERF_EVENT_WRITE_BYTE_NO(_x_)   PERF_EVENT(0x210, _x_)
/*! Performance evaluation: Writing byte _x_ */
#define PERF_EVENT_WRITE_BYTE(_x_)      PERF_EVENT(0x211, _x_)
/*! Performance evaluation: Writing bit no. _x_ */
#define PERF_EVENT_WRITE_BIT_NO(_x_)    PERF_EVENT(0x212, _x_)

/*! The name if the driver. This name gets a number appended, starting with 0. */
#define CBMDEVICENAME L"\\DosDevices\\opencbm"

/*! The device extension for the device */
typedef
struct _DEVICE_EXTENSION {

    /* Windows book keeping: */

    /*! Pointer to the functional device object */
    PDEVICE_OBJECT Fdo;

    /*! Are we running on an NT4.0 system? */
    BOOLEAN IsNT4;

    /*! Pointer to the lower device object *
     * \todo Only for WDM driver PDEVICE_OBJECT LowerDeviceObject;
     */

    /*! The name of this device */
    UNICODE_STRING DeviceName;

    /*! QUEUE structure which allows us queueing our IRPs */
    QUEUE IrpQueue;

    /*! The FILE_OBJECT of the parallel port driver */
    PFILE_OBJECT ParallelPortFileObject;

    /*! The DEVICE_OBJECT of the parallel port driver */
    PDEVICE_OBJECT ParallelPortFdo;

    /*! Information about the port we are connected to */
    PPARALLEL_PORT_INFORMATION PortInfo;

    /*! Info for the ISR routine */
    PARALLEL_INTERRUPT_SERVICE_ROUTINE Pisr;

    /*! Return value after connection the ISR */
    PARALLEL_INTERRUPT_INFORMATION Pii;

    /*! how many Irqs should we wait in cbmiec_interrupt
     * (for cbmiec_wait_for_listener()? This determines
     * if we want to schedule an EOI (=2) or not (=1).
     * =0 means that the interrupt is not interesting for us.
     */
    ULONG IrqCount;

    /*! This even is used to wake-up the task inside of 
        wait_for_listener() again */
    KEVENT EventWaitForListener;

    /*! FLAG: We already allocated the parallel port */
    BOOLEAN ParallelPortAllocated;
    
    /*! FLAG: The mode of the parallel port has already been set */
    BOOLEAN ParallelPortModeSet;

    /*! FLAG: The interrupt for the parallel port has been allocated */
    BOOLEAN ParallelPortAllocatedInterrupt;

    /*! FLAG: We are running on an SMP or an HT machine */
    BOOLEAN IsSMP;

    // IEC related vars:

    /*! 
     *  >1 --> autodetect
     * ==0 --> non-inverted (XM1541)
     *  =1 --> inverted (XA1541)
     */
    USHORT IecCable;

    /*! The current state of the output bits of the parallel port */
    UCHAR IecOutBits;

    /*! EOR mask for outputting on the parallel port */
    UCHAR IecOutEor;

    /*! Remember if this device is busy */
    BOOLEAN IecBusy;

    /*! TRUE -> we received an EOI over the bus */
    BOOLEAN Eoi;

    /*! The thread handle for the worker thread*/
    HANDLE ThreadHandle;

    /*! Pointer to the thread structure */
    PKTHREAD Thread;

    /*! Boolean value: Should the running thread quit itself? */
    BOOLEAN QuitThread;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


extern VOID
DriverUnload(IN PDRIVER_OBJECT DriverObject);

extern NTSTATUS
DriverCommonInit(IN PDRIVER_OBJECT Driverobject, IN PUNICODE_STRING RegistryPath);

extern VOID
DriverCommonUninit(VOID);

extern NTSTATUS
AddDeviceCommonInit(IN PDEVICE_OBJECT Fdo, IN PUNICODE_STRING DeviceName, IN PCWSTR ParallelPortName);

extern NTSTATUS
cbm_install(IN PDEVICE_EXTENSION Pdx, OUT PCBMT_I_INSTALL_OUT ReturnBuffer, IN OUT PULONG ReturnLength);

#if DBG
extern NTSTATUS
cbm_dbg_readbuffer(IN PDEVICE_EXTENSION Pdx, OUT PCHAR ReturnBuffer, IN OUT PULONG ReturnLength);
#endif // #if DBG

extern VOID
cbm_init_registry(IN PUNICODE_STRING RegistryPath);

extern NTSTATUS
cbm_startio(IN PDEVICE_OBJECT Fdo, IN PIRP Irp);

extern NTSTATUS
cbm_createopenclose(IN PDEVICE_OBJECT Fdo, IN PIRP Irp);

extern NTSTATUS 
cbm_readwrite(IN PDEVICE_OBJECT Fdo, IN PIRP Irp);

extern NTSTATUS 
cbm_execute_readwrite(IN PDEVICE_EXTENSION Pdx, IN PIRP Irp);

extern NTSTATUS
cbm_execute_createopen(IN PDEVICE_EXTENSION Pdx, IN PIRP Irp);

extern NTSTATUS
cbm_cleanup(IN PDEVICE_OBJECT Fdo, IN PIRP Irp);

extern NTSTATUS
cbm_execute_close(IN PDEVICE_EXTENSION Pdx, IN PIRP Irp);

extern NTSTATUS 
cbm_devicecontrol(IN PDEVICE_OBJECT Fdo, IN PIRP Irp);

extern NTSTATUS
cbm_execute_devicecontrol(IN PDEVICE_EXTENSION Pdx, IN PIRP Irp);

extern BOOLEAN
cbm_isr(IN PKINTERRUPT Interrupt, IN PVOID Pdx);

extern VOID
cbm_thread(IN PVOID Context);

extern NTSTATUS
cbm_start_thread(IN PDEVICE_EXTENSION Pdx);

extern VOID
cbm_stop_thread(IN PDEVICE_EXTENSION Pdx);

#ifndef PENUMERATE_DEFINED
   /*! opaque enumeration structure. */
   typedef PVOID PENUMERATE;
#endif

extern NTSTATUS 
ParPortEnumerateOpen(PENUMERATE *EnumStruct);

extern NTSTATUS 
ParPortEnumerate(PENUMERATE EnumStruct, PCWSTR *DriverName);

extern VOID
ParPortEnumerateClose(PENUMERATE EnumStruct);

extern NTSTATUS
ParPortInit(PUNICODE_STRING ParallelPortName, PDEVICE_EXTENSION Pdx);

extern NTSTATUS
ParPortDeinit(PDEVICE_EXTENSION Pdx);

extern NTSTATUS
ParPortAllocate(PDEVICE_EXTENSION Pdx);

extern NTSTATUS
ParPortFree(PDEVICE_EXTENSION Pdx);

extern NTSTATUS
ParPortSetMode(PDEVICE_EXTENSION Pdx);

extern NTSTATUS
ParPortUnsetMode(PDEVICE_EXTENSION Pdx);

extern NTSTATUS
ParPortAllocInterrupt(PDEVICE_EXTENSION Pdx, PKSERVICE_ROUTINE Isr);

extern NTSTATUS
ParPortFreeInterrupt(PDEVICE_EXTENSION Pdx);

extern NTSTATUS
ParPortAllowInterruptIoctl(PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbm_registry_open_for_read(OUT PHANDLE HandleKey, IN PUNICODE_STRING Path);

extern NTSTATUS
cbm_registry_open_hardwarekey(OUT PHANDLE HandleKey, OUT PDEVICE_OBJECT *Pdo,
                              IN PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbm_registry_close(IN HANDLE HandleKey);

extern NTSTATUS
cbm_registry_close_hardwarekey(IN HANDLE HandleKey, IN PDEVICE_OBJECT Pdo);

extern NTSTATUS
cbm_registry_read_ulong(IN HANDLE HandleKey, IN PCWSTR KeyName, OUT PULONG Value);

extern NTSTATUS
cbm_registry_write_ulong(IN HANDLE HandleKey, IN PCWSTR KeyName, IN ULONG Value);

extern NTSTATUS
CbmOpenDeviceRegistryKey(IN PDEVICE_OBJECT a, IN ULONG b, IN ACCESS_MASK c, OUT PHANDLE d);
