/*
 * Board interface routines for the PJRC Teensy 2
 * Copyright (c) 2014 Thomas Kindler <mail_xum@t-kindler.de>
 * Copyright (c) 2009-2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _BOARD_TEENSY2_H
#define _BOARD_TEENSY2_H

// Initialize the board (timer, indicators, UART)
void board_init(void);
// Initialize the IO ports for IEC mode
void board_init_iec(void);

// Mapping of IEC lines to IO port signals.
#define IO_DATA         _BV(0) // PF0
#define IO_CLK          _BV(1) // PF1
#define IO_ATN          _BV(4) // PF4
#define IO_SRQ          _BV(5) // PF5
#define IO_RESET        _BV(6) // PF6

#define LED_MASK        _BV(6) // PD6
#define LED_PORT        PORTD
#define LED_DDR         DDRD

// IEC and parallel port accessors
#define PAR_PORT_PORT   PORTB
#define PAR_PORT_DDR    DDRB
#define PAR_PORT_PIN    PINB
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
    return (PINF & line) == 0;
}

INLINE void
iec_set(uint8_t line)
{
    DDRF |= line;
}

INLINE void
iec_release(uint8_t line)
{
    DDRF &= ~line;
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
    PAR_PORT_DDR = 0;
    PAR_PORT_PORT = 0;
    return PAR_PORT_PIN;
}

// Make 8-bits of port output and write out the parallel data
INLINE void
iec_pp_write(uint8_t val)
{
    PAR_PORT_DDR = 0xff;
    PAR_PORT_PORT = val;
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

#endif // _BOARD_TEENSY2_H
