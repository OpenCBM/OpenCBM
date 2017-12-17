/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include <stdio.h> // for vsnprintf()
#include <stdarg.h> // for va_list

#include "dbg.h"
#include "xuac.h" // the inventory

extern UART_HandleTypeDef huart;

// Forward declarations

static void dbg_uart(uint8_t level, const char *fmt, ...);

// Register operations in inventory

Debug_Operations DebugOps =
{
	DBGL_STD,
	dbg_uart
};

// Implementation

static void dbg_uart(uint8_t level, const char *fmt, ...)
{
	char buffer[DBG_MAX_LEN];
	va_list args;

	if (level <= xuac->Debug->dbg_level)
	{
		va_start(args, fmt);
		int len = vsnprintf(buffer, DBG_MAX_LEN, fmt, args);
		HAL_UART_Transmit(&huart, (uint8_t*)buffer, len, 1000);
		//while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY) { }
		va_end(args);
	}
}
