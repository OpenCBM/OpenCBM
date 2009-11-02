/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005-2008 Spiro Trikaliotis
 */

#ifndef ARCHLIB_H
#define ARCHLIB_H

#include "opencbm.h"

extern const char * cbmarch_get_driver_name(int PortNumber);
extern int  cbmarch_driver_open(CBM_FILE *HandleDevice, int PortNumber);
extern void cbmarch_driver_close(CBM_FILE HandleDevice);
extern void cbmarch_lock(CBM_FILE HandleDevice);
extern void cbmarch_unlock(CBM_FILE HandleDevice);
extern int  cbmarch_raw_write(CBM_FILE HandleDevice, const void *Buffer, size_t Count);
extern int  cbmarch_raw_read(CBM_FILE HandleDevice, void *Buffer, size_t Count);
extern int  cbmarch_listen(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
extern int  cbmarch_talk(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
extern int  cbmarch_open(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
extern int  cbmarch_unlisten(CBM_FILE HandleDevice);
extern int  cbmarch_untalk(CBM_FILE HandleDevice);
extern int  cbmarch_close(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
extern int  cbmarch_unlisten(CBM_FILE HandleDevice);
extern int  cbmarch_untalk(CBM_FILE HandleDevice);
extern int  cbmarch_get_eoi(CBM_FILE HandleDevice);
extern int  cbmarch_clear_eoi(CBM_FILE HandleDevice);
extern int  cbmarch_reset(CBM_FILE HandleDevice);
extern __u_char cbmarch_pp_read(CBM_FILE HandleDevice);
extern void cbmarch_pp_write(CBM_FILE HandleDevice, __u_char Byte);
extern int  cbmarch_iec_poll(CBM_FILE HandleDevice);
extern void cbmarch_iec_set(CBM_FILE HandleDevice, int Line);
extern void cbmarch_iec_release(CBM_FILE HandleDevice, int Line);
extern void cbmarch_iec_setrelease(CBM_FILE HandleDevice, int Set, int Release);
extern int  cbmarch_iec_wait(CBM_FILE HandleDevice, int Line, int State);

extern __u_char cbmarch_parallel_burst_read(CBM_FILE HandleDevice);
extern void cbmarch_parallel_burst_write(CBM_FILE HandleDevice, __u_char Value);
extern int  cbmarch_parallel_burst_read_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);
extern int  cbmarch_parallel_burst_read_track_var(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);
extern int  cbmarch_parallel_burst_write_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);

#endif // #ifndef ARCHLIB_H
