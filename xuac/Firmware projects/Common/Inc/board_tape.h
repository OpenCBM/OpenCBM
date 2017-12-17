/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __BOARD_TAPE_H_
#define __BOARD_TAPE_H_

#include "tape.h" // for eSignalEdge_t
#include "fw.h" // STM FW

// Structure for the inventory

typedef struct
{
	//Tape_HW_Data *data;
	void (*Init)                   (void);
	void (*DeInit)                 (void);
	void (*Prepare_DISCONNECT)     (void);
	void (*Enable_DISCONNECT)      (void);
	void (*Disable_DISCONNECT)     (void);
	void (*Prepare_MOTOR)          (void);
	void (*MOTOR_On)               (void);
	void (*MOTOR_Off)              (void);
	void (*Prepare_SENSE)          (void);
	void (*Enable_SENSE)           (void);
	void (*Disable_SENSE)          (void);
	void (*Timer_Setup_Capture)    (void);
	void (*Timer_Setup_Write)      (eSignalEdge_t FirstSignalEdge);
	void (*HAL_TIM_Base_MspInit)   (TIM_HandleTypeDef* htim);
	void (*HAL_TIM_MspPostInit)    (TIM_HandleTypeDef* htim);
	void (*HAL_TIM_Base_MspDeInit) (TIM_HandleTypeDef* htim);
	void (*HAL_TIM_IC_MspDeInit)   (TIM_HandleTypeDef* htim);
	void (*HAL_TIM_OC_MspDeInit)   (TIM_HandleTypeDef* htim);
} Tape_HW;

#endif /* __BOARD_TAPE_H_ */
