/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "tape_timer.h"
#include "xuac.h" // the inventory

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

// Forward declarations

static void Tape_Timer_Setup_Capture (void);
static void Tape_Timer_Setup_Write   (eSignalEdge_t FirstSignalEdge);
static void Tape_Timer_Start_Capture (void);
static void Tape_Timer_Start_Write   (void);
static void Tape_Timer_Stop_Capture  (void);
static void Tape_Timer_Stop_Write    (void);

static void Tape_Timer_Reset                       (void);
static void Tape_Timer_Clear_Pending_Capture_Flags (void);
static void Tape_Timer_Clear_Pending_Write_Flags   (void);

// Register operations in inventory

_TapeTimerCtrlOps_Read TapeTimerCtrlOps_Capture =
{
	Tape_Timer_Setup_Capture,
	Tape_Timer_Start_Capture,
	Tape_Timer_Stop_Capture
};

_TapeTimerCtrlOps_Write TapeTimerCtrlOps_Write =
{
	Tape_Timer_Setup_Write,
	Tape_Timer_Start_Write,
	Tape_Timer_Stop_Write
};

_TapeTimerCtrl TapeTimerCtrl =
{
	&TapeTimerCtrlOps_Capture,
	&TapeTimerCtrlOps_Write
};

// Implementation

static void Tape_Timer_Setup_Capture(void)
{
	xuac->Board_HW->Tape->Timer_Setup_Capture();
}


static void Tape_Timer_Setup_Write(eSignalEdge_t FirstSignalEdge)
{
	xuac->Board_HW->Tape->Timer_Setup_Write(FirstSignalEdge);
}


// Start timer cascade for tape capture.
static void Tape_Timer_Start_Capture(void)
{
	// Disable peripheral interrupts
	//HAL_NVIC_DisableIRQ(TIM2_IRQn);
	//HAL_NVIC_DisableIRQ(TIM4_IRQn);

	// Reset counters
	__HAL_TIM_SET_COUNTER(&htim4, 0);
	__HAL_TIM_SET_COUNTER(&htim3, 0);
	__HAL_TIM_SET_COUNTER(&htim2, 0);

	// Clear pending flags
	Tape_Timer_Clear_Pending_Capture_Flags();

	// Start timer cascade (capture)
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);

	// Test: Timer 1 generate constant waveform on T1C1 (OC)
	//HAL_Delay(20); // for testing first timestamp
	HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_1);

	xuac->Debug->dbg(DBGL_TAPTIM,"__TIM_SC\r\n");
}


// Start timer cascade for tape write.
static void Tape_Timer_Start_Write(void)
{
	// Clear pending flags
	Tape_Timer_Clear_Pending_Write_Flags();

	// Start timer cascade (write)
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start_IT(&htim4);

	xuac->Debug->dbg(DBGL_TAPTIM,"__TIM_SW\r\n");
}


static void Tape_Timer_Stop_Capture(void)
{
	Tape_Timer_Reset();
/*
	// Reset Timer 1 (capture)
	HAL_TIM_OC_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_OC_DeInit(&htim1);
	HAL_TIM_Base_Stop(&htim1);
	HAL_TIM_Base_DeInit(&htim1);

	// Reset Timer 2 (capture)
	HAL_TIM_Base_Stop(&htim2);
	HAL_TIM_Base_DeInit(&htim2);

	// Reset Timer 3 (capture)
	HAL_TIM_Base_Stop(&htim3);
	HAL_TIM_Base_DeInit(&htim3);

	// Reset Timer 4 (capture)
	HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_2);
	HAL_TIM_IC_Stop(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop(&htim4, TIM_CHANNEL_2);
	HAL_TIM_Base_Stop_IT(&htim4);
	HAL_TIM_Base_Stop(&htim4);
	HAL_TIM_IC_DeInit(&htim4);
	HAL_TIM_Base_DeInit(&htim4);
*/
}


static void Tape_Timer_Stop_Write(void)
{
	Tape_Timer_Reset();
/*
	// Reset Timer 2 (write)
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_1);
	HAL_TIM_OC_DeInit(&htim2);
	HAL_TIM_Base_Stop_IT(&htim2);
	HAL_TIM_Base_Stop(&htim2);
	HAL_TIM_Base_DeInit(&htim2);

	// Reset Timer 3 (write)
	HAL_TIM_Base_Stop(&htim3);
	HAL_TIM_Base_DeInit(&htim3);

	// Reset Timer 4 (write)
	HAL_TIM_Base_Stop_IT(&htim4);
	HAL_TIM_Base_Stop(&htim4);
	HAL_TIM_Base_DeInit(&htim4);
*/
}

static void Tape_Timer_Reset(void)
{
	// Reset Timer 2
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_1);
	HAL_TIM_OC_DeInit(&htim2);
	HAL_TIM_Base_Stop_IT(&htim2);
	HAL_TIM_Base_Stop(&htim2);
	HAL_TIM_Base_DeInit(&htim2);

	// Reset Timer 3
	HAL_TIM_Base_Stop(&htim3);
	HAL_TIM_Base_DeInit(&htim3);

	// Reset Timer 4
	HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_2);
	HAL_TIM_IC_Stop(&htim4, TIM_CHANNEL_1);
	HAL_TIM_IC_Stop(&htim4, TIM_CHANNEL_2);
	HAL_TIM_Base_Stop_IT(&htim4);
	HAL_TIM_Base_Stop(&htim4);
	HAL_TIM_IC_DeInit(&htim4);
	HAL_TIM_Base_DeInit(&htim4);

	// Clear pending flags
	Tape_Timer_Clear_Pending_Capture_Flags();
	Tape_Timer_Clear_Pending_Write_Flags();
}


static void Tape_Timer_Clear_Pending_Capture_Flags(void)
{
	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_CC1);
	__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_CC2);
	__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);
	__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_CC1);
	__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_CC2);
}


static void Tape_Timer_Clear_Pending_Write_Flags(void)
{
	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
	__HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
	__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);
	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_CC1);
	__HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
}
