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

// Timeout for read/write (XXX was 3300000 us or 3.3 seconds originally)
// At 8 Mhz, this is ~48 milliseconds, depending on how tight the loop is.
#define TO_HANDSHAKED_READ    0xffff

// TODO: verify timing values with scope
uint8_t
nib_parburst_read()
{
    uint8_t data;

    iec_release(IO_DATA|IO_CLK);
    iec_set(IO_ATN);

    DELAY_US(5);
    while (iec_get(IO_DATA) != 0)
        ;

    data = xu1541_pp_read();
    DELAY_US(1);
    iec_release(IO_ATN);

    while (iec_get(IO_DATA) == 0)
        ;
    return data;
}

// Caller must release IO_DATA before calling this function
int8_t
nib_read_handshaked(uint8_t *data, uint8_t toggle)
{
    uint16_t to;

    // Wait for a byte to be ready (data toggle matches expected value).
    to = TO_HANDSHAKED_READ;
    while (iec_get(IO_DATA) != toggle) {
        if (to-- == 0) {
            DEBUGF("nbrdh1 to\n");
            return -1;
        }
    }

    // Read it directly from the port without debouncing.
    *data = xu1541_pp_read();
    return 0;
}

// TODO: verify timing values with scope
void
nib_parburst_write(uint8_t data)
{

    iec_release(IO_DATA|IO_CLK);
    iec_set(IO_ATN);

    DELAY_US(5);
    while (iec_get(IO_DATA) != 0)
        ;

    xu1541_pp_write(data);
    DELAY_US(1);
    iec_release(IO_ATN);

    // Wait before checking for drive handshake (critical)
    DELAY_US(20);
    while (iec_get(IO_DATA) == 0)
        ;

    // Read from parallel port (critical)
    data = xu1541_pp_read();
}

// Caller must release IO_DATA before calling this function
int8_t
nib_write_handshaked(uint8_t data, uint8_t toggle)
{
    uint16_t to;

    // Wait for drive to be ready (data toggle matches expected value).
    to = TO_HANDSHAKED_READ;
    while (iec_get(IO_DATA) != toggle) {
        if (to-- == 0) {
            DEBUGF("nbwrh to\n");
            return -1;
        }
    }

    // Write out the data value via parallel.
    xu1541_pp_write(data);
    return 0;
}
