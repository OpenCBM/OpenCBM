/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __TAPE_TIMER_H_
#define __TAPE_TIMER_H_

#include "tape.h" // for eSignalEdge_t

// Structures for the inventory

typedef struct
{
	void (*Setup) (void);
	void (*Start) (void);
	void (*Stop)  (void);
} _TapeTimerCtrlOps_Read;

typedef struct
{
	void (*Setup) (eSignalEdge_t FirstSignalEdge);
	void (*Start) (void);
	void (*Stop)  (void);
} _TapeTimerCtrlOps_Write;

typedef struct
{
	_TapeTimerCtrlOps_Read  *Capture;
	_TapeTimerCtrlOps_Write *Write;
} _TapeTimerCtrl;

#endif /* __TAPE_TIMER_H_ */
