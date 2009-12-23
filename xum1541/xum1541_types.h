/*
 * xum1541 external message types and defines
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _XUM1541_TYPES_H
#define _XUM1541_TYPES_H

// XUM1541_INFO reports these versions
#define XUM1541_VERSION_MAJOR       5 // Same for all xum1541 models
#define XUM1541_VERSION_MINOR       1 // Bump if incompatible protocol change

// USB parameters for descriptor configuration
#define XUM_ENDPOINT_0_SIZE         8
#define XUM_ENDPOINT_BULK_SIZE      64
#define XUM_BULK_IN_ENDPOINT        3
#define XUM_BULK_OUT_ENDPOINT       4

// control transactions
#define XUM1541_ECHO                0xff
#define XUM1541_INFO                0
#define XUM1541_RESET               1
// XUM1541_RESET is a control cmd too but numbering kept same as xu1541

// Capabilities bits
#define XU1541_CAP_CBM              0x0001 // supports CBM commands
#define XU1541_CAP_LL               0x0002 // supports low level io
#define XU1541_CAP_PP               0x0004 // supports 8 bit port
#define XU1541_CAP_NIB              0x0008 // supports nibbler
#define XU1541_CAP_PROTO_S1         0x0010 // supports serial1 protocol
#define XU1541_CAP_PROTO_S2         0x0020 // supports serial2 protocol
#define XU1541_CAP_PROTO_PP         0x0040 // supports parallel protocol
#define XU1541_CAP_PROTO_P2         0x0080 // supports parallel2 protocol

#define XUM1541_CAPABILITIES       (XU1541_CAP_CBM |       \
                                    XU1541_CAP_LL |        \
                                    XU1541_CAP_PP |        \
                                    XU1541_CAP_PROTO_S1 |  \
                                    XU1541_CAP_PROTO_S2 |  \
                                    XU1541_CAP_PROTO_PP |  \
                                    XU1541_CAP_PROTO_P2 |  \
                                    XU1541_CAP_NIB)

// Size of each command block sent to the device (bytes)
#define XUM_CMDBUF_SIZE             4

// Size of the internal buffer for split read/write commands
#define XUM1541_IO_BUFFER_SIZE      128

// Time to wait for device in seconds (60 max possible)
#define XUM1541_W4L_TIMEOUT         20

// Basic ioctls
#define XUM1541_READ                2
#define XUM1541_WRITE               3
#define XUM1541_IOCTL               16

/* constants for valid ioctl commands */
#define XUM1541_TALK                (XUM1541_IOCTL + 0)
#define XUM1541_LISTEN              (XUM1541_IOCTL + 1)
#define XUM1541_UNTALK              (XUM1541_IOCTL + 2)
#define XUM1541_UNLISTEN            (XUM1541_IOCTL + 3)
#define XUM1541_OPEN                (XUM1541_IOCTL + 4)
#define XUM1541_CLOSE               (XUM1541_IOCTL + 5)
#define XUM1541_GET_EOI             (XUM1541_IOCTL + 7)
#define XUM1541_CLEAR_EOI           (XUM1541_IOCTL + 8)

/* support commands for async read/write */
#define XUM1541_REQUEST_READ        (XUM1541_IOCTL + 9)
#define XUM1541_GET_RESULT          (XUM1541_IOCTL + 10)

// States for the async command handler
#define XUM1541_IO_IDLE             0
#define XUM1541_IO_READ             1
#define XUM1541_IO_READ_DONE        2
#define XUM1541_IO_WRITE_PREPARED   3
#define XUM1541_IO_WRITE            4
#define XUM1541_IO_IRQ_PAUSE        5
#define XUM1541_IO_RESULT           6
#define XUM1541_IO_ASYNC            7

#define XUM1541_PP_READ             (XUM1541_IOCTL + 11)
#define XUM1541_PP_WRITE            (XUM1541_IOCTL + 12)
#define XUM1541_IEC_POLL            (XUM1541_IOCTL + 13)
#define XUM1541_IEC_WAIT            (XUM1541_IOCTL + 14)
#define XUM1541_IEC_SETRELEASE      (XUM1541_IOCTL + 15)

/* constants needed by parallel burst (nibbler) */
#define XUM1541_PARBURST_READ       (XUM1541_IOCTL + 16)
#define XUM1541_PARBURST_WRITE      (XUM1541_IOCTL + 17)

/* basic and special protocol codes begin at 64 */
#define XUM1541_CBM                  64
#define XUM1541_S1                   (XUM1541_CBM + 1)
#define XUM1541_S2                   (XUM1541_CBM + 2)
#define XUM1541_PP                   (XUM1541_CBM + 3)
#define XUM1541_P2                   (XUM1541_CBM + 4)
#define XUM1541_NIB                  (XUM1541_CBM + 5)
#define XUM1541_NIB_READ_N           (XUM1541_CBM + 6)
#define XUM1541_NIB_WRITE_N          (XUM1541_CBM + 7)

#endif // _XUM1541_TYPES_H
