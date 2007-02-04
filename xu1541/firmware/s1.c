/* Name: s1.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: s1.c,v 1.1 2007-02-04 12:36:34 harbaum Exp $
 *
 * $Log: s1.c,v $
 * Revision 1.1  2007-02-04 12:36:34  harbaum
 * Initial revision
 *
 *
 */

/* This file contains the "serial1" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include <avr/io.h>

#include "xu1541.h"
#include "s1.h"

static void s1_write_byte(unsigned char c) {
  unsigned char b, i;

  for(i=7; ; i--) {
    b=(c >> i) & 1;
    if(b) { SET(DATA); } else { RELEASE(DATA); }
    RELEASE(CLK);
    while(!GET(CLK));
    if(b) { RELEASE(DATA); } else { SET(DATA); }
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

