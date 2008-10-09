/* Name: p2.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: p2.c,v 1.3 2008-10-09 18:55:45 strik Exp $
 *
 * $Log: p2.c,v $
 * Revision 1.3  2008-10-09 18:55:45  strik
 * Removed spaces and tabs before LF.
 *
 * Revision 1.2  2007/03/15 17:40:51  harbaum
 * Plenty of changes incl. first async support
 *
 * Revision 1.1  2007/02/18 19:47:32  harbaum
 * Bootloader and p2 protocol
 *
 *
 */

/* This file contains the "parallel2" helper functions for opencbm */
/* changes in the protocol must be reflected here. The parallel2 protocol */
/* is the parallel protocol used by libcbmcopy */

#include <avr/io.h>
#include <util/delay.h>

#include "xu1541.h"
#include "p2.h"

static void p2_write_byte(uchar c) {

  xu1541_pp_write(c);

  iec_release(CLK);
  while(iec_get(DATA));

  iec_set(CLK);
  while(!iec_get(DATA));
}

uchar p2_write(uchar *data, uchar len) {
  uchar i;

  for(i=0;i<len;i++)
    p2_write_byte(*data++);

  return len;
}

static uchar p2_read_byte(void) {
  uchar c;

  iec_release(CLK);
  while(iec_get(DATA));

  c = xu1541_pp_read();

  iec_set(CLK);
  while(!iec_get(DATA));

  return c;
}

uchar p2_read(uchar *data, uchar len) {
  uchar i;

  for(i=0;i<len;i++)
    *data++ = p2_read_byte();

  return len;
}

