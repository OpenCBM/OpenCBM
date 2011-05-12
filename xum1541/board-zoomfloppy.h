/*
 * Board interface for the ZoomFloppy
 * Copyright (c) 2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _BOARD_ZOOMFLOPPY_H
#define _BOARD_ZOOMFLOPPY_H

// Initialize the board (timer, indicators, UART)
void board_init(void);
// Initialize the IO ports for IEC mode
void board_init_iec(void);

// Mapping of IEC lines to IO port output signals.
#define IO_CLK          _BV(0) // D0
#define IO_DATA         _BV(3) // D3
#define IO_SRQ          _BV(4) // D4
#define IO_RESET        _BV(6) // D6
#define IO_ATN          _BV(7) // C7
#define LED_MASK        _BV(2) // C2

// Input signals
#define IO_CLK_IN       _BV(1) // D1
#define IO_DATA_IN      _BV(2) // D2
#define IO_SRQ_IN       _BV(5) // D5
#define IO_RESET_IN     _BV(7) // D7
#define IO_ATN_IN       _BV(6) // C6

// Masks for setting data direction registers.
#define IO_MASK_C       (IO_ATN | LED_MASK)
#define IO_MASK_D       (IO_SRQ | IO_CLK | IO_DATA | IO_RESET)

// IEC and parallel port accessors
#define PAR_PORT_PORT   PORTB
#define PAR_PORT_DDR    DDRB
#define PAR_PORT_PIN    PINB

//
// ZOOMFLOPPY IEEE-488 Pinout  (Port,bit) OR (Input Port,Bit,Output Port,Bit)
//
#if MODEL == ZOOMFLOPPY
#define IEEE_SUPPORT    1
#endif

#define IEEE_EOI_IO     0xc5 // input, output
#define IEEE_ATN_I      0xc6 // input only
#define IEEE_ATN_O      0xc7 // output only
#define IEEE_DAV_IO     0xc4 // 
#define IEEE_IFC_I      0xd7 // 
#define IEEE_IFC_O      0xd6 // 
#define IEEE_SRQ_I      0xd5 // 
#define IEEE_SRQ_O      0xd4 // 
#define IEEE_NDAC_I     0xd2 // 
#define IEEE_NDAC_O     0xd3 // 
#define IEEE_NRFD_I     0xd1 // 
#define IEEE_NRFD_O     0xd0 // 
#define IEEE_REN_IO     0xc2 // 
#define IEEE_DATA_IO    0xb0 // Port B - Data

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
    uint8_t ret;

    switch (line) {
    case IO_SRQ:
        ret = PIND & IO_SRQ_IN;
        break;
    case IO_CLK:
        ret = PIND & IO_CLK_IN;
        break;
    case IO_DATA:
        ret = PIND & IO_DATA_IN;
        break;
    case IO_ATN:
        ret = PINC & IO_DATA_IN;
        break;
    case IO_RESET:
        ret = PINC & IO_RESET_IN;
        break;
    default:
        // Invalid set of requested signals, trigger WD reset
        for (;;) ;
    }

    return !ret;
}

INLINE void
iec_set(uint8_t line)
{
    if ((line & IO_ATN)) {
        PORTC |= IO_ATN;
        line &= ~IO_ATN;
    }
    if (line != 0)
        PORTD |= line;
}

INLINE void
iec_release(uint8_t line)
{
    if ((line & IO_ATN)) {
        PORTC &= ~IO_ATN;
        line &= ~IO_ATN;
    }
    if (line != 0)
        PORTD &= ~line;
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

// Since this is called with a runtime-specified mask, inlining doesn't help.
uint8_t iec_poll_pins(void);

// Status indicators (LEDs)
uint8_t board_get_status(void);
void board_set_status(uint8_t status);
void board_update_display(void);
bool board_timer_fired(void);

#endif // _BOARD_ZOOMFLOPPY_H
