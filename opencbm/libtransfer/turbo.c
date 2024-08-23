/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2006 Spiro Trikaliotis
*/

#include "libtrans.h"
#include "libtrans_int.h"

#include <stdio.h>

static const unsigned char turbomain_1541_1571_drive_prog[] = {
#include "turbomain-1541-1571.inc"
};

static const unsigned char turbomain_1581_drive_prog[] = {
#include "turbomain-1581.inc"
};

static const unsigned char turbomain_fdx000_drive_prog[] = {
#include "turbomain-fdx000.inc"
};


/*
// functions to perform:

libopencbmtransfer_test()
*/

static transfer_funcs *current_transfer_funcs = &libopencbmtransfer_s1;

int
libopencbmtransfer_set_transfer(opencbm_transfer_t TransferType)
{
    switch (TransferType)
    {
    case opencbm_transfer_serial1:
        current_transfer_funcs = &libopencbmtransfer_s1;
        break;

    case opencbm_transfer_serial2:
        current_transfer_funcs = &libopencbmtransfer_s2;
        break;

    case opencbm_transfer_serial3:
        current_transfer_funcs = &libopencbmtransfer_s3;
        break;

    case opencbm_transfer_parallel:
        current_transfer_funcs = &libopencbmtransfer_pp;
        break;

    default:
        printf("Unknown transfer type %u!\n", TransferType);
        return 1;
    }

    return 0;
}


/*! \brief Install the turbo routines into a drive

 This functions installs the turbo routines for later
 use in the drive.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \return
   0 means turbo routines have been installed successfully.
   Every other value denotes an error

 If this function does not return successfully, it is illegal
 to call any other libopencbmtransfer function.
*/
int
libopencbmtransfer_install(CBM_FILE HandleDevice, unsigned char DeviceAddress)
{
    enum cbm_device_type_e cbmDeviceType;
    const char *cbmDeviceString;
    int error = 0;
    const unsigned char *turbomain_drive_prog = 0;
    unsigned int turbomain_drive_prog_length = 0;

    if (cbm_identify(HandleDevice, DeviceAddress, &cbmDeviceType, &cbmDeviceString))
    {
        error = 1;
        DBG_ERROR((DBG_PREFIX "cbm_identify returned with an error."));
    }
    else
    {
        DBG_SUCCESS((DBG_PREFIX "cbm_identify returned %s", cbmDeviceString));
    }

    switch (cbmDeviceType)
    {
    case cbm_dt_fdx000:
        DBG_PRINT((DBG_PREFIX "recognized CMD FDx000 (FD2000/FD4000)."));
        turbomain_drive_prog = turbomain_fdx000_drive_prog;
        turbomain_drive_prog_length = sizeof(turbomain_fdx000_drive_prog);
        break;

    case cbm_dt_cbm1581:
        DBG_PRINT((DBG_PREFIX "recognized 1581."));
        turbomain_drive_prog = turbomain_1581_drive_prog;
        turbomain_drive_prog_length = sizeof(turbomain_1581_drive_prog);
        break;

    case cbm_dt_cbm1541:
        DBG_PRINT((DBG_PREFIX "recognized 1541."));
        turbomain_drive_prog = turbomain_1541_1571_drive_prog;
        turbomain_drive_prog_length = sizeof(turbomain_1541_1571_drive_prog);
        break;

    case cbm_dt_cbm1570:
    case cbm_dt_cbm1571:
        DBG_PRINT((DBG_PREFIX "recognized 1571."));
        turbomain_drive_prog = turbomain_1541_1571_drive_prog;
        turbomain_drive_prog_length = sizeof(turbomain_1541_1571_drive_prog);
        break;

    case cbm_dt_unknown:
        /* FALL THROUGH */

    default:
        DBG_ERROR((DBG_PREFIX "unknown device type!"));
        return 1;
    }


    // set device type for transfer routine

    if (current_transfer_funcs->set_device_type(cbmDeviceType)) {
        fprintf(stderr, "this transfer method does not work on this drive.");
        return 1;
    }

    // Upload turbo routines into drive

    if (current_transfer_funcs->upload(HandleDevice, DeviceAddress)) {
        fprintf(stderr, "this transfer method does not work on this drive,\nor upload failed.");
        return 1;
    }

    if (!error)
    {
        int bytesWritten;

        // Now, upload the main loop into the drive

        bytesWritten = cbm_upload(HandleDevice, DeviceAddress, 0x500,
            turbomain_drive_prog, turbomain_drive_prog_length);

        if (bytesWritten != turbomain_drive_prog_length)
        {
            DBG_ERROR((DBG_PREFIX "wanted to write %u bytes, but only %u "
                "bytes could be written", turbomain_drive_prog_length, bytesWritten));

            error = 1;
        }
    }

    if (!error)
    {
        if (cbm_exec_command(HandleDevice, DeviceAddress, "U3", 0))
        {
            DBG_ERROR((DBG_PREFIX "cbm_exec_command returnd with an error."));
            error = 1;
        }

        //printf("wait...\n");
        current_transfer_funcs->init(HandleDevice, DeviceAddress);
        //printf("... end\n");
    }

    FUNC_LEAVE_INT(error);
}


int
libopencbmtransfer_execute_command(CBM_FILE HandleDevice, unsigned char DeviceAddress,
                                   unsigned int ExecutionAddress)
{
    current_transfer_funcs->write1byte(HandleDevice, 0x80);
    current_transfer_funcs->write2byte(HandleDevice,
        (unsigned char) (ExecutionAddress & 0xFF),
        (unsigned char) (ExecutionAddress >> 8));

    return 0;
}

typedef int
(*ll_read_write_mem)(CBM_FILE HandleDevice, unsigned char DeviceAddress,
                     unsigned char Buffer[], unsigned int MemoryAddress, unsigned int Length);

static int
libopencbmtransfer_ll_write_mem(CBM_FILE HandleDevice, unsigned char DeviceAddress,
                                unsigned char Buffer[], unsigned int MemoryAddress, unsigned int Length)
{
    FUNC_ENTER();

    DBG_ASSERT(Length < 0x100);

    current_transfer_funcs->write1byte(HandleDevice, 0x00);
    current_transfer_funcs->write2byte(HandleDevice,
        (unsigned char) (MemoryAddress & 0xFF),
        (unsigned char) (MemoryAddress >> 8));
    current_transfer_funcs->write1byte(HandleDevice, (unsigned char) Length);
    current_transfer_funcs->writeblock(HandleDevice, Buffer, Length);

    FUNC_LEAVE_INT(0);
}


static int
libopencbmtransfer_ll_read_mem(CBM_FILE HandleDevice, unsigned char DeviceAddress,
                               unsigned char Buffer[], unsigned int MemoryAddress, unsigned int Length)
{
    FUNC_ENTER();

    DBG_ASSERT(Length < 0x100);

    current_transfer_funcs->write1byte(HandleDevice, 0x01);
    current_transfer_funcs->write2byte(HandleDevice,
        (unsigned char) (MemoryAddress & 0xFF),
        (unsigned char) (MemoryAddress >> 8));
    current_transfer_funcs->write1byte(HandleDevice, (unsigned char) Length);
    current_transfer_funcs->readblock(HandleDevice, Buffer, Length);

    FUNC_LEAVE_INT(0);
}

static int
libopencbmtransfer_read_write_mem(CBM_FILE HandleDevice, unsigned char DeviceAddress,
                                  unsigned char Buffer[], unsigned int MemoryAddress, unsigned int Length,
                                  ll_read_write_mem function)
{
    const static char monkey[]={",oO*^!:;"};// for fast moves

    FUNC_ENTER();

    fprintf(stderr, " ");

    // If we have to transfer more than one page, process the complete pages first
                                                                        SETSTATEDEBUG(DebugBlockCount = 0);
    while (Length >= 0x100)
    {
        int c = (Length >> 8) % (sizeof(monkey) - 1);
        fprintf(stderr, (c != 0) ? "\b%c" : "\b.%c" , monkey[c]);
        fflush(stderr);

                                                                        SETSTATEDEBUG(DebugBlockCount++);
        function(HandleDevice, DeviceAddress, Buffer, MemoryAddress, 0x00);

        Buffer += 0x100;
        MemoryAddress += 0x100;
        Length -= 0x100;
    }

    if (Length > 0)
    {
        unsigned int remainder = 0x100 - Length;
                                                                        SETSTATEDEBUG(DebugBlockCount++);
        //fprintf(stderr, "."); fflush(stderr);
        fprintf(stderr, "\010.");
        fflush(stderr);
        function(HandleDevice, DeviceAddress, Buffer, MemoryAddress - remainder, remainder);
    }
                                                                        SETSTATEDEBUG(DebugBlockCount = -1);
    fprintf(stderr, "\010.\n");  // fflush(stderr);

    FUNC_LEAVE_INT(0);
}

int
libopencbmtransfer_read_mem(CBM_FILE HandleDevice, unsigned char DeviceAddress,
                            unsigned char Buffer[], unsigned int MemoryAddress, unsigned int Length)
{
    return libopencbmtransfer_read_write_mem(HandleDevice, DeviceAddress,
                                  Buffer, MemoryAddress, Length, libopencbmtransfer_ll_read_mem);
}

int
libopencbmtransfer_write_mem(CBM_FILE HandleDevice, unsigned char DeviceAddress,
                            unsigned char Buffer[], unsigned int MemoryAddress, unsigned int Length)
{
    return libopencbmtransfer_read_write_mem(HandleDevice, DeviceAddress,
                                  Buffer, MemoryAddress, Length, libopencbmtransfer_ll_write_mem);
}

int
libopencbmtransfer_remove(CBM_FILE HandleDevice, unsigned char DeviceAddress)
{
    // TODO: does not work with 1581, only with 1541/1571!
    // better: write a JMP to the RESET routine, and execute that
    return cbm_reset(HandleDevice);
//    return libopencbmtransfer_execute_command(HandleDevice, DeviceAddress, 0xEBE7);
}
