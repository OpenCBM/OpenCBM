/*
 * Name: pp.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: pp.c,v 1.2 2009-12-11 20:28:17 natelawson Exp $
 *
 * $Log $
 * Revision 1.3  2008/10/09 18:55:45  strik
 * Removed spaces and tabs before LF.
 *
 * Revision 1.2  2007/03/15 17:40:51  harbaum
 * Plenty of changes incl. first async support
 *
 * Revision 1.1.1.1  2007/02/04 12:36:34  harbaum
 * Initial version
 */

/* This file contains the "parallel" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include "xum1541.h"

void
pp_write_2_bytes(uint8_t *c)
{
    while (!iec_get(IO_DATA));
    xu1541_pp_write(*c++);
    iec_release(IO_CLK);

    while (iec_get(IO_DATA));
    xu1541_pp_write(*c++);
    iec_set(IO_CLK);
}

void
pp_read_2_bytes(uint8_t *c)
{
    while (!iec_get(IO_DATA));
    *c++ = xu1541_pp_read();
    iec_release(IO_CLK);

    while (iec_get(IO_DATA));
    *c++ = xu1541_pp_read();
    iec_set(IO_CLK);
}
