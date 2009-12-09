/*
 * MNIB/nibtools compatible parallel read/write routines
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include "xum1541.h"

// At 8 Mhz, we have about 192 clocks per byte
// timeout for read (XXX was 300000 us originally)
#define TO_HANDSHAKED_READ    0xffff

uint8_t
nib_parburst_read()
{
    uint8_t data;

    iec_release(IO_DATA|IO_CLK);
    iec_set(IO_ATN);

    _delay_us(200);
    while (iec_get(IO_DATA) != 0)
        ;

    data = xu1541_pp_read();
    _delay_us(5);
    iec_release(IO_ATN);

    _delay_us(10);
    while (iec_get(IO_DATA) == 0)
        ;
    return data;
}

int8_t
nib_read_handshaked(uint8_t *data, uint8_t toggle)
{
    uint8_t byteArray[3];
    uint16_t to;

    iec_release(IO_DATA);

    // Not needed, polling?
    //_delay_us(2);

    /* Wait for a byte to be ready */
    to = TO_HANDSHAKED_READ;
    while (iec_get(IO_DATA) != toggle) {
        if (to-- == 0) {
            DEBUGF("nbrdh1 to\n");
            return -1;
        }
        // XXX needed?
        _delay_us(1);
    }

    /* Debounce the value read from the parallel port */
    byteArray[0] = 0;
    byteArray[1] = 1;
    do {
        byteArray[2] = byteArray[1];
        byteArray[1] = byteArray[0];
        byteArray[0] = xu1541_pp_read();
        if (to-- == 0) {
            DEBUGF("nbrdh2 to\n");
            return -1;
        }
    } while (byteArray[0] != byteArray[1] && byteArray[1] != byteArray[2]);

    *data = byteArray[0];
    return 0;
}

void
nib_parburst_write(uint8_t data)
{

    iec_release(IO_DATA|IO_CLK);
    iec_set(IO_ATN);

    _delay_us(200);
    while (iec_get(IO_DATA) != 0)
        ;

    xu1541_pp_write(data);
    _delay_us(5);
    iec_release(IO_ATN);

    _delay_us(20);
    while (iec_get(IO_DATA) == 0)
        ;
    // dummy read
    data = xu1541_pp_read();
}

int8_t
nib_write_handshaked(uint8_t data, uint8_t toggle)
{
    uint16_t to;

    iec_release(IO_CLK);

    /* Wait for a byte to be ready */
    to = TO_HANDSHAKED_READ;
    while (iec_get(IO_DATA) != toggle) {
        if (to-- == 0) {
            DEBUGF("nbwrh to\n");
            return -1;
        }
        // XXX needed?
        _delay_us(1);
    }

    xu1541_pp_write(data);
    return 0;
}
