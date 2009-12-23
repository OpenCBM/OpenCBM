/*
 * xum1541 firmware defines
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#ifndef _XUM1541_H
#define _XUM1541_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include <LUFA/Version.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Board/LEDs.h>

#include "xum1541_types.h"      // Version and protocol definitions

// All supported models. Add new ones below.
#define AT90USBKEY              0

#if MODEL == AT90USBKEY
#include "cpu-usbkey.h"
#include "board-usbkey.h"
#endif

#ifdef DEBUG
#include <stdio.h>
#define DEBUGF(format, args...)  printf_P(PSTR(format), ##args)
#else
#define DEBUGF(format, args...)
#endif

// USB parameters for descriptor configuration
#define XUM_DATA_DIR_NONE       0x0f

// Status levels to notify the user (e.g. LEDS)
#define STATUS_INIT             0
#define STATUS_CONNECTING       1
#define STATUS_READY            2
#define STATUS_ACTIVE           3
#define STATUS_ERROR            4

// USB IO functions and command handlers
int8_t usbHandleControl(uint8_t cmd, uint8_t *replyBuf);
int8_t usbHandleBulk(uint8_t *request, uint8_t *replyBuf);
void USB_ResetConfig(void);
bool USB_ReadBlock(uint8_t *buf, uint8_t len);
bool USB_WriteBlock(uint8_t *buf, uint8_t len);
void xu1541_handle(void);

// IEC functions
#define IEC_DELAY  (0.5)        // 500 ns
extern uint8_t eoi;
extern volatile bool doDeviceReset;
void cbm_init(void);
void xu1541_reset(void);
void xu1541_get_result(uint8_t *data);
uint8_t xu1541_wait(uint8_t line, uint8_t state);
uint8_t xu1541_poll(void);
void xu1541_setrelease(uint8_t set, uint8_t release);
void xu1541_request_async(const uint8_t *buf, uint8_t len, uint8_t atn,
    uint8_t talk);
uint8_t xu1541_write(uint8_t len);
void xu1541_request_read(uint8_t len);
uint8_t xu1541_read(uint8_t len);

/*
 * Special protocol handlers:
 * s1 - serial
 * s2 - serial
 * p2 - parallel
 * pp - parallel
 * nib - nibbler parallel
 */
uint8_t s1_read_byte(void);
void s1_write_byte(uint8_t c);
uint8_t s2_read_byte(void);
void s2_write_byte(uint8_t c);
uint8_t p2_read_byte(void);
void p2_write_byte(uint8_t c);
void pp_read_2_bytes(uint8_t *c);
void pp_write_2_bytes(uint8_t *c);
uint8_t nib_parburst_read(void);
int8_t nib_read_handshaked(uint8_t *c, uint8_t toggle);
void nib_parburst_write(uint8_t data);
int8_t nib_write_handshaked(uint8_t data, uint8_t toggle);

#endif // _XUM1541_H
