/*
 * Name: s3.c
 * Project: xum1541
 * Author: Spiro Trikaliotis
 * Tabsize: 4
 * Copyright: (c) 2024 by Spiro Trikaliotis
 * License: GPL
 *
 */

/* This file contains the "serial3" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include "xum1541.h"

INLINE uint8_t
s3_srq_read(void)
{
    uint8_t i, data;

    data = 0;
    for (i = 8; i != 0; --i) {
        // Wait for the drive to pull IO_SRQ.
        while (!iec_get(IO_SRQ))
            ;

        // Wait for drive to release SRQ, then delay another 375 ns for DATA
        // to stabilize before reading it.
        while (iec_get(IO_SRQ))
            ;
        DELAY_US(0.375);

        // Read data bit
        data = (data << 1) | (iec_get(IO_DATA) ? 0 : 1);
   }

   return data;
}

/*
 * Write out a byte by sending each bit on the DATA line (inverted) and
 * clocking the CIA with SRQ. We don't want clock jitter so the body of
 * the loop must not have any branches. At 500 Kbit/sec, each loop iteration
 * should take 2 us or 32 clocks per bit at 16 MHz.
 */
INLINE void
s3_srq_write(uint8_t data)
{
    uint8_t i;

    for (i = 8; i != 0; --i) {
        /* set SRQ: this signals that the data is not valid */
        iec_set(IO_SRQ);

        /* st DATA accordingly
         * Note: Make sure not to do this in one operation with
         * changing IO_SRQ. The setting above must be before changing
         * the data bit, the release must be afterwards to signal the
         * CIA/VIA that the data is valid again.
         */
        if (data & 0x80) {
            iec_release(IO_DATA);
        }
        else {
            iec_set(IO_DATA);
        }

        /* advance to next bit */
        data <<= 1;

        DELAY_US(2.1*0.3);       // (nibtools relies on this timing, do not change)

        /* now, signal that data is valid */
        iec_release(IO_SRQ);

        /* give the CIA/VIA time to get the data */
        DELAY_US(2.1*0.935);     // (nibtools relies on this timing, do not change)

        // Decrement i and loop: 3 clock cycles when branch taken
        // Total: 13 clocks per loop (minus delays); 19 clocks left.
    }
}

/*
 * Send 2 bytes to the drive.
 */

void
s3_write_2_bytes(uint8_t *c)
{
    /* wait for clock change.
     * This signals that the drive is ready to receive
     */
    while (!iec_get(IO_CLK)) {
        if (!TimerWorker())
            return;
    }

    s3_srq_write(*c++);

    /* wait for clock change.
     * This signals that the drive is ready to receive
     */
    while (iec_get(IO_CLK)) {
        if (!TimerWorker())
            return;
    }

    s3_srq_write(*c);
}

void
s3_read_2_bytes(uint8_t *c)
{
    /* make sure we do not interfere with data */
    iec_release(IO_DATA);

    iec_set(IO_CLK);
    *c++ = s3_srq_read();

    iec_release(IO_CLK);
    *c = s3_srq_read();
}
