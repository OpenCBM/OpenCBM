/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __BOARD_INFO_H_
#define __BOARD_INFO_H_

#include <stdint.h> // for uint8_t, uint16_t, uint32_t

// Board info: returned request maximum answer size.
#define BOARD_INFO_MAX_SIZE 50

// Exported operations.
uint16_t Board_Info(uint32_t InfoRequest);

#endif /* __BOARD_INFO_H_ */
