/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005 Spiro Trikaliotis
*/

/* $Id: libopencbmtransfer.h,v 1.1.2.1 2006-02-12 16:59:20 strik Exp $ */

#ifndef LIBOPENCBMTRANSFER_H
#define LIBOPENCBMTRANSFER_H

#include "opencbm.h"

typedef
enum opencbm_transfer_e
{
    opencbm_transfer_serial1,
    opencbm_transfer_serial2,
    opencbm_transfer_parallel
} opencbm_transfer_t;

int
libopencbmtransfer_set_transfer(opencbm_transfer_t type);

int
libopencbmtransfer_install(CBM_FILE HandleDevice, __u_char DeviceAddress);

int
libopencbmtransfer_execute_command(CBM_FILE HandleDevice, __u_char DeviceAddress,
                                   unsigned int ExecutionAddress);

int
libopencbmtransfer_read_mem(CBM_FILE HandleDevice, __u_char DeviceAddress,
                            __u_char Buffer[], unsigned int MemoryAddress, unsigned int Length);

int
libopencbmtransfer_write_mem(CBM_FILE HandleDevice, __u_char DeviceAddress,
                            __u_char Buffer[], unsigned int MemoryAddress, unsigned int Length);

int
libopencbmtransfer_remove(CBM_FILE HandleDevice, __u_char DeviceAddress);

#endif /* #ifndef LIBOPENCBMTRANSFER_H */
