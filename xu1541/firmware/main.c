/* Name: main.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2005 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: main.c,v 1.7 2007-03-08 11:16:23 harbaum Exp $
 *
 * $Log: main.c,v $
 * Revision 1.7  2007-03-08 11:16:23  harbaum
 * timeout and watchdog adjustments
 *
 * Revision 1.6  2007/03/01 12:59:08  harbaum
 * Added event log
 *
 * Revision 1.5  2007/02/23 21:33:44  harbaum
 * USB echo test added
 *
 * Revision 1.4  2007/02/18 19:47:32  harbaum
 * Bootloader and p2 protocol
 *
 * Revision 1.3  2007/02/13 19:20:14  harbaum
 * activity LED
 *
 * Revision 1.2  2007/02/05 17:01:55  harbaum
 * Simplified version numbering
 *
 * Revision 1.1.1.1  2007/02/04 12:36:33  harbaum
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

#ifndef USBTINY
// use avrusb library
#include "usbdrv.h"
#include "oddebug.h"
#else
// use usbtiny library 
#include "usb.h"
#include "usbtiny.h"
typedef byte_t uchar;

#define USBDDR DDRC
#define USB_CFG_IOPORT PORTC

#define USB_CFG_DMINUS_BIT USBTINY_DMINUS
#define USB_CFG_DPLUS_BIT USBTINY_DPLUS

#define usbInit()  usb_init()
#define usbPoll()  usb_poll()
#endif

#define MODE_ORIGINAL  0
#define MODE_S1        1
#define MODE_S2        2
#define MODE_PP        3
#define MODE_P2        4
unsigned char io_mode;

#include "xu1541.h"
#include "s1.h"
#include "s2.h"
#include "pp.h"
#include "p2.h"
#include "event_log.h"

#ifdef DEBUG
#define DEBUGF(format, args...) printf_P(PSTR(format), ##args)

/* -------------------------- serial debugging ---------------------------- */
static int uart_putchar(char c, FILE *stream) {
  if (c == '\n')
    uart_putchar('\r', stream);
  loop_until_bit_is_set(UCSRA, UDRE);
  UDR = c;
  return 0;
}

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
                                         _FDEV_SETUP_WRITE);
#else
#define DEBUGF(format, args...)
#endif

/* ------------------------------------------------------------------------- */

#ifndef USBTINY
uchar	usbFunctionSetup(uchar data[8]) {
  static uchar replyBuf[4];
  usbMsgPtr = replyBuf;
#else
extern	byte_t	usb_setup ( byte_t data[8] )
{
  byte_t *replyBuf = data;
#endif
  char talk, rv;
  unsigned short len;

  DEBUGF("cmd %d (ioctl %d)\n", data[1], data[1] - XU1541_IOCTL);

  switch(data[1]) {

    /* ----- USB testing ----- */
    case XU1541_ECHO:
      DEBUGF("echo\n");
      replyBuf[0] = data[2];
      replyBuf[1] = data[3];
      replyBuf[2] = data[4];
      replyBuf[3] = data[5];
      return 4;
      break;

    /* ----- Debugging ----- */
#ifdef ENABLE_EVENT_LOG
    case XU1541_GET_EVENT:
      DEBUGF("get event\n");
      replyBuf[0] = EVENT_LOG_LEN;
      replyBuf[1] = event_log_get(data[2]);
      return 2;
      break;
#endif

    /* ----- Basic I/O ----- */
    case XU1541_INFO:
      replyBuf[0] = XU1541_VERSION_MAJOR;
      replyBuf[1] = XU1541_VERSION_MINOR;
      *(unsigned short*)(replyBuf+2) = XU1541_CAPABILIIES;
      return 4;
      break;

    case XU1541_READ:
      io_mode = MODE_ORIGINAL;
      len = *(unsigned short*)(data+2);
      DEBUGF("req to rd %d bytes\n", len);
      /* more data to be expected? */
      return(len?0xff:0x00);
      break;

    case XU1541_WRITE:
      io_mode = MODE_ORIGINAL;
      len = *(unsigned short*)(data+2);
#ifdef USBTINY
      /* usbtiny always returns 0 on write */
      return 0;
#else
      return(len?0xff:0x00);
#endif
      break;

    case XU1541_TALK:
    case XU1541_LISTEN:
      data[2] &= 0x1f;     /* device */
      data[3] &= 0x0f;     /* secondary address */

      DEBUGF("talk/listen(%d,%d)\n", data[2], data[3]);
      talk = (data[1] == XU1541_TALK);
      data[2] |= talk ? 0x40 : 0x20;
      data[3] |= 0x60;
      rv = cbm_raw_write(data+2, 2, 1, talk);
      replyBuf[0] = rv > 0 ? 0 : rv;
      return 1;
      break;

    case XU1541_UNTALK:
    case XU1541_UNLISTEN:
      DEBUGF("untalk/unlisten()\n");
      data[2] = (data[1] == XU1541_UNTALK) ? 0x5f : 0x3f;
      rv = cbm_raw_write(data+2, 1, 1, 0);
      replyBuf[0] = rv > 0 ? 0 : rv;
      return 1;
      break;

    case XU1541_OPEN:
    case XU1541_CLOSE:
      data[2] &= 0x1f;     /* device */
      data[3] &= 0x0f;     /* secondary address */

      DEBUGF("open/close(%d,%d)\n", data[2], data[3]);
      data[2] |= 0x20;
      data[3] |= (data[1] == XU1541_OPEN) ? 0xf0 : 0xe0;
      rv = cbm_raw_write(data+2, 2, 1, 0);
      replyBuf[0] = rv > 0 ? 0 : rv;
      return 1;
      break;
  
    case XU1541_GET_EOI:
      replyBuf[0] = eoi ? 1 : 0;
      return 1;
      break;
      
    case XU1541_CLEAR_EOI:
      eoi = 0;
      return 0;
      break;
      
    case XU1541_RESET:
      LED_OFF();
      DEBUGF("reset()\n");
      do_reset();
      return 0;
      break;

	 /* ----- Low-level port access ----- */

    case XU1541_IEC_WAIT:
      xu1541_wait(data[2], data[3]);
      /* intentional fall through */
      
    case XU1541_IEC_POLL:
      replyBuf[0] = xu1541_poll();
      DEBUGF("poll() = %x\n", replyBuf[0]);
      return 1;
      break;
      
    case XU1541_IEC_SET:
      replyBuf[0] = xu1541_set(data[2]);
      return 1;
      break;
      
    case XU1541_IEC_RELEASE:
      replyBuf[0] = xu1541_release(data[2]);
      return 1;
      break;
      
    case XU1541_IEC_SETRELEASE:
      replyBuf[0] = xu1541_setrelease(data[2], data[3]);
      return 1;
      break;
      
    case XU1541_PP_READ:
      replyBuf[0] = xu1541_pp_read();
      return 1;
      break;
      
    case XU1541_PP_WRITE:
      xu1541_pp_write(data[2]);
      return 0;
      break;

      /* ----- special protocol drivers access ----- */    
      
      /* ----- S1 ----- */
    case XU1541_S1:
      switch(data[2]) {
	case XU1541_INFO:
	  replyBuf[0] = S1_VERSION_MAJOR;
	  replyBuf[1] = S1_VERSION_MINOR;
	  return 2;
	  break;

	case XU1541_READ:
	  io_mode = MODE_S1;
	  len = *(unsigned short*)(data+4);
	  /* more data to be expected? */
	  return(len?0xff:0x00);
	  break;
	  
	case XU1541_WRITE:
	  io_mode = MODE_S1;
	  len = *(unsigned short*)(data+4);
#ifdef USBTINY
      /* usbtiny always returns 0 on write */
	  return 0;
#else
	  return(len?0xff:0x00);
#endif
	  break;

	break;
	default:
	  DEBUGF("ERROR: s1 command %d not implemented\n", data[1]);
	  break;
      }
      break;

      /* ----- S2 ----- */
    case XU1541_S2:
      switch(data[2]) {
	case XU1541_INFO:
	  replyBuf[0] = S2_VERSION_MAJOR;
	  replyBuf[1] = S2_VERSION_MINOR;
	  return 2;
	  break;

	case XU1541_READ:
	  io_mode = MODE_S2;
	  len = *(unsigned short*)(data+4);
	  /* more data to be expected? */
	  return(len?0xff:0x00);
	  break;
	  
	case XU1541_WRITE:
	  io_mode = MODE_S2;
	  len = *(unsigned short*)(data+4);
#ifdef USBTINY
      /* usbtiny always returns 0 on write */
	  return 0;
#else
	  return(len?0xff:0x00);
#endif
	  break;

	break;
	default:
	  DEBUGF("ERROR: s2 command %d not implemented\n", data[1]);
	  break;
      }
      break;
      
      /* ----- PP ----- */
    case XU1541_PP:
      switch(data[2]) {
	case XU1541_INFO:
	  replyBuf[0] = PP_VERSION_MAJOR;
	  replyBuf[1] = PP_VERSION_MINOR;
	  return 2;
	  break;

	case XU1541_READ:
	  io_mode = MODE_PP;
	  len = *(unsigned short*)(data+4);
	  /* more data to be expected? */
	  return(len?0xff:0x00);
	  break;
	  
	case XU1541_WRITE:
	  io_mode = MODE_PP;
	  len = *(unsigned short*)(data+4);
#ifdef USBTINY
	  /* usbtiny always returns 0 on write */
	  return 0;
#else
	  return(len?0xff:0x00);
#endif
	  break;

	break;
	default:
	  DEBUGF("ERROR: pp command %d not implemented\n", data[1]);
	  break;
      }
      break;
      
      /* ----- P2 ----- */
    case XU1541_P2:
      switch(data[2]) {
	case XU1541_INFO:
	  replyBuf[0] = P2_VERSION_MAJOR;
	  replyBuf[1] = P2_VERSION_MINOR;
	  return 2;
	  break;

	case XU1541_READ:
	  io_mode = MODE_P2;
	  len = *(unsigned short*)(data+4);
	  /* more data to be expected? */
	  return(len?0xff:0x00);
	  break;
	  
	case XU1541_WRITE:
	  io_mode = MODE_P2;
	  len = *(unsigned short*)(data+4);
#ifdef USBTINY
	  /* usbtiny always returns 0 on write */
	  return 0;
#else
	  return(len?0xff:0x00);
#endif
	  break;

	break;
	default:
	  DEBUGF("ERROR: p2 command %d not implemented\n", data[1]);
	  break;
      }
      break;
      
    default:
      DEBUGF("ERROR: command %d not implemented\n", data[0]);
      // must not happen ...
      break;
  }
  
  return 0;  // reply len
}


/*---------------------------------------------------------------------------*/
/* usbFunctionRead                                                           */
/*---------------------------------------------------------------------------*/

#ifndef USBTINY
uchar usbFunctionRead( uchar *data, uchar len )
#else
byte_t usb_in ( byte_t* data, byte_t len )
#endif
{
  char rv = 0;

  DEBUGF("rd %d\n", len);
  LED_ON();  

  switch(io_mode) {
    case MODE_ORIGINAL:
      rv = xu1541_read(data, len);
      break;

    case MODE_S1:
      rv = s1_read(data, len);
      break;

    case MODE_S2:
      rv = s2_read(data, len);
      break;

    case MODE_PP:
      rv = pp_read(data, len);
      break;

    case MODE_P2:
      rv = p2_read(data, len);
      break;
  }

  LED_OFF();  
  return (rv>0)?rv:0;
}

/*---------------------------------------------------------------------------*/
/* usbFunctionWrite                                                          */
/*---------------------------------------------------------------------------*/

#ifndef USBTINY
uchar usbFunctionWrite( uchar *data, uchar len )
#else
void usb_out ( byte_t* data, byte_t len )
#endif
{
  char rv = 0;  
  LED_ON();  

  switch(io_mode) {
    case MODE_ORIGINAL:
      rv = xu1541_write(data, len);
      break;

    case MODE_S1:
      rv = s1_write(data, len);
      break;

    case MODE_S2:
      rv = s2_write(data, len);
      break;

    case MODE_PP:
      rv = pp_write(data, len);
      break;

    case MODE_P2:
      rv = p2_write(data, len);
      break;
  }

  LED_OFF();  
#ifndef USBTINY
  return (rv>0)?rv:0;
#endif
}


/* ------------------------------------------------------------------------- */

int	main(void) {
  wdt_enable(WDTO_1S);

#ifdef ENABLE_EVENT_LOG
  event_log_init();
  EVENT(EVENT_START);

  /* save reset reasons */
  if(MCUCSR & (_BV(EXTRF)|_BV(PORF))) {
    EVENT(EVENT_RESET_EXT);
    MCUCSR &= ~_BV(EXTRF);
    MCUCSR &= ~_BV(PORF);
  }

  if(MCUCSR & _BV(WDRF)) {
    EVENT(EVENT_RESET_WATCHDOG);
    MCUCSR &= ~_BV(WDRF);
  }

  if(MCUCSR & _BV(BORF)) {
    EVENT(EVENT_RESET_BROWNOUT);
    MCUCSR &= ~_BV(BORF);
  }
#endif  


#if DEBUG_LEVEL > 0
  /* let debug routines init the uart if they want to */
  odDebugInit();
#else
#ifdef DEBUG
  /* quick'n dirty uart init @ 115200 bit/s */
  UCSRA |= _BV(U2X);
  UCSRB |= _BV(TXEN);
  UBRRL = F_CPU / (57600 * 16L) - 1;
#else
  /* no debugging at all, drive led on tx, led is on port d.1 */
  DDRD |= _BV(1);     /* pin is output */
  LED_ON();           /* drive led during startup */

  /* rxd (d.0) is iec_srq. we don't use it (yet) so just make it an input */
  DDRD &= ~_BV(0);     /* pin is input */  
#endif
#endif

#ifdef DEBUG
  stdout = &mystdout;
#endif

  DEBUGF("xu1541 %d.%02d - (c) 2007 by Till Harbaum\n", 
	 XU1541_VERSION_MAJOR, XU1541_VERSION_MINOR);

  cbm_init();

  /* clear usb ports */
  USB_CFG_IOPORT   &= (uchar)~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));

  /* make usb data lines outputs */
  USBDDR    |= ((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));

  /* USB Reset by device only required on Watchdog Reset */
  _delay_ms(10);

  /* make usb data lines inputs */
  USBDDR &= ~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));

  usbInit();

  sei();

  LED_OFF();

  for(;;) {	/* main event loop */
    wdt_reset();
    usbPoll();
  }

  return 0;
}

/* ------------------------------------------------------------------------- */
