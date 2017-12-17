/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __ERROR_HANDLER_H_
#define __ERROR_HANDLER_H_

#include <stdint.h> // for uint8_t, uint16_t, uint32_t

// Structure for the inventory

typedef struct
{
	void (*ErrorHandler) (uint32_t ErrorNum, char *file, int line);
} ErrorHandler_Operations;

// Exported operations

void _Error_Handler(char *file, int line);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#endif /* __ERROR_HANDLER_H_ */
