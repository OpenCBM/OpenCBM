/*
 * Name: p2.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: p2.c,v 1.4 2011-01-04 02:51:31 natelawson Exp $
 *
 * $Log $
 * Revision 1.3  2008/10/09 18:55:45  strik
 * Removed spaces and tabs before LF.
 *
 * Revision 1.2  2007/03/15 17:40:51  harbaum
 * Plenty of changes incl. first async support
 *
 * Revision 1.1  2007/02/18 19:47:32  harbaum
 * Bootloader and p2 protocol
 */

/* This file contains the "parallel2" helper functions for opencbm */
/* changes in the protocol must be reflected here. The parallel2 protocol */
/* is the parallel protocol used by libcbmcopy */

#include "xum1541.h"

/*
 * Send a byte to the drive.
 *
 * The delay after setting the parallel data is to be sure it is
 * ready on the port before signaling by toggling CLK. The parallel
 * lines transition quickly because there is little load due to the
 * drive keeping them in hi-Z. Thus the delays are not strictly needed,
 * at least at 16 MHz or below.
 */
void
p2_write_byte(uint8_t c)
{
    xu1541_pp_write(c);
    DELAY_US(0.5);

    iec_release(IO_CLK);
    while (iec_get(IO_DATA)) {
        if (!TimerWorker())
            return;
    }

    iec_set(IO_CLK);
    while (!iec_get(IO_DATA)) {
        if (!TimerWorker())
            return;
    }
}

uint8_t
p2_read_byte(void)
{
    uint8_t c;

    iec_release(IO_CLK);
    while (iec_get(IO_DATA)) {
        if (!TimerWorker())
            return -1;
    }

    c = xu1541_pp_read();

    iec_set(IO_CLK);
    while (!iec_get(IO_DATA)) {
        if (!TimerWorker())
            return -1;
    }

    return c;
}
