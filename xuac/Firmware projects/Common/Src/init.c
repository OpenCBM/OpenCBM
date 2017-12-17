/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "xuac.h" // the inventory

// Initialize the platform.
void Init(void)
{
	xuac->Board_HW->Init();
	xuac->Board_HW->Tape->Init();
	xuac->Watchdog->Setup();

#if defined(STM32F407xx)
	HAL_NVIC_SetPriority(OTG_FS_IRQn, 1, 0);
#elif defined(STM32F103xB)
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 1, 0); // =USB_LP_CAN1_RX0_IRQn
#else
	#error "Target STM32Fxxx device not specified."
#endif

	// Send board info on debug channel.
	xuac->Board_HW->Send_Board_Info_Dbg();
}
