/* Name: s2.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: s2.c,v 1.1 2007-02-04 12:36:34 harbaum Exp $
 *
 * $Log: s2.c,v $
 * Revision 1.1  2007-02-04 12:36:34  harbaum
 * Initial revision
 *
 *
 */

/* This file contains the "serial2" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include <avr/io.h>

#include "xu1541.h"
#include "s2.h"

static void s2_write_byte(unsigned char c) {
  char i;

  for(i=4; ; i--) {
    if(c & 1) { SET(DATA) } else { RELEASE(DATA); }
    c >>= 1;
    RELEASE(ATN);
    while(GET(CLK));

    if(c & 1) { SET(DATA) } else { RELEASE(DATA); }
    c >>= 1;
    SET(ATN);
    while(!GET(CLK));
  }
}

unsigned char s2_write(unsigned char *data, unsigned char len) {
  unsigned char i;

  for(i=0;i<len;i++)
    s2_write_byte(*data++);

  return len;
}

static unsigned char s2_read_byte(void) {
  unsigned char c = 0;
  char i;

  for(i=4; i>0; i--) {
    while(GET(CLK));
    c = (c>>1) | (GET(DATA) ? 0x80 : 0);
    RELEASE(ATN);
    while(!GET(CLK));
    c = (c>>1) | (GET(DATA) ? 0x80 : 0);
    SET(ATN);
  }
  return c;
}

unsigned char s2_read(unsigned char *data, unsigned char len) {
  unsigned char i;

  for(i=0;i<len;i++)
    *data++ = s2_read_byte();

  return len;
}

