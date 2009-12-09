/*
 * Name: s2.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: s2.c,v 1.1 2009-12-09 06:10:23 natelawson Exp $
 *
 * $Log: s2.c,v $
 * Revision 1.1  2009-12-09 06:10:23  natelawson
 * Initial revision
 *
 * Revision 1.7  2007/03/17 19:31:34  harbaum
 * Some timeouts via hw timer
 *
 * Revision 1.6  2007/03/15 17:40:51  harbaum
 * Plenty of changes incl. first async support
 *
 * Revision 1.5  2007/03/08 11:16:23  harbaum
 * timeout and watchdog adjustments
 *
 * Revision 1.4  2007/03/03 14:54:14  harbaum
 * More 1571 adjustments
 *
 * Revision 1.3  2007/02/06 22:34:44  harbaum
 * Release IO_DATA after byte was written
 *
 * Revision 1.2  2007/02/04 15:12:04  harbaum
 * Fixed broken optimization in s1/s2 write byte
 *
 * Revision 1.1.1.1  2007/02/04 12:36:34  harbaum
 * Initial version
 */

/* This file contains the "serial2" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include "xum1541.h"

void
s2_write_byte(uint8_t c)
{
    uint8_t i;

    wdt_reset();

    for (i = 0; i < 4; i++) {
        if ((c & 1) != 0)
            iec_set(IO_DATA);
        else
            iec_release(IO_DATA);
        c >>= 1;
        iec_release(IO_ATN);
        while (iec_get(IO_CLK));

        if ((c & 1) != 0)
            iec_set(IO_DATA);
        else
            iec_release(IO_DATA);
        c >>= 1;
        iec_set(IO_ATN);

        while (!iec_get(IO_CLK));
    }

    iec_release(IO_DATA);
}

uint8_t
s2_read_byte(void)
{
    uint8_t c = 0;
    int8_t i;

    wdt_reset();

    for (i = 4; i > 0; i--) {
        while (iec_get(IO_CLK));

        c = (c >> 1) | (iec_get(IO_DATA) ? 0x80 : 0);
        iec_release(IO_ATN);
        while (!iec_get(IO_CLK));

        c = (c >> 1) | (iec_get(IO_DATA) ? 0x80 : 0);
        iec_set(IO_ATN);
    }

    return c;
}
