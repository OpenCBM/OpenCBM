/*
 * xum1541 IEC routines
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * Based on: firmware/xu1541.c
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 *
 * Imported at revision:
 * Revision 1.16    2009/01/24 14:51:01    strik
 * New version 1.17;
 * Do not return data for XUM1541_READ after an EOI occurred.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include "xum1541.h"

/* specifiers for the lines (must match values from opencbm.h) */
#define IEC_DATA    0x01
#define IEC_CLOCK   0x02
#define IEC_ATN     0x04
#define IEC_RESET   0x08

/* global variable to keep track of eoi state */
uint8_t eoi = 0;

/*
 * Commands are temporarily stored here to be processed while the host
 * is off the USB bus. We then report the status when completed.
 */
static uint8_t io_buffer[XUM1541_IO_BUFFER_SIZE];
static uint8_t io_buffer_len, io_request, io_result;

/* fast conversion between logical and physical mapping */
static const uint8_t iec2hw_table[] PROGMEM = {
    0,
    IO_DATA,
              IO_CLK,
    IO_DATA | IO_CLK,
                       IO_ATN,
    IO_DATA |          IO_ATN,
              IO_CLK | IO_ATN,
    IO_DATA | IO_CLK | IO_ATN,
                                IO_RESET,
    IO_DATA |                   IO_RESET,
              IO_CLK |          IO_RESET,
    IO_DATA | IO_CLK |          IO_RESET,
                       IO_ATN | IO_RESET,
    IO_DATA |          IO_ATN | IO_RESET,
              IO_CLK | IO_ATN | IO_RESET,
    IO_DATA | IO_CLK | IO_ATN | IO_RESET,
};

static uint8_t
iec2hw(uint8_t iec)
{
    return pgm_read_byte(iec2hw_table + iec);
}

// Initialize our command buffer and all IEC lines to idle
void
cbm_init(void)
{
    DEBUGF("init\n");

    io_buffer_len = 0;
    io_request = XUM1541_IO_IDLE;

    iec_release(IO_ATN | IO_CLK | IO_DATA | IO_RESET);
    DELAY_US(100);
}

static uint8_t
check_if_bus_free(void)
{
    iec_release(IO_ATN | IO_CLK | IO_DATA | IO_RESET);

    // wait for the drive to have time to react
    DELAY_US(100);

    // assert ATN
    iec_set(IO_ATN);

    // now, wait for the drive to have time to react
    DELAY_US(100);

    // if DATA is still unset, we have a problem.
    if (iec_get(IO_DATA) == 0) {
        iec_release(IO_ATN | IO_CLK | IO_DATA | IO_RESET);
        return 0;
    }

    // ok, at least one drive reacted. Now, test releasing ATN:
    iec_release(IO_ATN);
    DELAY_US(100);

    if (iec_get(IO_DATA) == 0) {
        iec_release(IO_ATN | IO_CLK | IO_DATA | IO_RESET);
        return 1;
    }

    iec_release(IO_ATN | IO_CLK | IO_DATA | IO_RESET);
    return 0;
}

static void
wait_for_free_bus(void)
{
    uint16_t i = XUM1541_RESET_TIMEOUT * 1000;

    while (1) {
        wdt_reset();

        if (check_if_bus_free())
            break;

        if (i-- == 0) {
            DEBUGF("wait4free bus to\n");
            break;
        }
        DELAY_MS(1);
    }
}

void
xu1541_reset(void)
{
    DEBUGF("reset\n");
    iec_release(IO_DATA | IO_ATN | IO_CLK);
    iec_set(IO_RESET);
    DELAY_MS(20);
    iec_release(IO_RESET);

    wait_for_free_bus();
}

static uint8_t
iec_wait_timeout_2ms(uint8_t mask, uint8_t state)
{
    uint8_t count = 200;

    while ((iec_poll() & mask) == state && count-- != 0)
        DELAY_US(10);

    return ((iec_poll() & mask) != state);
}

static void
iec_wait_clk(void)
{
    uint8_t count = 200;

    while (iec_get(IO_CLK) == 0 && count-- != 0)
        DELAY_US(2);
}

static uint8_t
send_byte(uint8_t b)
{
    uint8_t i, ack = 0;

    for (i = 0; i < 8; i++) {
        /* each _bit_ takes a total of 90us to send ... */
        DELAY_US(70);

        if ((b & 1) == 0)
            iec_set(IO_DATA);

        iec_release(IO_CLK);
        DELAY_US(20);

        iec_set_release(IO_CLK, IO_DATA);

        b >>= 1;
    }

    /* wait 2ms for data to be driven */
    ack = iec_wait_timeout_2ms(IO_DATA, IO_DATA);

    if (!ack) {
            DEBUGF("sndbyte nak\n");
    }

    return ack;
}

/*
 * Wait for listener to release DATA line, N second timeout
 *
 * Technically, this is the listener hold-off time and is allowed to be
 * infinite (e.g., for printers or other slow equipment).
 */
static uint8_t
wait_for_listener(void)
{
    /* release the clock line to indicate that we are ready */
    iec_release(IO_CLK);

    /* wait for client to do the same with the DATA line */
    while (iec_get(IO_DATA)) {
        DELAY_US(10);
    }

    return 1;
}

/* return number of successful written bytes or 0 on error */
uint8_t
cbm_raw_write(const uint8_t *buf, uint8_t len, uint8_t atn, uint8_t talk)
{
    uint8_t rv = len;

    eoi = 0;

    DEBUGF("cwr %d, atn %d\n", len, atn);

    iec_release(IO_DATA);
    iec_set(IO_CLK | (atn ? IO_ATN : 0));

    /* wait for any device to pull data */
    if (!iec_wait_timeout_2ms(IO_DATA, IO_DATA)) {
        DEBUGF("write: no devs\n");
        iec_release(IO_CLK | IO_ATN);
        return 0;
    }

    while (len && rv) {
        /* wait 50 us */
        DELAY_US(50);

        /* data line must be pulled by device */
        if (iec_get(IO_DATA)) {
            /* release clock and wait for listener to release data */
            if (!wait_for_listener()) {
                DEBUGF("w4l to\n");
                iec_release(IO_CLK | IO_ATN);
                return 0;
            }

            /* this is timing critical and if we are not sending an eoi */
            /* the iec_set(CLK) must be reached in less than ~150us. The USB */
            /* at 1.5MBit/s transfers 160 bits (20 bytes) in ~100us, this */
            /* should not interfere */

            if (len == 1 && !atn) {
                /* signal eoi by waiting so long (>200us) that listener */
                /* pulls data */

                /* wait 2ms for data to be pulled */
                iec_wait_timeout_2ms(IO_DATA, IO_DATA);

                /* wait 2ms for data to be release */
                iec_wait_timeout_2ms(IO_DATA, 0);
            }

            /* wait 10 us, why 10?? This delay is the most likely to be hit */
            /* by an USB irq */
            DELAY_US(10);

            iec_set(IO_CLK);

            if (send_byte(*buf++)) {
                len--;
                board_update_display();
                DELAY_US(100);
            } else {
                DEBUGF("write: io err\n");
                rv = 0;
            }
        } else {
            DEBUGF("write: dev not pres\n");
            rv = 0;
        }
    }

    if (talk) {
        iec_set(IO_DATA);
        iec_release(IO_CLK | IO_ATN);
        while (iec_get(IO_CLK) == 0);
    } else {
        iec_release(IO_ATN);
    }
    DELAY_US(100);

    DEBUGF("wrv=%d\n", rv);
    return rv;
}

/* return number of successful written bytes or 0 on error */
uint8_t
cbm_raw_read(uint8_t *buf, uint8_t len)
{
    uint8_t ok, bit, b, count;
    uint16_t to;

    DEBUGF("crd %d\n", len);
    count = 0;
    do {
        to = 0;

        /* wait for clock to be released. typically times out during: */
        /* directory read */
        while (iec_get(IO_CLK)) {
            if (to >= 50000) {
                /* 1.0 (50000 * 20us) sec timeout */
                DEBUGF("rd to\n");
                return 0;
            } else {
                to++;
                DELAY_US(20);
            }
        }

        if (eoi) {
            /* re-enable interrupts and return */
            // XXX is this right?
            io_request = XUM1541_IO_READ_DONE;
            io_buffer_len = 0;
            return 0;
        }

        /* disable IRQs to make sure IEC transfer goes uninterrupted */

        /* release DATA line */
        iec_release(IO_DATA);

        /* use special "timer with wait for clock" */
        iec_wait_clk();

        if (iec_get(IO_CLK) == 0) {
            /* device signals eoi */
            eoi = 1;
            iec_set(IO_DATA);
            DELAY_US(70);
            iec_release(IO_DATA);
        }

        cli();

        /* wait 2ms for clock to be asserted */
        ok = iec_wait_timeout_2ms(IO_CLK, IO_CLK);

        /* read all bits of byte */
        for (bit = b = 0; bit < 8 && ok; bit++) {
            /* wait 2ms for clock to be released */
            ok = iec_wait_timeout_2ms(IO_CLK, 0);
            if (ok) {
                b >>= 1;
                if (iec_get(IO_DATA) == 0)
                    b |= 0x80;

                /* wait 2ms for clock to be asserted */
                ok = iec_wait_timeout_2ms(IO_CLK, IO_CLK);
            }
        }

        sei();

        /* acknowledge byte */
        if (ok)
            iec_set(IO_DATA);

        if (ok) {
            *buf++ = b;
            count++;
            board_update_display();
            DELAY_US(50);
        }

    } while (count != len && ok && !eoi);

    if (!ok) {
        DEBUGF("read io err\n");
        count = 0;
    }

    DEBUGF("rv=%d\n", count);
    return count;
}

/*
 * Main worker task for processing buffered commands.
 * This should be called after every new command is read in from the
 * host.
 */
void
xu1541_handle(void)
{

    if (io_request == XUM1541_IO_ASYNC) {
        DEBUGF("h-as\n");
        // write async cmd byte(s) used for (un)talk/(un)listen, open and close
        io_result = !cbm_raw_write(io_buffer+2, io_buffer_len,
            /*atn*/io_buffer[0], io_buffer[1]/*talk*/);

        io_request = XUM1541_IO_RESULT;
    }

    if (io_request == XUM1541_IO_WRITE) {
        DEBUGF("h-wr %d\n", io_buffer_len);
        io_result = cbm_raw_write(io_buffer, io_buffer_len,
            /*atn*/0, /*talk*/0);

        io_request = XUM1541_IO_RESULT;
    }

    if (io_request == XUM1541_IO_READ) {
        DEBUGF("h-rd %d\n", io_buffer_len);
        io_result = cbm_raw_read(io_buffer, io_buffer_len);

        io_request = XUM1541_IO_READ_DONE;
        io_buffer_len = io_result;
    }
}

void
xu1541_request_read(uint8_t len)
{

    io_request = XUM1541_IO_READ;
    io_buffer_len = len;
}

uint8_t
xu1541_read(uint8_t len)
{
    if (io_request != XUM1541_IO_READ_DONE) {
        DEBUGF("no rd (%d)\n", io_request);
        return 0;
    }
#if 0
    if (len != io_buffer_len) {
        DEBUGF("bad rd len (%d != %d)\n", len, io_buffer_len);
        return 0;
    }
#endif
    if (len > io_buffer_len)
        len = io_buffer_len;

    if (!USB_WriteBlock(io_buffer, len)) {
        DEBUGF("rd abrt\n");
        return 0;
    }

    io_buffer_len = 0;
    io_request = XUM1541_IO_IDLE;

    return len;
}

/* write to buffer */
uint8_t
xu1541_write(uint8_t len)
{
    DEBUGF("st %d\n", len);
    if (!USB_ReadBlock(io_buffer, len)) {
        DEBUGF("st err\n");
        return 0;
    }
    io_buffer_len = len;
    io_request = XUM1541_IO_WRITE;

    return len;
}

/* return result code from async operations */
void
xu1541_get_result(uint8_t *data)
{

    DEBUGF("r %d/%d\n", io_request, io_result);
    data[0] = io_request;
    data[1] = io_result;
}

void
xu1541_request_async(const uint8_t *buf, uint8_t len,
    uint8_t atn, uint8_t talk)
{

    io_request = XUM1541_IO_ASYNC;
    memcpy(io_buffer+2, buf, len);
    io_buffer_len = len;
    io_buffer[0] = atn;
    io_buffer[1] = talk;
}

/* wait forever for a specific line to reach a certain state */
uint8_t
xu1541_wait(uint8_t line, uint8_t state)
{
    uint8_t hw_mask, hw_state;

    /* calculate hw mask and expected state */
    hw_mask = iec2hw(line);
    hw_state = state ? hw_mask : 0;

    while ((iec_poll() & hw_mask) == hw_state) {
        DELAY_US(10);
    }

    return 0;
}

uint8_t
xu1541_poll(void)
{
    uint8_t iec_state, rv = 0;

    iec_state = iec_poll();
    if ((iec_state & IO_DATA) == 0)
        rv |= IEC_DATA;
    if ((iec_state & IO_CLK) == 0)
        rv |= IEC_CLOCK;
    if ((iec_state & IO_ATN) == 0)
        rv |= IEC_ATN;

    return rv;
}

void
xu1541_setrelease(uint8_t set, uint8_t release)
{
    iec_set_release(iec2hw(set), iec2hw(release));
}
