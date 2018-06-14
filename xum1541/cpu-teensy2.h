/*
 * CPU initialization and timer routines for the Teensy 2 (atmega32u4)
 * Copyright (c) 2014 Thomas Kindler <mail_xum@t-kindler.de>
 * Copyright (c) 2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _CPU_TEENSY2_H
#define _CPU_TEENSY2_H

// Initialize the CPU (clock rate, UART)
static inline void
cpu_init(void)
{
    // Disable clock division.
    clock_prescale_set(clock_div_1);

    // Enable watchdog timer and set for 1 second.
    wdt_enable(WDTO_1S);
}

static inline void
cpu_bootloader_start(void)
{
    // Disable timer and then jump to bootloader address
    TCCR1B = 0;
    OCR1A = 0;

    // Jump to Teensy's HalfKay bootloader
    __asm__ __volatile__ ("jmp 0x3F00" "\n\t");
}

// Timer and delay functions
#define DELAY_MS(x) _delay_ms(x)
#define DELAY_US(x) _delay_us(x)

#endif // _CPU_TEENSY2_H
