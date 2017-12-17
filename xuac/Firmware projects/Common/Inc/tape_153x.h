/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __TAPE_153x_H_
#define __TAPE_153x_H_

#include <stdint.h> // for uint8_t, uint16_t, uint32_t
#include "tape_timer.h" // for _TapeTimerCtrl

// Structures for the inventory

typedef struct _TapeData
{
	uint32_t Sense_Delay_ms; // Default delay: 10 ms
	uint8_t  First_Write_Edge;
	uint8_t  First_Capture_Edge;
	uint32_t Lost_Signal_Edges_Count; // Lost signal edges while READing (two successive falling or rising edges).
	uint32_t Discarded_Signal_Count;  // Discarded signals while READing (send buffer full).
	uint32_t Overcapture_Count;       // Capture events on both channels at same time (corrupt timestamps).
} TapeData;

typedef struct
{
	TapeData  *data;
	uint16_t (*GetFirmwareVersion) (void);
	uint16_t (*MotorOn)            (void);
	uint16_t (*MotorOff)           (void);
	uint16_t (*PrepareCapture)     (void);
	uint16_t (*PrepareWrite)       (void);
	uint16_t (*GetSense)           (void);
	uint16_t (*WaitForStopSense)   (void);
	uint16_t (*WaitForPlaySense)   (void);
	uint16_t (*Capture)            (void);
	uint16_t (*Write)              (uint32_t ExpectedByteCount);
	uint8_t  (*Tape_Reset)         (void);
	_TapeTimerCtrl *TimerCtrl;
} Tape_Operations;

#endif /* __TAPE_153x_H_ */
