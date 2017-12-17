/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "status_led.h"
#include "xuac.h"

#include <stdio.h> // for memset()
#include <string.h> // for memset()

// Forward declarations

static void StatusLED_Init        (StatusLED_Def *LED);
static void StatusLED_LED_On      (StatusLED_Def *LED);
static void StatusLED_LED_Off     (StatusLED_Def *LED);
static void StatusLED_LED_Toggle  (StatusLED_Def *LED, uint32_t times, uint32_t delay_ms);
#if defined(STM32_F4VE_V2_0_1509)
static void StatusLED_LED_Toggle2 (struct _StatusLED_Def *LED, struct _StatusLED_Def *LED2, uint32_t times, uint32_t delay_ms);
#endif

// Register operations in inventory

StatusLED_Def StatusLED =
{
	PORT_USER_LED,
	PIN_USER_LED
};

#if defined(STM32_F4VE_V2_0_1509)
StatusLED_Def StatusLED2 =
{
	PORT_USER_LED2,
	PIN_USER_LED2
};
#endif

StatusLED_List StatusLEDs =
{
	&StatusLED
#if defined(STM32_F4VE_V2_0_1509)
	, &StatusLED2
#endif
};

StatusLED_Operations StatusLED_Ops =
{
	&StatusLEDs,
	StatusLED_Init,
	StatusLED_LED_On,
	StatusLED_LED_Off,
	StatusLED_LED_Toggle,
#if defined(STM32_F4VE_V2_0_1509)
	StatusLED_LED_Toggle2
#endif
};

// Implementation

// Configure status LED
// Cannot init two LEDs at the same time if located on different ports.
static void StatusLED_Init(StatusLED_Def *LED)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// Configure GPIO pin Output Level
	HAL_GPIO_WritePin(LED->PORT, LED->PIN, GPIO_PIN_SET);

	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

	// Configure GPIO pin
	GPIO_InitStruct.Pin = LED->PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED->PORT, &GPIO_InitStruct);
}

// Cannot turn on two LEDs at the same time if located on different ports.
static void StatusLED_LED_On(StatusLED_Def *LED)
{
	HAL_GPIO_WritePin(LED->PORT, LED->PIN, GPIO_PIN_RESET);
}


// Cannot turn off two LEDs at the same time if located on different ports.
static void StatusLED_LED_Off(StatusLED_Def *LED)
{
	HAL_GPIO_WritePin(LED->PORT, LED->PIN, GPIO_PIN_SET);
}


// Cannot toggle two LEDs at the same time if located on different ports.
static void StatusLED_LED_Toggle(StatusLED_Def *LED, uint32_t times, uint32_t delay_ms)
{
	for (uint32_t i = 0; i < 2*times; i++)
	{
		HAL_GPIO_TogglePin(LED->PORT, LED->PIN);
		HAL_Delay(delay_ms);
	}
}


#if defined(STM32_F4VE_V2_0_1509)
static void StatusLED_LED_Toggle2(struct _StatusLED_Def *LED, struct _StatusLED_Def *LED2, uint32_t times, uint32_t delay_ms)
{
	for (uint32_t i = 0; i < 2*times; i++)
	{
		HAL_GPIO_TogglePin(LED->PORT, LED->PIN);
		HAL_GPIO_TogglePin(LED2->PORT, LED2->PIN);
		HAL_Delay(delay_ms);
	}
}
#endif
