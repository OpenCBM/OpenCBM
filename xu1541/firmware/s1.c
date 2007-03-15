/* Name: s1.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: s1.c,v 1.4 2007-03-15 17:40:51 harbaum Exp $
 *
 * $Log: s1.c,v $
 * Revision 1.4  2007-03-15 17:40:51  harbaum
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
 *
 *
 */

/* This file contains the "serial1" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include <avr/io.h>
#include <avr/wdt.h>

#include "xu1541.h"
#include "s1.h"


static void s1_write_byte(uchar c) {
  uchar i;

  for(i=0; i<8; i++, c<<=1) {
    if(c & 0x80) { iec_set(DATA); } else { iec_release(DATA); }
    iec_release(CLK);

    while(!iec_get(CLK))
      wdt_reset();

    if(c & 0x80) { iec_release(DATA); } else { iec_set(DATA); }
    while(iec_get(CLK));
    iec_release(DATA);
    iec_set(CLK);
    while(!iec_get(DATA))
      wdt_reset();
  }
}

uchar s1_write(uchar *data, uchar len) {
  uchar i;

  for(i=0;i<len;i++)
    s1_write_byte(*data++);

  return len;
}

static uchar s1_read_byte(void) {
  char i;
  uchar b, c;

  c = 0;
  for(i=7; i>=0; i--) {
    while(iec_get(DATA))
      wdt_reset();

    iec_release(CLK);
    b = iec_get(CLK);
    c = (c >> 1) | (b ? 0x80 : 0);
    iec_set(DATA);
    while(b == iec_get(CLK))
      wdt_reset();

    iec_release(DATA);

    while(!iec_get(DATA))
      wdt_reset();

    iec_set(CLK);
  }
  return c;
}

uchar s1_read(uchar *data, uchar len) {
  uchar i;

  for(i=0;i<len;i++)
    *data++ = s1_read_byte();

  return len;
}

