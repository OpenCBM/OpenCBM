/*
 * Board interface routines for the Pro Micro
 * Copyright (c) 2015 Marko Solajic <msolajic@gmail.com>
 * Copyright (c) 2014 Thomas Kindler <mail_xum@t-kindler.de>
 * Copyright (c) 2009-2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _BOARD_PROMICRO_H
#define _BOARD_PROMICRO_H

// Initialize the board (timer, indicators, UART)
void board_init(void);
// Initialize the IO ports for IEC mode
void board_init_iec(void);

// pinout is: PIN / PIN NAME ON BOARD

#define IO_DATA         _BV(2) // PB2 / 16
#define IO_CLK          _BV(3) // PB3 / 14
#define IO_ATN          _BV(4) // PB4 / 8
#define IO_SRQ          _BV(5) // PB5 / 9
#define IO_RESET        _BV(6) // PB6 / 10

#define IEC_PORT		PORTB
#define IEC_DDR			DDRB
#define IEC_PIN			PINB

#define LED_MASK        _BV(5) // PD5 / GREEN ONBOARD LED
#define LED_PORT        PORTD
#define LED_DDR         DDRD

// IEC and parallel port accessors
// Pins: 3, 2, RXI, TXO, A3, A2, A1, A0 make a Parallel Port
// in that order, coressponding to D0 - D7

#define PAR_PORT0_MASK   0xf0   /* port F pins 4-7 */
#define PAR_PORT0_DDR    DDRF
#define PAR_PORT0_PIN    PINF
#define PAR_PORT0_PORT   PORTF

#define PAR_PORT1_MASK   0x0f   /* port D pins 0-3 */
#define PAR_PORT1_DDR    DDRD
#define PAR_PORT1_PIN    PIND
#define PAR_PORT1_PORT   PORTD

#define SRQ_NIB_SUPPORT

/*
 * Use always_inline to override gcc's -Os option. Since we measured each
 * inline function's disassembly and verified the size decrease, we are
 * certain when we specify inline that we really want it.
 */
#define INLINE          static inline __attribute__((always_inline))

/*
 * Routines for getting/setting individual IEC lines and parallel port.
 *
 * We no longer add a short delay after changing line(s) state, even though
 * it takes about 0.5 us for the line to stabilize (measured with scope).
 * This is because we need to toggle SRQ quickly to send data to the 1571
 * and the delay was breaking our deadline.
 *
 * These are all inlines and this was incrementally measured that each
 * decreases the firmware size. Some (set/get) compile into a single
 * instruction (say, sbis). This works because the "line" argument is
 * almost always a constant.
 */

INLINE uint8_t
iec_get(uint8_t line)
{
    return (IEC_PIN & line) == 0;
}

INLINE void
iec_set(uint8_t line)
{
    IEC_DDR |= line;
}

INLINE void
iec_release(uint8_t line)
{
    IEC_DDR &= ~line;
}

INLINE void
iec_set_release(uint8_t s, uint8_t r)
{
    iec_set(s);
    iec_release(r);
}

// Make 8-bit port all inputs and read parallel value
INLINE uint8_t
iec_pp_read(void)
{
    uint8_t retval;

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

// Make 8-bits of port output and write out the parallel data
INLINE void
iec_pp_write(uint8_t val)
{
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

INLINE uint8_t
iec_srq_read(void)
{
    uint8_t i, data;

    data = 0;
    for (i = 8; i != 0; --i) {
        // Wait for the drive to pull IO_SRQ.
        while (!iec_get(IO_SRQ))
            ;

        // Wait for drive to release SRQ, then delay another 375 ns for DATA
        // to stabilize before reading it.
        while (iec_get(IO_SRQ))
            ;
        DELAY_US(0.375);

        // Read data bit
        data = (data << 1) | (iec_get(IO_DATA) ? 0 : 1);
   }

   return data;
}

INLINE void
iec_srq_write(uint8_t data)
{
    uint8_t i;

    for (i = 8; i != 0; --i) {
        if ((data & 0x80))   // send MSB
            iec_release(IO_DATA);
        else
            iec_set(IO_DATA);
        iec_set(IO_SRQ);     // set SRQ
        data <<= 1;          // next bit
        DELAY_US(0.3);       // (nibtools relies on this timing, do not change)
        iec_release(IO_SRQ); // release SRQ
        DELAY_US(0.935);     // (nibtools relies on this timing, do not change)
    }
}

// Since this is called with a runtime-specified mask, inlining doesn't help.
uint8_t iec_poll_pins(void);

// Status indicators (LEDs)
uint8_t board_get_status(void);
void board_set_status(uint8_t status);
void board_update_display(void);
bool board_timer_fired(void);

#endif // _BOARD_PROMICRO_H
