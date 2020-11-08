/*
 * CPU initialization and timer routines for the Pro Micro (atmega32u4)
 * Copyright (c) 2015 Marko Solajic <msolajic@gmail.com>
 * Copyright (c) 2014 Thomas Kindler <mail_xum@t-kindler.de>
 * Copyright (c) 2010 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _CPU_PROMICRO_H
#define _CPU_PROMICRO_H

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
    // this is uint16_t on the original Caterina bootloader,
    // and uint8_t on the sparkfun variant
    // However, as the value written is 0x7777 for uint16_t and
    // 0x77 for uint8_t, we are compatible with both bootloaders
    // if we do as if it were always uint16_t.
    //
    volatile static uint16_t *const bootKeyPtr = (volatile uint16_t *) 0x0800;
    enum { bootKey = 0x7777 };

    // Disable timer and then jump to bootloader address
    TCCR1B = 0;
    OCR1A = 0;

    // pretend that there was already a previous reset
    *bootKeyPtr = bootKey;

    // Jump to Caterina bootloader by doing a WD reset,
    // so the bootloader recognizes it as a HW reset
    //
    wdt_enable(WDTO_15MS);
    while (true)
        ;
}

// Timer and delay functions
#define DELAY_MS(x) _delay_ms(x)
#define DELAY_US(x) _delay_us(x)

#endif // _CPU_PROMICRO_H
