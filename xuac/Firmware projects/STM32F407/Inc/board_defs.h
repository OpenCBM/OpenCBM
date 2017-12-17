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

// --- Board definitions ------------------------------------------------------

// Select board here:
#define STM32_F4VE_V2_0_1509
//#define DIY_MORE_STM32_407

#if defined(STM32_F4VE_V2_0_1509)

	#include "board_defs_STM32_F4VE_V2_0_1509.h"

#elif defined(DIY_MORE_STM32_407)

	#include "board_defs_DIY_MORE_STM32_407.h"

#else
	#error "Target board not specified in 'board_defs.h'."
#endif

// STM firmware package version:
#define STM_FW                 (uint8_t *) "STM32Cube_FW_F4_V1.17.0"

// ----------------------------------------------------------------------------

#endif /* __BOARD_DEFS_H */
