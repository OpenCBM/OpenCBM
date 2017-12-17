/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "error_handler.h"
#include "xuac.h" // the inventory

// Forward declarations

static void ErrorHandler(uint32_t ErrorNum, char *file, int line);
static void ErrorHandlerExt(uint32_t ErrorNum);

// Register operations in inventory

ErrorHandler_Operations ErrorHandlerOps =
{
	ErrorHandler
};

// Implementation

static void ErrorHandler(uint32_t ErrorNum, char *file, int line)
{
	// Send error message on debug channel.
	xuac->Debug->dbg(DBGL_ERR,"Error occurred (#%d). File: %s, Line: %d.\r\n", ErrorNum, file, line);

	ErrorHandlerExt(ErrorNum);
}

// Exported operation. Called by STM firmware.
void _Error_Handler(char *file, int line)
{
	// Send error message on debug channel.
	xuac->Debug->dbg(DBGL_ERR,"Error occurred. File: %s, Line: %d.\r\n", file, line);

	ErrorHandlerExt(3);
}

static void ErrorHandlerExt(uint32_t ErrorNum)
{
	HAL_GPIO_WritePin(PORT_USER_LED, PIN_USER_LED, GPIO_PIN_SET);
//	HAL_GPIO_WritePin(PORT_USER_LED, PIN_USER_LED2, GPIO_PIN_RESET);

	while (1)
	{
		// Blink user LED 'ErrorNum' times.
		for (uint32_t i = 0; i < ErrorNum*2; i++)
		{
			HAL_GPIO_TogglePin(PORT_USER_LED, PIN_USER_LED/*|PIN_USER_LED2*/);
			HAL_Delay(200);
		}

		HAL_Delay(2000);
	}
}
