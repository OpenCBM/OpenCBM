/*
 * CPU initialization and timer routines for the USBKEY devkit (at90usb1287)
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _CPU_USBKEY_H
#define _CPU_USBKEY_H

// Initialize the CPU (clock rate, UART)
static inline void
cpu_init(void)
{
    // Disable clock division. This takes us from 1 MHz -> 8 MHz.
    clock_prescale_set(clock_div_1);

    // Enable watchdog timer and set for 4 seconds.
    wdt_enable(WDTO_4S);
}

// Timer and delay functions
#define DELAY_MS(x) _delay_ms(x)
#define DELAY_US(x) _delay_us(x)

#endif // _CPU_USBKEY_H
