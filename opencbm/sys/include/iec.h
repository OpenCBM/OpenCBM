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
** \file sys/include/iec.h \n
** \author Spiro Trikaliotis \n
** \version $Id: iec.h,v 1.3 2005-01-22 19:50:41 strik Exp $ \n
** \n
** \brief Definitions for the libiec library
**
****************************************************************/

#ifndef CBMIEC_H
#define CBMIEC_H

#include "cbmioctl.h"

extern NTSTATUS
cbmiec_reset(IN PDEVICE_EXTENSION Pdx);

extern BOOLEAN
cbmiec_send_byte(IN PDEVICE_EXTENSION Pdx, IN UCHAR Byte);

extern VOID
cbmiec_wait_for_listener(IN PDEVICE_EXTENSION Pdx, IN BOOLEAN SendEoi);

extern VOID
cbmiec_release_bus(IN PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbmiec_untalk(IN PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbmiec_unlisten(IN PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbmiec_listen(IN PDEVICE_EXTENSION Pdx, IN UCHAR DeviceAddress, IN UCHAR SecondaryAddress);

extern NTSTATUS
cbmiec_talk(IN PDEVICE_EXTENSION Pdx, IN UCHAR DeviceAddress, IN UCHAR SecondaryAddress);

extern NTSTATUS
cbmiec_open(IN PDEVICE_EXTENSION Pdx, IN UCHAR DeviceAddress, IN UCHAR SecondaryAddress);

extern NTSTATUS
cbmiec_close(IN PDEVICE_EXTENSION Pdx, IN UCHAR DeviceAddress, IN UCHAR SecondaryAddress);

extern NTSTATUS
cbmiec_get_eoi(IN PDEVICE_EXTENSION Pdx, OUT PBOOLEAN Result);

extern NTSTATUS
cbmiec_clear_eoi(IN PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbmiec_iec_wait(IN PDEVICE_EXTENSION Pdx, IN UCHAR Line, IN UCHAR State, OUT PUCHAR Result);

extern NTSTATUS
cbmiec_iec_poll(IN PDEVICE_EXTENSION Pdx, OUT PUCHAR Result);

extern NTSTATUS
cbmiec_iec_set(IN PDEVICE_EXTENSION Pdx, IN USHORT Line);

extern NTSTATUS
cbmiec_iec_release(IN PDEVICE_EXTENSION Pdx, IN USHORT Line);

extern NTSTATUS
cbmiec_iec_setrelease(IN PDEVICE_EXTENSION Pdx, IN USHORT Mask, IN USHORT Line);

extern NTSTATUS
cbmiec_pp_read(IN PDEVICE_EXTENSION Pdx, OUT UCHAR *Byte);

extern NTSTATUS
cbmiec_pp_write(IN PDEVICE_EXTENSION Pdx, IN UCHAR Byte);

extern BOOLEAN
cbmiec_interrupt(IN PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbmiec_cleanup(IN PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbmiec_init(IN PDEVICE_EXTENSION Pdx);

extern NTSTATUS
cbmiec_global_init(IN PHANDLE HKey);

extern NTSTATUS
cbmiec_raw_write(IN PDEVICE_EXTENSION Pdx, IN const PUCHAR Buffer, IN USHORT BufferLength, OUT USHORT* Written);

extern NTSTATUS 
cbmiec_raw_read(IN PDEVICE_EXTENSION Pdx, OUT PUCHAR Buffer, IN USHORT BufferLength, OUT USHORT* Read);

#endif /* #ifndef CBMIEC_H */
