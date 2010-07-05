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

// Initialize the board (IO ports, indicators, UART)
void board_init(void);

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

// IEC and parallel port access functions
void iec_set(uint8_t line);
void iec_release(uint8_t line);
void iec_set_release(uint8_t s, uint8_t r);
uint8_t iec_get(uint8_t line);
uint8_t iec_poll(void);
uint8_t xu1541_pp_read(void);
void xu1541_pp_write(uint8_t val);

// Status indicators (LEDs)
uint8_t board_get_status(void);
void board_set_status(uint8_t status);
void board_update_display(void);
bool board_timer_fired(void);

#endif // _BOARD_ZOOMFLOPPY_H
