/*
 * Name: s1.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: s1.c,v 1.1 2009-12-09 06:10:23 natelawson Exp $
 *
 * $Log: s1.c,v $
 * Revision 1.1  2009-12-09 06:10:23  natelawson
 * Initial revision
 *
 * Revision 1.5  2007/03/17 19:31:34  harbaum
 * Some timeouts via hw timer
 *
 * Revision 1.4  2007/03/15 17:40:51  harbaum
 * Plenty of changes incl. first async support
 *
 * Revision 1.3  2007/03/08 11:16:23  harbaum
 * timeout and watchdog adjustments
 *
 * Revision 1.2  2007/02/04 15:12:04  harbaum
 * Fixed broken optimization in s1/s2 write byte
 *
 * Revision 1.1.1.1  2007/02/04 12:36:34  harbaum
 * Initial version
 */

/* This file contains the "serial1" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include "xum1541.h"

void
s1_write_byte(uint8_t c)
{
    uint8_t i;

    wdt_reset();

    for (i = 0; i < 8; i++, c <<= 1) {
        if ((c & 0x80) != 0)
            iec_set(IO_DATA);
        else
            iec_release(IO_DATA);
        iec_release(IO_CLK);
        while (!iec_get(IO_CLK));

        if ((c & 0x80) != 0)
            iec_release(IO_DATA);
        else
            iec_set(IO_DATA);
        while (iec_get(IO_CLK));

        iec_release(IO_DATA);
        iec_set(IO_CLK);
        while (!iec_get(IO_DATA));
    }
}

uint8_t
s1_read_byte(void)
{
    int8_t i;
    uint8_t b, c;

    wdt_reset();

    c = 0;
    for (i = 7; i >= 0; i--) {
        while (iec_get(IO_DATA));

        iec_release(IO_CLK);
        b = iec_get(IO_CLK);
        c = (c >> 1) | (b ? 0x80 : 0);
        iec_set(IO_DATA);
        while (b == iec_get(IO_CLK));

        iec_release(IO_DATA);

        while (!iec_get(IO_DATA));

        iec_set(IO_CLK);
    }

    return c;
}
