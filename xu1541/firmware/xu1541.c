/* Name: xu1541.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: xu1541.c,v 1.4 2007-03-01 12:59:08 harbaum Exp $
 *
 * $Log: xu1541.c,v $
 * Revision 1.4  2007-03-01 12:59:08  harbaum
 * Added event log
 *
 * Revision 1.3  2007/02/18 19:47:32  harbaum
 * Bootloader and p2 protocol
 *
 * Revision 1.2  2007/02/13 19:20:14  harbaum
 * activity LED
 *
 * Revision 1.1.1.1  2007/02/04 12:36:34  harbaum
 * Initial version
 *
 *
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include <util/delay.h>

#include "xu1541.h"
#include "event_log.h"

#ifdef DEBUG
#define DEBUGF(format, args...) printf_P(PSTR(format), ##args)
#else
#define DEBUGF(format, args...) 
#endif

/* specifiers for the lines (must match values from opencbm.h) */
#define IEC_DATA   0x01 /*!< Specify the DATA line */
#define IEC_CLOCK  0x02 /*!< Specify the CLOCK line */
#define IEC_ATN    0x04 /*!< Specify the ATN line */
#define IEC_RESET  0x08 /*!< Specify the RESET line */

/* fast conversion between logical and physical mapping */
static const unsigned char iec2hw_table[] PROGMEM = {
  0,
  DATA,
         CLK,
  DATA | CLK,
               ATN,
  DATA |       ATN,
         CLK | ATN,
  DATA | CLK | ATN,
                     RESET,
  DATA |             RESET,
         CLK |       RESET,
  DATA | CLK |       RESET,
               ATN | RESET,
  DATA |       ATN | RESET,
         CLK | ATN | RESET,
  DATA | CLK | ATN | RESET
};

static unsigned char iec2hw(char iec) {
  return pgm_read_byte(iec2hw_table + iec);
}

/* global variable to keep track of eoi state */
unsigned char eoi;

void cbm_init(void) {
  DEBUGF("cbm: init\n");

  RELEASE(ATN | CLK | DATA | RESET);
  _delay_us(100);

  //  do_reset();
}

/* wrapper around _delay_ms() since it doesn't work > 16ms */
static void delay_ms(unsigned char n) {
  while(n--) 
    _delay_ms(1);
}

static char check_if_bus_free(void) {
  RELEASE(ATN | CLK | DATA | RESET);

  // wait for the drive to have time to react
  _delay_us(100);

  // assert ATN
  SET(ATN);

  // now, wait for the drive to have time to react
  _delay_us(100);

  // if DATA is still unset, we have a problem.
  if (!GET(DATA)) {
    RELEASE(ATN | CLK | DATA | RESET);
    return 0;
  }

  // ok, at least one drive reacted. Now, test releasing ATN:    
  RELEASE(ATN);
  _delay_us(100);

  if (!GET(DATA)) {
    RELEASE(ATN | CLK | DATA | RESET);
    return 1;
  }

  RELEASE(ATN | CLK | DATA | RESET);
  return 0;
}

static void wait_for_free_bus(void) {
  short i=1;
  
  while (1) {
    wdt_reset();

    if (check_if_bus_free())
      break;
    
    ++i;
    
    if (i == 1000) {
      EVENT(EVENT_TIMEOUT_FREE_BUS);
      DEBUGF("Quiting because of timeout\n");
      break;
    }
    _delay_ms(1);
  }
}

void do_reset( void ) {
  DEBUGF("cbm: resetting devices\n");
  RELEASE(DATA | ATN | CLK);
  SET(RESET);
  delay_ms(100);
  RELEASE(RESET);
  
  DEBUGF("cbm: waiting for free bus...\n");
  wait_for_free_bus();
}

/*
 *  send byte
 */
static char send_byte(unsigned char b) {
  unsigned char i, ack = 0;

  for( i = 0; i < 8; i++ ) {

    /* each _bit_ takes a total of 90us to send ... */
    _delay_us(70);

    if( !(b & 1) ) 
      SET(DATA);

    RELEASE(CLK);
    _delay_us(20);

    SET_RELEASE(CLK, DATA);

    b >>= 1;
  }

  /* wait up to 2ms for ack */
  for( i = 0; (i < 200) && !(ack=GET(DATA)); i++ ) {
    _delay_us(10);
  }

#ifdef ENABLE_EVENT_LOG
  if(!ack) {
    EVENT(EVENT_BYTE_NAK);
  }
#endif

  return ack;
}

/* wait for listener to release DATA line, 5 second timeout */
static char wait_for_listener(void) {
  unsigned short a,b;

  /* release the clock line to indicate that we are ready */
  RELEASE(CLK);

  /* wait for client to do the same with the DATA line */
  for(a=0;GET(DATA)&&(a < XU1541_W4L_TIMEOUT * 100);a++) {
    for(b=0;GET(DATA)&&(b < 1000);b++) {
      wdt_reset();
      _delay_us(10);
    }
  }

  return !GET(DATA);
}

char cbm_raw_write(const unsigned char *buf, char cnt, char atn, char talk) {
  unsigned char c;
  char i;
  char rv = 0;
  char sent = 0;
  
  eoi = 0;

  DEBUGF("cbm_write: %d bytes, atn=%d\n", cnt, atn);

  RELEASE(DATA);
  SET(CLK | (atn ? ATN : 0));

  /* all devices will pull DATA down, 1ms timeout */
  for(i=0; (i<100) && !GET(DATA); i++) {
    wdt_reset();
    _delay_us(10);
  }

  /* noone pulls DATA? */
  if(!GET(DATA)) {
    DEBUGF("cbm_write: no devices found\n");
    RELEASE(CLK | ATN);
    EVENT(EVENT_WRITE_NO_DEV);
    return -1;
  }

  while(cnt > sent && rv == 0) {
    c = *buf++;

    /* wait 50 us */
    _delay_us(50);      

    /* data line must be pulled by device */
    if(GET(DATA)) {

      /* release clock and wait for listener to release data */
      if(!wait_for_listener()) {
	DEBUGF("w4l timeout\n");
	EVENT(EVENT_TIMEOUT_LISTENER);
	RELEASE(CLK | ATN);
	return -1;
      }

      if((sent == (cnt-1)) && (atn == 0)) {
	/* signal eoi y wating so long (>200us) that listener */
	/* pulls data */

	/* wait for data line to be pulled, wait max 1ms */
	for(i=0; (i<100) && !GET(DATA); i++) {
	  wdt_reset();
	  _delay_us(10);
	}
	
	/* wait for data line to be released, wait max 1ms */
	for(i=0; (i<100) && GET(DATA); i++) {
	  wdt_reset();
	  _delay_us(10);
	}
      }

      /* wait 10 us */
      _delay_us(10);

      SET(CLK);

      if(send_byte(c)) {
	sent++;
	_delay_us(100);
      } else {
	EVENT(EVENT_WRITE_FAILED);
	DEBUGF("cbm_write: I/O error\n");
	rv = -1;
      }
    } else {
      EVENT(EVENT_WRITE_DEV_NOT_PRESENT);
      DEBUGF("cbm_write: device not present\n");
      rv = -1;
    }
  }
  
  if(talk) {
    SET(DATA);
    RELEASE(ATN);
    RELEASE(CLK);
    while(!GET(CLK));
  } else {
    RELEASE(ATN);
  }
  _delay_us(100);
  
  DEBUGF("%d bytes sent, rv=%d\n", sent, rv);

  return (rv < 0) ? rv : (int)sent;
}

char xu1541_read(unsigned char *data, unsigned char len) {
  unsigned char i, ok, bit, b;
  unsigned char received = 0;

  do {
    i = 0;
    
    /* wait for clock to be released */
    while(GET(CLK)) {
      if( i >= 50 ) {
	/* 1 sec timeout */
	EVENT(EVENT_READ_TIMEOUT);
	DEBUGF("timeout\n");
	return -1;
      } else {
	i++;
	_delay_us(20);
      }
    }
    
    /* release DATA line */
    RELEASE(DATA);
    
    /* wait for CLK to be asserted */
    for(i = 0; (i < 40) && !(ok=GET(CLK)); i++) {
      _delay_us(10);
    }
    
    if(!ok) {
      /* device signals eoi */
      eoi = 1;
      SET(DATA);
      _delay_us(70);
      RELEASE(DATA);
    }
    
    /* wait for clock to be released */
    for(i = 0; i < 100 && !(ok=GET(CLK)); i++) {
      _delay_us(20);
    }
    
    /* read all bits of byte */
    for(bit = b = 0; (bit < 8) && ok; bit++) {
	 
      /* wait for clock to be asserted */
      for(i = 0; (i < 200) && !(ok=(GET(CLK)==0)); i++) {
	_delay_us(10);
      }
      if(ok) {
	b >>= 1;
	if(GET(DATA)==0) {
	  b |= 0x80;
	}
	for(i = 0; i < 100 && !(ok=GET(CLK)); i++) {
	  _delay_us(20);
	}
      }
    }
    
    /* acknowledge byte */
    if(ok) {
      SET(DATA);
    }
    
    if(ok) {
      data[received++] = b;
      _delay_us(50);
    }
  } while(received < len && ok && !eoi);
  
  if(!ok) {
    EVENT(EVENT_READ_ERROR);
    DEBUGF("cbm_read: I/O error\n");
    return -1;
  }

  return received;
}

char xu1541_write(unsigned char *data, unsigned char len) {
  return cbm_raw_write(data, len, 0, 0);
}

/* wait while a specific line to reach a certain state */
char xu1541_wait(char line, char state) {
  char hw_mask, hw_state;
  unsigned short i,j;

  /* calculate hw mask */
  switch(line) {
  case IEC_DATA:
    hw_mask = DATA;
    break;
  case IEC_CLOCK:
    hw_mask = CLK;
    break;
  case IEC_ATN:
    hw_mask = ATN;
    break;
  default:
    return -1;
  }

  hw_state = (state & 0xff) ? hw_mask : 0;
  j = i = 0;
  while((POLL() & hw_mask) == hw_state) {
    if(i >= 1000) {
      if(j++ > XU1541_W4L_TIMEOUT * 100) {
	EVENT(EVENT_TIMEOUT_IEC_WAIT);
	DEBUGF("iec_wait timeout\n");
	return -1;
      }
      
      i = 0;
    } else {
      i++;
      _delay_us(10);
    }
  }
  return 0;
}

char xu1541_poll(void) {
  char rv = 0;
  
  if((POLL() & DATA) == 0) rv |= IEC_DATA;
  if((POLL() & CLK)  == 0) rv |= IEC_CLOCK;
  if((POLL() & ATN)  == 0) rv |= IEC_ATN;
  
  return rv;
}

char xu1541_set(char lines) {
  if (lines & ~(IEC_DATA | IEC_CLOCK | IEC_ATN | IEC_RESET)) {
    // there was some bit set that is not recognized, return
    // with an error
    return -1;
  }
  
  SET(iec2hw(lines));
  return 0;
}

char xu1541_release(char lines) {
  if (lines & ~(IEC_DATA | IEC_CLOCK | IEC_ATN | IEC_RESET)) {
    // there was some bit set that is not recognized, return
    // with an error
    return -1;
  }

  RELEASE(iec2hw(lines));
  return 0;
}

char xu1541_setrelease(char set, char release) {
  if ( (set & ~(IEC_DATA | IEC_CLOCK | IEC_ATN | IEC_RESET))
	  || (release & ~(IEC_DATA | IEC_CLOCK | IEC_ATN | IEC_RESET))) {
    // there was some bit set that is not recognized, return
    // with an error
    return -1;
  }

  SET_RELEASE(iec2hw(set), iec2hw(release));

  return 0;
}

/* read byte from parallel port */
unsigned char xu1541_pp_read(void) {
  unsigned char retval;

  /* make port(s) input */
  PAR_PORT0_DDR &= ~PAR_PORT0_MASK;
  PAR_PORT1_DDR &= ~PAR_PORT1_MASK;

  /* disable pullups */
  PAR_PORT0_PORT &= ~PAR_PORT0_MASK;
  PAR_PORT1_PORT &= ~PAR_PORT1_MASK;

  /* and read value */
  retval  = PAR_PORT0_PIN & PAR_PORT0_MASK;
  retval |= PAR_PORT1_PIN & PAR_PORT1_MASK;

  return retval;
}

void xu1541_pp_write(unsigned char val) {

  /* make ports output */
  PAR_PORT0_DDR |= PAR_PORT0_MASK;
  PAR_PORT1_DDR |= PAR_PORT1_MASK;

  /* mask pins */
  PAR_PORT0_PORT &= ~PAR_PORT0_MASK;
  PAR_PORT1_PORT &= ~PAR_PORT1_MASK;
  
  /* and put data bits on port */
  PAR_PORT0_PORT |= val & PAR_PORT0_MASK;
  PAR_PORT1_PORT |= val & PAR_PORT1_MASK;
}
