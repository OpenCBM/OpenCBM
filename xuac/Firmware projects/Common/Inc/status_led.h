/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __STATUS_LED_H
#define __STATUS_LED_H

#include "fw.h" // STM FW
#include "board_defs.h"

 // Structure for the inventory

typedef struct _StatusLED_Def
{
	GPIO_TypeDef *PORT;
	uint16_t     PIN;
} StatusLED_Def;

typedef struct
{
	StatusLED_Def *LED;
#if defined(STM32_F4VE_V2_0_1509)
	StatusLED_Def *LED2;
#endif
} StatusLED_List;

typedef struct
{
	StatusLED_List *StatusLEDs;
	void (*LED_Init)    (struct _StatusLED_Def *LED);
	void (*LED_On)      (struct _StatusLED_Def *LED);
	void (*LED_Off)     (struct _StatusLED_Def *LED);
	void (*LED_Toggle)  (struct _StatusLED_Def *LED, uint32_t times, uint32_t delay_ms);
#if defined(STM32_F4VE_V2_0_1509)
	void (*LED_Toggle2) (struct _StatusLED_Def *LED, struct _StatusLED_Def *LED2, uint32_t times, uint32_t delay_ms);
#endif
} StatusLED_Operations;

#endif /* __STATUS_LED_H */
