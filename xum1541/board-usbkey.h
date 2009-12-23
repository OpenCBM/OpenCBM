/*
 * Board interface for the USBKEY development kit
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _BOARD_USBKEY_H
#define _BOARD_USBKEY_H

// Initialize the board (IO ports, indicators, UART)
void board_init(void);

/*
 * Mapping of iec lines to IO port signals.
 *
 * NOTE: the XAP1541 adapter Nate is using has separate I/O pins
 * for inputs and outputs, so we depend on the IN signal bits being
 * the OUT signal << 1.
 *
 * The below defined pins are the OUT signals only but we derive the
 * input pin numbers from them.
 */
#define IO_ATN          _BV(0)
#define IO_CLK          _BV(2)
#define IO_DATA         _BV(4)
#define IO_RESET        _BV(6)
#define IO_OUTPUT_MASK  (IO_ATN | IO_CLK | IO_DATA | IO_RESET)

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

#endif // _BOARD_USBKEY_H
