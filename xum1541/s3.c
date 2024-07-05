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

    iec_srq_write(*c++);

    /* wait for clock change.
     * This signals that the drive is ready to receive
     */
    while (iec_get(IO_CLK)) {
        if (!TimerWorker())
            return;
    }

    iec_srq_write(*c);
}

void
s3_read_2_bytes(uint8_t *c)
{
    /* make sure we do not interfere with data */
    iec_release(IO_DATA);

    iec_set(IO_CLK);
    *c++ = iec_srq_read();

    iec_release(IO_CLK);
    *c = iec_srq_read();
}
