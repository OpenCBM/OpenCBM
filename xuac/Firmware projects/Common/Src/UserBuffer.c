/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "usbd_cdc.h"
#include "UserBuffer.h"

// Implementation

uint32_t UserBuffer_Capacity(UserBuffer *ub)
{
	return ub->data->capacity;
}


uint32_t UserBuffer_Entries(UserBuffer *ub)
{
	return ub->data->entries;
}


uint32_t UserBuffer_Avail(UserBuffer *ub)
{
	return ub->data->capacity - ub->data->entries;
}


void UserBuffer_PushN(UserBuffer *ub, uint8_t *srcbuf, uint32_t len)
{
	UserBufferData *d = ub->data;

	for (uint32_t i = 0; i < len; i++)
	{
		d->buffer[d->writePos] = srcbuf[i];
		d->writePos = (d->writePos + 1) % d->capacity;
	}

	d->entries += len;
}


void UserBuffer_PushN_safe(UserBuffer *ub, uint8_t *srcbuf, uint32_t len)
{
	UserBufferData *d = ub->data;

	for (uint32_t i = 0; i < len; i++)
	{
		d->buffer[d->writePos] = srcbuf[i];
		d->writePos = (d->writePos + 1) % d->capacity;
	}

	// Safely update #entries
	__disable_irq(); // Global interrupt disable (PRIMASK - Primary Interrupt Mask)
	__ISB();
	__DSB();
	d->entries += len;
	__DSB();
	__enable_irq();
}

void UserBuffer_Push(UserBuffer *ub, uint8_t value)
{
	UserBufferData *d = ub->data;
	d->buffer[d->writePos] = value;
	d->writePos = (d->writePos + 1) % d->capacity;

	// Safely update #entries
	__disable_irq(); // Global interrupt disable (PRIMASK - Primary Interrupt Mask)
	__ISB();
	__DSB();
	d->entries++;
	__DSB();
	__enable_irq();
}

void UserBuffer_PopN(UserBuffer *ub, uint8_t *trgbuf, uint32_t len)
{
	UserBufferData *d = ub->data;

	for (uint32_t i = 0; i < len; i++)
	{
		trgbuf[i] = d->buffer[d->readPos];
		d->readPos = (d->readPos + 1) % d->capacity;
	}

	ub->data->entries -= len;
}


void UserBuffer_PopN_safe(UserBuffer *ub, uint8_t *trgbuf, uint32_t len)
{
	UserBufferData *d = ub->data;

	for (uint32_t i = 0; i < len; i++)
	{
		trgbuf[i] = d->buffer[d->readPos];
		d->readPos = (d->readPos + 1) % d->capacity;
	}

	// Safely update #entries
	__disable_irq(); // Global interrupt disable (PRIMASK - Primary Interrupt Mask)
	__ISB();
	__DSB();
	ub->data->entries -= len;
	__DSB();
	__enable_irq();
}


uint8_t UserBuffer_Pop(UserBuffer *ub)
{
	UserBufferData *d = ub->data;

	uint8_t value = d->buffer[d->readPos];
	d->readPos = (d->readPos + 1) % d->capacity;

	d->entries--;

	return value;
}


void UserBuffer_Clear(UserBuffer *ub)
{
	// Safely reset UserBuffer.
	__disable_irq(); // Global interrupt disable (PRIMASK - Primary Interrupt Mask)
	__ISB();
	__DSB();

	ub->data->readPos = 0;
	ub->data->writePos = 0;
	ub->data->entries = 0;

	__DSB();
	__enable_irq();
}
