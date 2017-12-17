/**
  ******************************************************************************
  * File Name          : board.c
  * Description        : This file provides code for the board initialization
  *                      and de-initialization.
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#include "system_clock.h" // for SystemClock_Config()
#include "gpio.h"         // for MX_GPIO_Init()
#include "usart.h"        // for MX_USART_UART_Init()
#include "usb_device.h"   // for MX_USB_DEVICE_Init()

#include "board.h"
#include "board_defs.h"
#include "xuac.h" // the inventory

// Forward declarations

static void Board_Init          (void);
static void Board_Send_Dbg_Info (void);

// Register data in inventory

Board_HW_Info The_Board_HW_Info =
{
	PLATFORM_NAME,          /* (uint8_t *) Platform_Name                */
	BOARD_FIRMWARE_VERSION, /* (uint8_t *) Firmware_Version             */
	BOARD_FIRMWARE_MAJ_STR, /* (uint8_t *) Firmware_Version_Major_Str   */
	BOARD_FIRMWARE_MAJ_VAL, /* (uint32_t)  Firmware_Version_Major_Value */
	BOARD_FIRMWARE_MIN_STR, /* (uint8_t *) Firmware_Version_Minor_Str   */
	BOARD_FIRMWARE_MIN_VAL, /* (uint32_t)  Firmware_Version_Minor_Value */
	BOARD_FIRMWARE_SUB_STR, /* (uint8_t *) Firmware_Version_Sub_Str     */
	BOARD_FIRMWARE_SUB_VAL, /* (uint32_t)  Firmware_Version_Sub_Value   */
	BOARD_NAME,             /* (uint8_t *) Board_Name                   */
	MCU_NAME,               /* (uint8_t *) MCU_Name                     */
	MCU_MHZ_STR,            /* (uint8_t *) MCU_MHZ_Str                  */
	MCU_MHZ_VAL,            /* (uint32_t)  MCU_MHZ_Value                */
	TIMER_SPEED_MHZ_STR,    /* (uint8_t *) Timer_Speed_MHZ_Str          */
	TIMER_SPEED_MHZ_VAL,    /* (uint32_t)  Timer_Speed_MHZ_Value        */
	BUFFER_SIZE_STR,        /* (uint8_t *) Buffer_Size_Str              */
	BUFFER_SIZE_VAL,        /* (uint32_t)  Buffer_Size_Value            */
	BUFFER_SIZE_VAL_STR,    /* (uint8_t *) Buffer_Size_Value_Str        */
	SAMPLING_RATE,          /* (uint8_t *) Sampling_Rate                */
	BOARD_CAPABILITIES      /* (uint32_t)  Board_Capabilities           */
};

Board_HW_Data The_Board_HW_Data =
{
	cfg_nothing             /* enum ECfgMode CfgMode */
};

extern Tape_HW Tape_HW_Ops;
extern uint16_t Board_Info(uint32_t InfoRequest);

Board_HW_Operations Board_HW_Ops =
{
	&The_Board_HW_Info,
	&The_Board_HW_Data,
	Board_Init,
	Board_Send_Dbg_Info,
	Board_Info,
	&Tape_HW_Ops
};

// Implementation

static void Board_Init(void)
{
	// Reset all peripherals, initialize Flash interface and Systick.
	HAL_Init();

	// Configure system clock.
	SystemClock_Config();

	// Initialize used peripherals.
	MX_GPIO_Init();
	MX_USART_UART_Init();
	MX_USB_DEVICE_Init();

	// Initialize and blink status LED(s) a few times: Hello world.

	// Initialize status LED(s).
	xuac->StatusLED_Ops->LED_Init(xuac->StatusLED_Ops->StatusLEDs->LED);

	// Turn on status LED.
	xuac->StatusLED_Ops->LED_On(xuac->StatusLED_Ops->StatusLEDs->LED);

#if defined(STM32_F4VE_V2_0_1509)
	xuac->StatusLED_Ops->LED_Init(xuac->StatusLED_Ops->StatusLEDs->LED2);
	xuac->StatusLED_Ops->LED_Off(xuac->StatusLED_Ops->StatusLEDs->LED2);
	xuac->StatusLED_Ops->LED_Toggle2(xuac->StatusLED_Ops->StatusLEDs->LED, xuac->StatusLED_Ops->StatusLEDs->LED2, 5, 100);
#else
	xuac->StatusLED_Ops->LED_Toggle(xuac->StatusLED_Ops->StatusLEDs->LED, 5, 100);
#endif
}

// Send board info on debug channel.
static void Board_Send_Dbg_Info(void)
{
/*	uint8_t addr;
	xuac->Debug->dbg(DBGL_MAIN,"%s, %s, [0x%x]\r\n",
		PLATFORM_NAME, BOARD_FIRMWARE_VERSION, &addr);*/

	xuac->Debug->dbg(DBGL_MAIN,"--------------------------------------------\r\n");
	HAL_Delay(3);
	xuac->Debug->dbg(DBGL_MAIN,"Platform: %s\r\n", PLATFORM_NAME);
	HAL_Delay(3);
	xuac->Debug->dbg(DBGL_MAIN,"Board: %s\r\n", BOARD_NAME);
	HAL_Delay(3);
	xuac->Debug->dbg(DBGL_MAIN,"MCU: %s @ %s MHz\r\n", MCU_NAME, MCU_MHZ_STR);
	HAL_Delay(3);
	xuac->Debug->dbg(DBGL_MAIN,"Firmware: %s\r\n", BOARD_FIRMWARE_VERSION);
	HAL_Delay(3);
	xuac->Debug->dbg(DBGL_MAIN,"Buffer: %s\r\n", BUFFER_SIZE_STR);
	HAL_Delay(3);
	xuac->Debug->dbg(DBGL_MAIN,"Timer: %s MHz\r\n", TIMER_SPEED_MHZ_STR);
	HAL_Delay(3);
	xuac->Debug->dbg(DBGL_MAIN,"STM: %s\r\n", STM_FW);
	HAL_Delay(3);
	xuac->Debug->dbg(DBGL_MAIN,"--------------------------------------------\r\n");
	HAL_Delay(3);
}
