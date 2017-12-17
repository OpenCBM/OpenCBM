/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __MEM_H_
#define __MEM_H_

#include <stdint.h> // for uint8_t, uint16_t, uint32_t
#include "UserBuffer.h"

// Structure for the inventory

typedef struct
{
	UserBuffer *MainUserBuffer;
	UserBuffer *CommandBuffer;
	uint16_t  (*MainUserBuffer_Clear)  (void);
	uint16_t  (*MainUserBuffer_isFull) (void);
	uint16_t  (*MainUserBuffer_Fill)   (uint32_t NumBytes);
} _Memory;

// Exported operations.

uint16_t Memory_MainUserBuffer_Clear  (void);
uint16_t Memory_MainUserBuffer_isFull (void);
uint16_t Memory_MainUserBuffer_Fill   (uint32_t NumBytes);

#endif /* __MEM_H_ */
