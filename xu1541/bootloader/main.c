/*
  AVRUSBBoot - USB bootloader for Atmel AVR controllers

  Thomas Fischl <tfischl@gmx.de>

  License:
  The project is built with AVR USB driver by Objective Development, which is
  published under a proprietary Open Source license. To conform with this
  license, USBasp is distributed under the same license conditions. See
  documentation.

  Target.........: ATMega8 at 12 MHz
  Creation Date..: 2006-03-18
  Last change....: 2006-06-25

  To adapt the bootloader to your hardware, you have to modify the following files:
  - bootloaderconfig.h:
    Define the condition when the bootloader should be started
  - usbconfig.h:
    Define the used data line pins. You have to adapt USB_CFG_IOPORT, USB_CFG_DMINUS_BIT and 
    USB_CFG_DPLUS_BIT to your hardware. The rest should be left unchanged.
*/

#define F_CPU 12000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/boot.h>
#include <util/delay.h>

#ifndef USBTINY
// use avrusb library
#include "usbdrv.h"
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

#define USBBOOT_FUNC_WRITE_PAGE 2
#define USBBOOT_FUNC_LEAVE_BOOT 1
#define USBBOOT_FUNC_GET_PAGESIZE 3

#define STATE_IDLE 0
#define STATE_WRITE_PAGE 1

static uchar state = STATE_IDLE;
static unsigned int page_address;
static unsigned int page_offset;

void (*jump_to_app)(void) = 0x0000;

void leaveBootloader() {
      cli();
      boot_rww_enable();
      GICR = (1 << IVCE);  /* enable change of interrupt vectors */
      GICR = (0 << IVSEL); /* move interrupts to application flash section */
      jump_to_app();
}

#ifndef USBTINY
uchar   usbFunctionSetup(uchar data[8]) {
  static uchar replyBuf[8];
  usbMsgPtr = replyBuf;
#else
extern  byte_t  usb_setup ( byte_t data[8] ) {
  byte_t *replyBuf = data;
#endif
  uchar len = 0;
  
  if (data[1] == USBBOOT_FUNC_LEAVE_BOOT) {
    leaveBootloader();
  } else if (data[1] == USBBOOT_FUNC_WRITE_PAGE) {
    
    state = STATE_WRITE_PAGE;
    
    page_address = (data[3] << 8) | data[2]; /* page address */
    page_offset = 0;
    
    eeprom_busy_wait();
    cli();
    boot_page_erase(page_address); /* erase page */
    sei();
    boot_spm_busy_wait(); /* wait until page is erased */
    
#ifdef USBTINY
    /* usbtiny always returns 0 on write */
    len = 0;
#else
    len = 0xff; /* multiple out */
#endif
   
  } else if (data[1] == USBBOOT_FUNC_GET_PAGESIZE) {
    
    replyBuf[0] = SPM_PAGESIZE >> 8;
    replyBuf[1] = SPM_PAGESIZE & 0xff;
    return 2;

  }
  return len;
}

#ifndef USBTINY
uchar usbFunctionWrite( uchar *data, uchar len )
#else
void usb_out ( byte_t* data, byte_t len )
#endif
{
  uchar i;

  /* check if we are in correct state */
  if (state != STATE_WRITE_PAGE)
#ifndef USBTINY
    return 0xff;
#else
    return;
#endif
  
  for (i = 0; i < len; i+=2) {

    cli();    
    boot_page_fill(page_address + page_offset, data[i] | (data[i + 1] << 8));
    sei();
    page_offset += 2;

    /* check if we are at the end of a page */
    if (page_offset >= SPM_PAGESIZE) {
      
      /* write page */
      cli();
      boot_page_write(page_address);
      sei();
      boot_spm_busy_wait();

      state = STATE_IDLE;
#ifndef USBTINY
      return 1;
#else
      return;
#endif
    }

  }
  
#ifndef USBTINY
  return 0;
#endif
}

int main(void)
{
    /* check if portb.4 (miso) is tied to gnd and call main application if not */
    PORTB |= _BV(4);    // drive pin high
    DDRB  &=  ~_BV(4);  // pin is input (with pullup)

    // check if pin goes high
    if(PINB & _BV(4))
      leaveBootloader();

    /* make led output and switch it on */
    DDRD  |=  _BV(1);
    PORTD &= ~_BV(1);

    /* clear usb ports */
    USB_CFG_IOPORT   &= (uchar)~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));

    /* make usb data lines inputs */
    USBDDR &= ~((1<<USB_CFG_DMINUS_BIT)|(1<<USB_CFG_DPLUS_BIT));

    GICR = (1 << IVCE);  /* enable change of interrupt vectors */
    GICR = (1 << IVSEL); /* move interrupts to boot flash section */

    usbInit();
    sei();

    for(;;){    /* main event loop */
        usbPoll();
    }
    return 0;
}

