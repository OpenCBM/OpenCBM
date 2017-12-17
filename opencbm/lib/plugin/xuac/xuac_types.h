/*
 * xuac external message types and defines
 * Copyright (c) 2009-2010 Nate Lawson <nate@root.org>
 * Copyright (c) 2017 Arnd Menge (Modifications for XUAC platform)
 *
 * Incorporates content from 'xum1541_types.h' by:
 * Copyright (c) 2009-2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _XUAC_TYPES_H
#define _XUAC_TYPES_H

// Vendor and product ID. These are owned by Nate Lawson, do not reuse.
#define XUM1541_VID                    0x16d0
#define XUM1541_PID                    0x0504
#define XUAC_VID                       XUM1541_VID
#define XUAC_PID                       XUM1541_PID

// XUAC_INIT reports this versions
#define XUAC_VERSION                   7

// USB parameters for descriptor configuration
#define XUAC_BULK_IN_ENDPOINT          1
#define XUAC_BULK_OUT_ENDPOINT         2
//#define XUAC_ENDPOINT_0_SIZE         8

// control transactions
#define XUAC_ECHO                      0
#define XUAC_INIT                      (XUAC_ECHO + 1)
#define XUAC_RESET                     (XUAC_ECHO + 2)
#define XUAC_SHUTDOWN                  (XUAC_ECHO + 3)
//#define XUAC_ENTER_BOOTLOADER        (XUAC_ECHO + 4)
#define XUAC_TAP_BREAK                 (XUAC_ECHO + 5)

// Adapter capabilities, but device may not support them
#define BOARD_CAPABILITY_DISK_CBM      0x01 // supports CBM commands
#define BOARD_CAPABILITY_DISK_NIB_PAR  0x02 // parallel nibbler
#define BOARD_CAPABILITY_DISK_NIB_SRQ  0x04 // 1571 serial nibbler
#define BOARD_CAPABILITY_DISK_IEEE488  0x08 // GPIB (PET) parallel bus
#define BOARD_CAPABILITY_TAPE          0x10 // 153x tape support

#define XUAC_CAPABILITIES              BOARD_CAPABILITY_TAPE

// Actual auto-detected status
#define XUAC_DOING_RESET               0x01 // no clean shutdown, will reset now
//#define XUAC_DOING_RESET             0x01 // no clean shutdown, will reset now
//#define XUAC_NO_DEVICE               0x02 // no IEC device present yet
//#define XUAC_IEEE488_PRESENT         0x10 // IEEE-488 device connected
//#define XUAC_TAPE_PRESENT            0x20 // 153x tape device connected

// Sizes for commands and responses in bytes
#define XUAC_CMD_LEN                   6
#define XUAC_ANSWER_LEN                7
#define XUAC_DEVINFO_SIZE              8

/*
 * Control msg command timeout. Since the longest command we run in this
 * mode is XUAC_RESET (or INIT if it has to run RESET), we chose a
 * time greater than the maximum a device has to respond after ATN
 * goes active. The IEC spec lists Tat as 1 ms assuming the device is
 * alive, so we're being very generous.
 */
#define XUAC_TIMEOUT                   1.5

/*
 * Maximum size for USB transfers (read/write commands, all protocols).
 * This should be ok for the raw USB protocol. I haven't tested this much
 * but at least 8192 works (e.g. for nib protocol reads. For longer
 * transfers, the usermode code should break it up into chunks this size.
 */
#define XUAC_MAX_XFER_SIZE             32768

// USB bulk request IDs for misc operations.
#define XUAC_MISC                         80
#define XUAC_BOARD_INFO                   (XUAC_MISC + 0)
#define XUAC_MEMORY_TRANSFERBUFFER_CLEAR  (XUAC_MISC + 1)
#define XUAC_MEMORY_TRANSFERBUFFER_ISFULL (XUAC_MISC + 2)
#define XUAC_MEMORY_TRANSFERBUFFER_FILL   (XUAC_MISC + 3)
#define XUAC_SET_VALUE32                  (XUAC_MISC + 4)
#define XUAC_GET_VALUE32                  (XUAC_MISC + 5)

// USB bulk request IDs for tape operations.
#define XUAC_TAP                          100 
#define XUAC_TAP_MOTOR_ON                 (XUAC_TAP + 0)
#define XUAC_TAP_GET_VER                  (XUAC_TAP + 1)
#define XUAC_TAP_PREPARE_CAPTURE          (XUAC_TAP + 2)
#define XUAC_TAP_PREPARE_WRITE            (XUAC_TAP + 3)
#define XUAC_TAP_GET_SENSE                (XUAC_TAP + 4)
#define XUAC_TAP_WAIT_FOR_STOP_SENSE      (XUAC_TAP + 5)
#define XUAC_TAP_WAIT_FOR_PLAY_SENSE      (XUAC_TAP + 6)
#define XUAC_TAP_CAPTURE                  (XUAC_TAP + 7)
#define XUAC_TAP_WRITE                    (XUAC_TAP + 8)
#define XUAC_TAP_MOTOR_OFF                (XUAC_TAP + 9)

/*
 * Status return values for async commands.
 * Status response also contains a second data word, 16-bits little-endian.
 * This value, accessed via XUAC_GET_STATUS_VAL(), usually gives a length
 * but is command-specific.
 */
#define XUAC_IO_BUSY                   1
#define XUAC_IO_READY                  2
#define XUAC_IO_ERROR                  3

// Tape mode error return value.
//#define XUAC_Error_NoTapeSupport      -100

// Macros to retrive the status and extended value (usually a length).
#define XUAC_GET_STATUS(buf)           (buf[0])
#define XUAC_GET_STATUS_VAL(buf)       (((buf[2]) << 8) | (buf[1]))

#endif // _XUAC_TYPES_H
