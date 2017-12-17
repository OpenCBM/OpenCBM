/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __DBG_H_
#define __DBG_H_

#include <stdint.h> // for uint8_t, uint16_t, uint32_t

// Debug message max length.
#define DBG_MAX_LEN   (uint8_t) 79

// Default debug level at device startup.
#define DBGL_STD      (uint8_t) 2

// Debug levels
#define DBGL_MAIN     (uint8_t) 1
#define DBGL_USBFUNC  (uint8_t) 1
#define DBGL_INFO     (uint8_t) 1
#define DBGL_TAPE     (uint8_t) 2
#define DBGL_TAPE2    (uint8_t) 5
#define DBGL_TAPTIM   (uint8_t) 10
#define DBGL_XFER     (uint8_t) 1
#define DBGL_MEM      (uint8_t) 2
#define DBGL_MEM2     (uint8_t) 5
#define DBGL_CTLREQ   (uint8_t) 5
#define DBGL_ERR      (uint8_t) 1 // Errors

// Structure for the inventory

typedef struct
{
	uint8_t dbg_level;
	void (*dbg) (uint8_t level, const char *fmt, ...);
} Debug_Operations;

#endif /* __DBG_H_ */
