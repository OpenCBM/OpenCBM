/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __BOARD_H_
#define __BOARD_H_

#include "fw.h" // STM firmware
#include <stdint.h> // for uint8_t, uint16_t, uint32_t
#include "board_tape.h" // for Tape_HW

enum ECfgMode { cfg_nothing = 0, tape_cfg_capture = 1, tape_cfg_write = 2 };

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

// Structure for the inventory

typedef struct
{
	uint8_t  *Platform_Name;
	uint8_t  *Firmware_Version;
	uint8_t  *Firmware_Version_Major_Str;
	uint32_t  Firmware_Version_Major_Value;
	uint8_t  *Firmware_Version_Minor_Str;
	uint32_t  Firmware_Version_Minor_Value;
	uint8_t  *Firmware_Version_Sub_Str;
	uint32_t  Firmware_Version_Sub_Value;
	uint8_t  *Board_Name;
	uint8_t  *MCU_Name;
	uint8_t  *MCU_MHZ_Str;
	uint32_t  MCU_MHZ_Value;
	uint8_t  *Timer_Speed_MHZ_Str;
	uint32_t  Timer_Speed_MHZ_Value;
	uint8_t  *Buffer_Size_Str;
	uint32_t  Buffer_Size_Value;
	uint8_t  *Buffer_Size_Value_Str;
	uint8_t  *Sampling_Rate;
	uint32_t  Board_Capabilities;
} Board_HW_Info;


typedef struct
{
	enum ECfgMode CfgMode;
} Board_HW_Data;


typedef struct
{
	Board_HW_Info *info;
	Board_HW_Data *data;
	void         (*Init)                (void);
	void         (*Send_Board_Info_Dbg) (void);
	uint16_t     (*Board_Info)          (uint32_t InfoRequest);
	Tape_HW       *Tape;
} Board_HW_Operations;

#endif /* __BOARD_H_ */
