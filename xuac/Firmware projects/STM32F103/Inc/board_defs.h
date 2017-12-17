/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __BOARD_DEFS_H
#define __BOARD_DEFS_H

// --- Board info definitions -------------------------------------------------

// Select board here:
#define BOARD_BluePill
//#define BOARD_BlackPill

#if defined(BOARD_BluePill)

	#include "board_defs_BluePill.h"

#elif defined(BOARD_BlackPill)

	#include "board_defs_BlackPill.h"

#else
	#error "Target board not specified in 'board_defs.h'."
#endif

// STM firmware package version:
#define STM_FW                 (uint8_t *) "STM32Cube_FW_F1_V1.4.0"

// ----------------------------------------------------------------------------

#endif /* __BOARD_DEFS_H */
