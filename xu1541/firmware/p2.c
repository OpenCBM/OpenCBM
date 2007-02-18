/* Name: p2.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: p2.c,v 1.1 2007-02-18 19:47:32 harbaum Exp $
 *
 * $Log: p2.c,v $
 * Revision 1.1  2007-02-18 19:47:32  harbaum
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

static void p2_write_byte(unsigned char c) {

  xu1541_pp_write(c);

  RELEASE(CLK);
  while(GET(DATA));

  SET(CLK);
  while(!GET(DATA));
}

unsigned char p2_write(unsigned char *data, unsigned char len) {
  unsigned char i;
  
  for(i=0;i<len;i++)
    p2_write_byte(*data++);

  return len;
}

static unsigned char p2_read_byte(void) {
  unsigned char c;

  RELEASE(CLK);
  while(GET(DATA));

  c = xu1541_pp_read();
  
  SET(CLK);
  while(!GET(DATA));
  
  return c;
}

unsigned char p2_read(unsigned char *data, unsigned char len) {
  unsigned char i;

  for(i=0;i<len;i++)
    *data++ = p2_read_byte();

  return len;
}

