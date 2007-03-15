/* Name: s2.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: s2.c,v 1.6 2007-03-15 17:40:51 harbaum Exp $
 *
 * $Log: s2.c,v $
 * Revision 1.6  2007-03-15 17:40:51  harbaum
 * Plenty of changes incl. first async support
 *
 * Revision 1.5  2007/03/08 11:16:23  harbaum
 * timeout and watchdog adjustments
 *
 * Revision 1.4  2007/03/03 14:54:14  harbaum
 * More 1571 adjustments
 *
 * Revision 1.3  2007/02/06 22:34:44  harbaum
 * Release DATA after byte was written
 *
 * Revision 1.2  2007/02/04 15:12:04  harbaum
 * Fixed broken optimization in s1/s2 write byte
 *
 * Revision 1.1.1.1  2007/02/04 12:36:34  harbaum
 * Initial version
 */

/* This file contains the "serial2" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include <avr/io.h>
#include <avr/wdt.h>

#include "xu1541.h"
#include "s2.h"

static void s2_write_byte(uchar c) {
  uchar i;

  for(i=0; i<4; i++) {
    if(c & 1) { iec_set(DATA); } else { iec_release(DATA); }
    c >>= 1;
    iec_release(ATN);
    while(iec_get(CLK))
      wdt_reset();

    if(c & 1) { iec_set(DATA); } else { iec_release(DATA); }
    c >>= 1;
    iec_set(ATN);

    while(!iec_get(CLK))
      wdt_reset();
  }

  iec_release(DATA);
}

uchar s2_write(uchar *data, uchar len) {
  uchar i;

  for(i=0;i<len;i++)
    s2_write_byte(*data++);

  return len;
}

static uchar s2_read_byte(void) {
  uchar c = 0;
  char i;

  for(i=4; i>0; i--) {
    while(iec_get(CLK))
      wdt_reset();

    c = (c>>1) | (iec_get(DATA) ? 0x80 : 0);
    iec_release(ATN);
    while(!iec_get(CLK))
      wdt_reset();

    c = (c>>1) | (iec_get(DATA) ? 0x80 : 0);
    iec_set(ATN);
  }
  return c;
}

uchar s2_read(uchar *data, uchar len) {
  uchar i;

  for(i=0;i<len;i++)
    *data++ = s2_read_byte();

  return len;
}

