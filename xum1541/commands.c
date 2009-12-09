/*
 * Handle USB bulk and control transactions from the host
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include "xum1541.h"

/*
 * Basic inline IO functions where each byte is processed as it is
 * received. It is not necessary to buffer these because once we are in
 * the special protocol handlers, the drive is transferring data quickly
 * enough that we can do it inline. This decreases latency, especially for
 * protocols like nibbler.
 *
 * Do not use inline IO for slow transfers like IEC open/close/listen.
 */
typedef uint8_t (*ReadFn_t)(void);
typedef void (*WriteFn_t)(uint8_t data);
typedef void (*Read2Fn_t)(uint8_t *data);
typedef void (*Write2Fn_t)(uint8_t *data);

static uint16_t usbDataLen;
static uint8_t usbDataDir = XUM_DATA_DIR_NONE;

static void
usbInitIo(uint16_t len, uint8_t dir)
{
#ifdef DEBUG
    if (usbDataDir != XUM_DATA_DIR_NONE)
        DEBUGF("ERR: usbInitIo left in bad state %d\n", usbDataDir);
#endif

    // Select the proper endpoint for this direction
    if (dir == ENDPOINT_DIR_IN) {
        Endpoint_SelectEndpoint(XUM_BULK_IN_ENDPOINT);
    } else if (dir == ENDPOINT_DIR_OUT) {
        Endpoint_SelectEndpoint(XUM_BULK_OUT_ENDPOINT);
    } else {
        DEBUGF("ERR: usbInitIo bad dir %d\n");
    }

    usbDataLen = len;
    usbDataDir = dir;

    // Wait until endpoint is ready before continuing
    while (!Endpoint_IsReadWriteAllowed())
        ;
}

static void
usbIoDone(void)
{
    // Clear any leftover state
    if (usbDataLen != 0) {
        DEBUGF("incomplete io %d: %d\n", usbDataDir, usbDataLen);
        usbDataLen = 0;
    }

    // Finalize any outstanding transactions
    if (usbDataDir == ENDPOINT_DIR_IN) {
        if (Endpoint_BytesInEndpoint() != 0) {
            Endpoint_ClearIN();
        }
    } else if (usbDataDir == ENDPOINT_DIR_OUT) {

    } else {
        DEBUGF("done: bad io dir %d\n", usbDataDir);
    }
    usbDataDir = XUM_DATA_DIR_NONE;
}

static uint8_t
usbSendByte(uint8_t data)
{

#ifdef DEBUG
    if (usbDataDir != ENDPOINT_DIR_IN) {
        DEBUGF("ERR: usbSendByte when dir was %d\n", usbDataDir);
        return -1;
    }
#endif

    /*
     * Check if the endpoint is currently full.
     * If so, clear the endpoint bank to send it to the host and
     * wait until the buffer is free again.
     */
    if (!Endpoint_IsReadWriteAllowed()) {
        Endpoint_ClearIN();
        while (!Endpoint_IsReadWriteAllowed())
            ;
    }

    // Write data back to the host buffer for USB transfer
    Endpoint_Write_Byte(data);
    usbDataLen--;

    // If the endpoint is now full, flush the block to the host
    if (!Endpoint_IsReadWriteAllowed()) {
        Endpoint_ClearIN();
        }

    // Check if the current command is being aborted by the host
    if (doDeviceReset) {
        DEBUGF("sndrst\n");
        return -1;
    }

    return 0;
}

static uint8_t
usbRecvByte(uint8_t *data)
{

#ifdef DEBUG
    if (usbDataDir != ENDPOINT_DIR_OUT) {
        DEBUGF("ERR: usbRecvByte when dir was %d\n", usbDataDir);
        return -1;
    }
#endif

    /*
     * Check if the endpoint is currently empty.
     * If so, clear the endpoint bank to get more data from host and
     * wait until the host has sent data.
     */
    if (!Endpoint_IsReadWriteAllowed()) {
        Endpoint_ClearOUT();
        while (!Endpoint_IsReadWriteAllowed())
            ;
    }

    // Read data from the host buffer, received via USB
    *data = Endpoint_Read_Byte();
    usbDataLen--;

    // If the endpoint is now empty, pre-fetch the next data
    if (!Endpoint_IsReadWriteAllowed()) {
        Endpoint_ClearOUT();
    }

    // Check if the current command is being aborted by the host
    if (doDeviceReset) {
        DEBUGF("rcvrst\n");
        return -1;
    }

    return 0;
}

static uint8_t
ioReadLoop(ReadFn_t readFn, uint16_t len)
{
    uint8_t data;

    usbInitIo(len, ENDPOINT_DIR_IN);
    while (len-- != 0) {
        data = readFn();
        usbSendByte(data);
    }
    usbIoDone();
    return 0;
}

static uint8_t
ioWriteLoop(WriteFn_t writeFn, uint16_t len)
{
    uint8_t data;

    usbInitIo(len, ENDPOINT_DIR_OUT);
    while (len-- != 0) {
        usbRecvByte(&data);
        writeFn(data);
    }
    usbIoDone();
    return 0;
}

static uint8_t
ioRead2Loop(Read2Fn_t readFn, uint16_t len)
{
    uint8_t data[2];

    usbInitIo(len, ENDPOINT_DIR_IN);
    while (len != 0) {
        readFn(data);
        usbSendByte(data[0]);
        usbSendByte(data[1]);
        len -= 2;
    }
    usbIoDone();
    return 0;
}

static uint8_t
ioWrite2Loop(Write2Fn_t writeFn, uint16_t len)
{
    uint8_t data[2];

    usbInitIo(len, ENDPOINT_DIR_OUT);
    while (len != 0) {
        usbRecvByte(&data[0]);
        usbRecvByte(&data[1]);
        writeFn(data);
        len -= 2;
    }
    usbIoDone();
    return 0;
}

static uint8_t
ioReadNibLoop(uint16_t len)
{
    uint16_t i;
    int8_t ret;
    uint8_t data;

    usbInitIo(len, ENDPOINT_DIR_IN);
    for (i = 0; i < len; i++) {
        // Read a byte from the parport
        ret = nib_read_handshaked(&data, i & 1);
        if (ret < 0) {
            DEBUGF("nbrdth1 to %d\n", i);
            break;
        }
        // Send the byte via USB
        usbSendByte(data);
    }

    // All bytes read ok so read the final dummy byte
    if (i == len)
        data = nib_parburst_read();

    usbIoDone();
    return 0;
}

static uint8_t
ioWriteNibLoop(uint16_t len)
{
    uint16_t i;
    int8_t ret;
    uint8_t data;

    usbInitIo(len, ENDPOINT_DIR_OUT);
    for (i = 0; i < len; i++) {
        // Get the byte via USB
        usbRecvByte(&data);

        // Write a byte to the parport
        ret = nib_write_handshaked(data, i & 1);
        if (ret < 0) {
            DEBUGF("nbwrh1 to\n");
            break;
        }
    }
    // All bytes read ok so read the final dummy byte
    if (i == len) {
        nib_write_handshaked(0, i & 1);
        data = nib_parburst_read();
    }

    usbIoDone();
    return i;
}

/*
 * Process the given USB control command, storing the result in replyBuf
 * and returning the number of output bytes. Returns -1 if command is
 * invalid. All control processing has to happen until completion (no
 * delayed execution) and it may not take additional input from the host
 * (data direction set as host to device).
 */
int8_t
usbHandleControl(uint8_t cmd, uint8_t *replyBuf)
{
    DEBUGF("cmd %d (%d)\n", cmd, cmd - XUM1541_IOCTL);

    board_set_status(STATUS_ACTIVE);
    switch (cmd) {
    case XUM1541_ECHO:
        replyBuf[0] = cmd;
        return 1;
    case XUM1541_INFO:
        replyBuf[0] = XUM1541_VERSION_MAJOR;
        replyBuf[1] = XUM1541_VERSION_MINOR;
        return 2;
    case XUM1541_RESET:
        // Clear any USB state, reset the IEC bus
        USB_ResetConfig();
        xu1541_reset();
        board_set_status(STATUS_READY);
        return 0;
    default:
        DEBUGF("ERR: control cmd %d not impl\n", cmd);
        return -1;
    }
}

int8_t
usbHandleBulk(uint8_t *request, uint8_t *replyBuf)
{
    uint8_t talk;
    uint16_t len = (request[3] << 8) | request[2];
    uint8_t proto;

    board_set_status(STATUS_ACTIVE);
    switch (request[0]) {
    case XUM1541_GET_RESULT:
        xu1541_get_result(replyBuf);
        return 2;
    case XUM1541_REQUEST_READ:
        DEBUGF("req rd %d\n", len);
        xu1541_request_read((uint8_t)len);
        return 0;
    case XUM1541_READ:
        proto = request[1];
        DEBUGF("rd:%d %d\n", proto, len);
        if (proto == XUM1541_CBM) {
            xu1541_read((uint8_t)len);
        } else {
            // loop to read all the bytes now, sending back each as we get it
            switch (proto) {
            case XUM1541_S1:
                ioReadLoop(s1_read_byte, len);
                break;
            case XUM1541_S2:
                ioReadLoop(s2_read_byte, len);
                break;
            case XUM1541_PP:
                ioRead2Loop(pp_read_2_bytes, len);
                break;
            case XUM1541_P2:
                ioReadLoop(p2_read_byte, len);
                break;
            case XUM1541_NIB:
                ioReadNibLoop(len);
                break;
            default:
                DEBUGF("badproto %d\n", proto);
            }
        }
        return 0;
    case XUM1541_WRITE:
        proto = request[1];
        DEBUGF("wr:%d %d\n", proto, len);
        if (proto == XUM1541_CBM) {
            xu1541_write((uint8_t)len);
        } else {
            // loop to fetch each byte and write it as we get it
            switch (proto) {
            case XUM1541_S1:
                ioWriteLoop(s1_write_byte, len);
                break;
            case XUM1541_S2:
                ioWriteLoop(s2_write_byte, len);
                break;
            case XUM1541_PP:
                ioWrite2Loop(pp_write_2_bytes, len);
                break;
            case XUM1541_P2:
                ioWriteLoop(p2_write_byte, len);
                break;
            case XUM1541_NIB:
                ioWriteNibLoop(len);
                break;
            default:
                DEBUGF("badproto %d\n", proto);
            }
        }
        return 0;

    /* Async commands for the control channel */
    case XUM1541_TALK:
    case XUM1541_LISTEN:
        DEBUGF("tlk/lst(%d,%d)\n", request[1], request[2]);
        talk = (request[0] == XUM1541_TALK);
        request[1] |= talk ? 0x40 : 0x20;
        request[2] |= 0x60;
        xu1541_request_async(request + 1, 2, /*atn*/1, talk);
        return 0;
    case XUM1541_UNTALK:
    case XUM1541_UNLISTEN:
        DEBUGF("untlk/unlst\n");
        request[1] = (request[0] == XUM1541_UNTALK) ? 0x5f : 0x3f;
        xu1541_request_async(request + 1, 1, /*atn*/1, /*talk*/0);
        return 0;
    case XUM1541_OPEN:
    case XUM1541_CLOSE:
        DEBUGF("open/close(%d,%d)\n", request[1], request[2]);
        request[1] |= 0x20;
        request[2] |= (request[0] == XUM1541_OPEN) ? 0xf0 : 0xe0;
        xu1541_request_async(request + 1, 2, /*atn*/1, /*talk*/0);
        return 0;

    /* Low-level port access */
    case XUM1541_GET_EOI:
        replyBuf[0] = eoi ? 1 : 0;
        return 1;
    case XUM1541_CLEAR_EOI:
        eoi = 0;
        return 0;
    case XUM1541_IEC_WAIT:
        xu1541_wait(/*line*/request[1], /*state*/request[2]);
        /* FALLTHROUGH */
    case XUM1541_IEC_POLL:
        replyBuf[0] = xu1541_poll();
        DEBUGF("poll=%x\n", replyBuf[0]);
        return 1;
    case XUM1541_IEC_SETRELEASE:
        xu1541_setrelease(/*set*/request[1], /*release*/request[2]);
        return 0;
    case XUM1541_PP_READ:
        replyBuf[0] = xu1541_pp_read();
        return 1;
    case XUM1541_PP_WRITE:
        xu1541_pp_write(request[1]);
        return 0;
    default:
        DEBUGF("ERR: bulk cmd %d not impl.\n", request[0]);
        return -1;
    }

    return 0;
}
