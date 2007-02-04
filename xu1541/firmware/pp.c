/* Name: pp.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: pp.c,v 1.1 2007-02-04 12:36:34 harbaum Exp $
 *
 * $Log: pp.c,v $
 * Revision 1.1  2007-02-04 12:36:34  harbaum
 * Initial revision
 *
 *
 */

/* This file contains the "parallel" helper functions for opencbm */
/* changes in the protocol must be reflected here. */

#include <avr/io.h>
#include <util/delay.h>

#include "xu1541.h"
#include "pp.h"

static void pp_write_2_bytes(unsigned char *c) {
  while(!GET(DATA));
  xu1541_pp_write(*c++);
  RELEASE(CLK);

  while(GET(DATA));
  xu1541_pp_write(*c++);
  SET(CLK);
}

unsigned char pp_write(unsigned char *data, unsigned char len) {
  unsigned char i;
  
  for(i=0;i<len;i+=2) {
    pp_write_2_bytes(data);
    data += 2;
  }

  return len;
}

static void pp_read_2_bytes(unsigned char *c) {
  while(!GET(DATA));
  *c++ = xu1541_pp_read();
  RELEASE(CLK);

  while(GET(DATA));
  *c++ = xu1541_pp_read();
  SET(CLK);
}

unsigned char pp_read(unsigned char *data, unsigned char len) {
  unsigned char i;

  for(i=0;i<len;i+=2) {
    pp_read_2_bytes(data);
    data += 2;
  }

  return len;
}

