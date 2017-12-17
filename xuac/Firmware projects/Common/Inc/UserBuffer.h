/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __USERBUFFER_H_
#define __USERBUFFER_H_

#include <stdint.h> // for uint8_t, uint32_t

// Structures for the inventory

typedef struct _UserBufferData
{
	uint8_t *buffer;
	uint32_t readPos;
	uint32_t writePos;
	uint32_t entries;
	uint32_t capacity;
} UserBufferData;

typedef struct _UserBuffer
{
	UserBufferData  *data;
	uint32_t (*Capacity)   (struct _UserBuffer *ub);
	uint32_t (*Entries)    (struct _UserBuffer *ub);
	uint32_t (*Avail)      (struct _UserBuffer *ub);
	void     (*PushN)      (struct _UserBuffer *ub, uint8_t *srcbuf, uint32_t len);
	void     (*PopN)       (struct _UserBuffer *ub, uint8_t *trgbuf, uint32_t len);
	void     (*PushN_safe) (struct _UserBuffer *ub, uint8_t *srcbuf, uint32_t len);
	void     (*PopN_safe)  (struct _UserBuffer *ub, uint8_t *trgbuf, uint32_t len);
	void     (*Push)       (struct _UserBuffer *ub, uint8_t value);
	uint8_t  (*Pop)        (struct _UserBuffer *ub);
	void     (*Clear)      (struct _UserBuffer *ub);
} UserBuffer;

// Available public operations

uint32_t UserBuffer_Capacity   (UserBuffer *ub);
uint32_t UserBuffer_Entries    (UserBuffer *ub);
uint32_t UserBuffer_Avail      (UserBuffer *ub);
void     UserBuffer_PushN      (UserBuffer *ub, uint8_t *srcbuf, uint32_t len);
void     UserBuffer_PopN       (UserBuffer *ub, uint8_t *trgbuf, uint32_t len);
void     UserBuffer_PushN_safe (UserBuffer *ub, uint8_t *srcbuf, uint32_t len);
void     UserBuffer_PopN_safe  (UserBuffer *ub, uint8_t *trgbuf, uint32_t len);
void     UserBuffer_Push       (UserBuffer *ub, uint8_t value);
uint8_t  UserBuffer_Pop        (UserBuffer *ub);
void     UserBuffer_Clear      (UserBuffer *ub);

#endif /* __USERBUFFER_H_ */
