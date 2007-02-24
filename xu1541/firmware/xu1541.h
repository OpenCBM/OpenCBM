/*
  xu1541.h - xu1541
*/

#ifndef XU1541_H
#define XU1541_H

#include "xu1541_types.h"
#include "version.h"

/* use port c for cbm io */
#define CBM_PORT  PORTC
#define CBM_DDR   DDRC
#define CBM_PIN   PINC

/* mapping of iec lines */
#define CLK   _BV(2)
#define DATA  _BV(3)
#define ATN   _BV(4)
#define RESET _BV(5)

/* the parallel port is mapped onto two avr ports */
#define PAR_PORT0_MASK   0xf8   /* port d pins 7-3 */
#define PAR_PORT0_DDR    DDRD
#define PAR_PORT0_PIN    PIND
#define PAR_PORT0_PORT   PORTD

#define PAR_PORT1_MASK   0x07   /* port b pins 2-0 */
#define PAR_PORT1_DDR    DDRB
#define PAR_PORT1_PIN    PINB
#define PAR_PORT1_PORT   PORTB

#define XU1541_DELAY { _udelay_us(10); }

/* makros are used to actually access the port pins */
#define POLL()           (CBM_PIN)
/* set line means: make it an output and drive it low */
#define SET(line)        { CBM_PORT &= ~(line); CBM_DDR |= (line); }
/* release means: make it an input and enable the pull-ups */
#define RELEASE(line)    { CBM_DDR &= ~(line); CBM_PORT |= (line); }
#define SET_RELEASE(s,r) { SET(s); RELEASE(r); }
#define GET(line)        ((CBM_PIN&(line))==0?1:0)

#ifndef DEBUG
#define LED_ON()     { PORTD &= ~_BV(1); }
#define LED_OFF()    { PORTD |=  _BV(1); }
#else
#define LED_ON()
#define LED_OFF()
#endif

extern unsigned char eoi;

/* exported functions */
extern void cbm_init(void);
extern void do_reset(void);
extern char cbm_raw_write(const unsigned char *buf, char cnt, char atn, char talk);
extern char xu1541_read(unsigned char *data, unsigned char len);
extern char xu1541_write(unsigned char *data, unsigned char len);

/* low level io on single lines */
extern char xu1541_wait(char line, char state);
extern char xu1541_poll(void);
extern char xu1541_set(char lines);
extern char xu1541_release(char lines);
extern char xu1541_setrelease(char set, char release);

/* low level io on parallel lines */
extern unsigned char xu1541_pp_read(void);
extern void xu1541_pp_write(unsigned char);

#endif // XU1541_H
