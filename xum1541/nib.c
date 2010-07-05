/*
 * MNIB/nibtools compatible parallel read/write routines
 * Copyright (c) 2009-2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include "xum1541.h"

/*
 * Timeout count for a burst read/write.
 *
 * At 8 Mhz, this is ~48 milliseconds, depending on how tight the loop is.
 * This should be more than enough to transfer a byte, given that we only
 * have about 25 us/byte at 40 KB/s.
 */
#define TO_HANDSHAKED_READ    0xffff

/*
 * ATN/DATA handshaked read from the drive.
 *
 * It takes about 2 us for the drive to pull DATA once we set ATN.
 * Then it takes about 13 us for it to release DATA once a byte is ready.
 * Once we release ATN, it takes another 2 us to release DATA.
 */
uint8_t
nib_parburst_read()
{
    uint8_t data;

    iec_set_release(IO_ATN, IO_DATA|IO_CLK);
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
            DEBUGF(DBG_ERROR, "nbrdh1 to\n");
            return -1;
        }
    }

    // Read it directly from the port without debouncing.
    *data = xu1541_pp_read();
    return 0;
}

/*
 * ATN/DATA handshaked write to the drive.
 *
 * It takes about 13 us for the drive to release DATA once we set ATN.
 * Once we release ATN, it takes the drive 2-5 us to set DATA.
 * However, we need to keep the data valid for a while after releasing
 * ATN so the drive can register it. 5 us always fails but 10 us works.
 */
void
nib_parburst_write(uint8_t data)
{

    iec_set_release(IO_ATN, IO_DATA|IO_CLK);
    DELAY_US(5);
    while (iec_get(IO_DATA) != 0)
        ;

    xu1541_pp_write(data);
    DELAY_US(1);
    iec_release(IO_ATN);

    /*
     * Hold parallel data ready until the drive can read it. Even though
     * the drive is supposed to grab DATA after it has gotten the byte ok,
     * it seems to do so prematurely. Thus we have to add this idle loop
     * to keep the data valid, and by that point, DATA has been set for
     * a while.
     */
    DELAY_US(10);
    while (iec_get(IO_DATA) == 0)
        ;

    // Read from parallel port, making the outputs inputs. (critical)
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
            DEBUGF(DBG_ERROR, "nbwrh to\n");
            return -1;
        }
    }

    // Write out the data value via parallel.
    xu1541_pp_write(data);
    return 0;
}
