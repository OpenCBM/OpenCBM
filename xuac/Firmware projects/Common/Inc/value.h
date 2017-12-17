/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __VALUE_H_
#define __VALUE_H_

#include <stdint.h> // for uint8_t, uint16_t, uint32_t

// Structure for the inventory

typedef struct
{
	uint16_t (*SetValue32) (uint8_t ValueID, uint32_t data);
	uint16_t (*GetValue32) (uint8_t ValueID, uint32_t *data);
} Data_Exchange_Operations;

#endif /* __VALUE_H_ */
