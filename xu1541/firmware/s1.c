/* Name: s1.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: s1.c,v 1.2 2007-02-04 15:12:04 harbaum Exp $
 *
 * $Log: s1.c,v $
 * Revision 1.2  2007-02-04 15:12:04  harbaum
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

#include "xu1541.h"
#include "s1.h"

static void s1_write_byte(unsigned char c) {
  unsigned char i;

  for(i=0; i<8; i++, c<<=1) {
    if(c & 0x80) { SET(DATA); } else { RELEASE(DATA); }
    RELEASE(CLK);
    while(!GET(CLK));
    if(c & 0x80) { RELEASE(DATA); } else { SET(DATA); }
    while(GET(CLK));
    RELEASE(DATA);
    SET(CLK);
    while(!GET(DATA));
  }
}

unsigned char s1_write(unsigned char *data, unsigned char len) {
  unsigned char i;

  for(i=0;i<len;i++)
    s1_write_byte(*data++);

  return len;
}

static unsigned char s1_read_byte(void) {
  char i;
  unsigned char b, c;

  c = 0;
  for(i=7; i>=0; i--) {
    while(GET(DATA));
    RELEASE(CLK);
    b = GET(CLK);
    c = (c >> 1) | (b ? 0x80 : 0);
    SET(DATA);
    while(b == GET(CLK));
    RELEASE(DATA);
    while(!GET(DATA));
    SET(CLK);
  }
  return c;
}

unsigned char s1_read(unsigned char *data, unsigned char len) {
  unsigned char i;

  for(i=0;i<len;i++)
    *data++ = s1_read_byte();

  return len;
}

