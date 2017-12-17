/**
  ******************************************************************************
  * File Name          : board_tape.c
  * Description        : This file provides code for the board initialization
  *                      and de-initialization for tape specific components.
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

#include "board.h"
#include <stdio.h> // for memset()
#include <string.h> // for memset()
#include "xuac.h" // the inventory

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

// Forward declarations

static void board_tape_Init                (void);
static void board_tape_DeInit              (void);

static void board_tape_Prepare_DISCONNECT  (void);
static void board_tape_Enable_DISCONNECT   (void);
static void board_tape_Disable_DISCONNECT  (void);
static void board_tape_Prepare_MOTOR       (void);
static void board_tape_MOTOR_On            (void);
static void board_tape_MOTOR_Off           (void);
static void board_tape_Prepare_SENSE       (void);
static void board_tape_Enable_SENSE        (void);
static void board_tape_Disable_SENSE       (void);
static void board_tape_Timer_Setup_Capture (void);
static void board_tape_Timer_Setup_Write   (eSignalEdge_t FirstSignalEdge);

void HAL_TIM_Base_MspInit_Tape      (TIM_HandleTypeDef* htim);
void HAL_TIM_MspPostInit_Tape       (TIM_HandleTypeDef* htim);
void HAL_TIM_Base_MspDeInit_Tape    (TIM_HandleTypeDef* htim);
void HAL_TIM_IC_MspDeInit_Tape      (TIM_HandleTypeDef *htim);
void HAL_TIM_OC_MspDeInit_Tape      (TIM_HandleTypeDef *htim);

static void board_tape_capture__MX_TIM1_Init (void);
static void board_tape_capture__MX_TIM2_Init (void);
static void board_tape_capture__MX_TIM3_Init (void);
static void board_tape_capture__MX_TIM4_Init (void);
static void board_tape_write__MX_TIM2_Init   (eSignalEdge_t FirstSignalEdge);
static void board_tape_write__MX_TIM3_Init   (void);
static void board_tape_write__MX_TIM4_Init   (void);

// Register operations in inventory

Tape_HW Tape_HW_Ops __attribute__ ((unused)) =
{
	board_tape_Init,
	board_tape_DeInit,
	board_tape_Prepare_DISCONNECT,
	board_tape_Enable_DISCONNECT,
	board_tape_Disable_DISCONNECT,
	board_tape_Prepare_MOTOR,
	board_tape_MOTOR_On,
	board_tape_MOTOR_Off,
	board_tape_Prepare_SENSE,
	board_tape_Enable_SENSE,
	board_tape_Disable_SENSE,
	board_tape_Timer_Setup_Capture,
	board_tape_Timer_Setup_Write,
	HAL_TIM_Base_MspInit_Tape,
	HAL_TIM_MspPostInit_Tape,
	HAL_TIM_Base_MspDeInit_Tape,
	HAL_TIM_IC_MspDeInit_Tape,
	HAL_TIM_OC_MspDeInit_Tape
};


// Implementation

static void board_tape_Init(void)
{
	xuac->Board_HW->Tape->Prepare_MOTOR();
	xuac->Board_HW->Tape->Prepare_DISCONNECT();
}


static void board_tape_DeInit(void)
{
	// No de-initialization so far.
}


static void board_tape_Prepare_DISCONNECT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// Configure GPIO pin Output Level
	HAL_GPIO_WritePin(PORT_DISCON_OUT, PIN_DISCON_OUT, GPIO_PIN_RESET);

	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

	// Configure GPIO pin #1
	GPIO_InitStruct.Pin = PIN_DISCON_IN;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(PORT_DISCON_IN, &GPIO_InitStruct);

	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

	// Configure GPIO pin #2
	GPIO_InitStruct.Pin = PIN_DISCON_OUT;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PORT_DISCON_OUT, &GPIO_InitStruct);
}


static void board_tape_Enable_DISCONNECT(void)
{
	// Clear pending DISCONNECT flag.
	__HAL_GPIO_EXTI_CLEAR_IT(PIN_DISCON_IN);

	// Initialize external interrupt: DISCONNECT
	HAL_NVIC_SetPriority(IRQn_Type_DISCON_IN, 0, 0);
	HAL_NVIC_EnableIRQ(IRQn_Type_DISCON_IN);
}


static void board_tape_Disable_DISCONNECT(void)
{
	// Disable external interrupt: DISCONNECT
	HAL_NVIC_DisableIRQ(IRQn_Type_DISCON_IN);

	// Clear pending DISCONNECT flag.
	__HAL_GPIO_EXTI_CLEAR_IT(PIN_DISCON_IN);
}


static void board_tape_Prepare_MOTOR(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

	// Set MOTOR pin output level (motor off)
	HAL_GPIO_WritePin(PORT_MOTOR, PIN_MOTOR, GPIO_PIN_RESET);

	// Configure MOTOR pin
	GPIO_InitStruct.Pin = PIN_MOTOR;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PORT_MOTOR, &GPIO_InitStruct);
}


static void board_tape_MOTOR_On(void)
{
	HAL_GPIO_WritePin(PORT_MOTOR, PIN_MOTOR, GPIO_PIN_SET);
}


static void board_tape_MOTOR_Off(void)
{
	HAL_GPIO_WritePin(PORT_MOTOR, PIN_MOTOR, GPIO_PIN_RESET);
}


static void board_tape_Prepare_SENSE(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

	// Configure SENSE pin
	GPIO_InitStruct.Pin = PIN_SENSE;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PORT_SENSE, &GPIO_InitStruct);
}


static void board_tape_Enable_SENSE(void)
{
	// Clear pending SENSE flag.
	__HAL_GPIO_EXTI_CLEAR_IT(PIN_SENSE);

	// Initialize external interrupt: SENSE
	HAL_NVIC_SetPriority(IRQn_Type_SENSE, 0, 0);
	HAL_NVIC_EnableIRQ(IRQn_Type_SENSE);
}


static void board_tape_Disable_SENSE(void)
{
	// Disable external interrupt: SENSE
	HAL_NVIC_DisableIRQ(IRQn_Type_SENSE);

	// Clear pending SENSE flag.
	__HAL_GPIO_EXTI_CLEAR_IT(PIN_SENSE);
}


static void board_tape_Timer_Setup_Capture(void)
{
	// Init timer hardware for capture
	board_tape_capture__MX_TIM1_Init();
	board_tape_capture__MX_TIM2_Init();
	board_tape_capture__MX_TIM3_Init();
	board_tape_capture__MX_TIM4_Init();
}


static void board_tape_Timer_Setup_Write(eSignalEdge_t FirstSignalEdge)
{
	// Init timer hardware for writing
	board_tape_write__MX_TIM2_Init(FirstSignalEdge);
	board_tape_write__MX_TIM3_Init();
	board_tape_write__MX_TIM4_Init();
}


/* TIM1 init function (capture) */
static void board_tape_capture__MX_TIM1_Init(void)
{
	TIM_ClockConfigTypeDef         sClockSourceConfig;
	TIM_MasterConfigTypeDef        sMasterConfig;
	TIM_OC_InitTypeDef             sConfigOC;
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

	// Init timer hardware for capture

	xuac->Board_HW->data->CfgMode = tape_cfg_capture;

	memset(&htim1, 0, sizeof(TIM_HandleTypeDef));

	htim1.Instance = TIM1;
#if defined(STM32F407xx)
	htim1.Init.Prescaler = 167; // F407: 1 MHz
#elif defined(STM32F103xB)
	htim1.Init.Prescaler = 71; // F103: 1 MHz
#else
	#error "Target STM32Fxxx device not specified."
#endif
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	// Input capture stress test. Test signal generator timing:
	htim1.Init.Period = 4; // 5us
	//htim1.Init.Period = 9; // 10us
	//htim1.Init.Period = 64999; // 65ms

	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sClockSourceConfig, 0, sizeof(TIM_ClockConfigTypeDef));

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_OC_Init(&htim1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sMasterConfig, 0, sizeof(TIM_MasterConfigTypeDef));

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sConfigOC, 0, sizeof(TIM_OC_InitTypeDef));

	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCPOLARITY_LOW;
	sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_SET;
	sConfigOC.OCNIdleState = TIM_OCIDLESTATE_SET;
	if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sBreakDeadTimeConfig, 0, sizeof(TIM_BreakDeadTimeConfigTypeDef));

	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	xuac->Board_HW->Tape->HAL_TIM_MspPostInit(&htim1);

	xuac->Debug->dbg(DBGL_TAPTIM,"__TIM1_Init-C\r\n");
}


/* TIM2 init function (capture) */
static void board_tape_capture__MX_TIM2_Init(void)
{
	TIM_SlaveConfigTypeDef  sSlaveConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef      sConfigOC; // for deactivating T2-OC from tape write.

	// Init timer hardware for capture

	xuac->Board_HW->data->CfgMode = tape_cfg_capture;

	memset(&htim2, 0, sizeof(TIM_HandleTypeDef));

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 0xFFFF;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sSlaveConfig, 0, sizeof(TIM_SlaveConfigTypeDef));

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR2;
	if (HAL_TIM_SlaveConfigSynchronization(&htim2, &sSlaveConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sMasterConfig, 0, sizeof(TIM_MasterConfigTypeDef));

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	// Disable T2-OC from tape write if still activated.
	sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	/* Disable the Output compare channel */
	  TIM_CCxChannelCmd(htim2.Instance, TIM_CHANNEL_1, TIM_CCx_DISABLE);
	  // HAL_TIM_OC_Stop() would disable the timer component.
}


/* TIM3 init function (capture) */
static void board_tape_capture__MX_TIM3_Init(void)
{
	TIM_SlaveConfigTypeDef  sSlaveConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	// Init timer hardware for capture

	xuac->Board_HW->data->CfgMode = tape_cfg_capture;

	memset(&htim3, 0, sizeof(TIM_HandleTypeDef));

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 0xFFFF;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sSlaveConfig, 0, sizeof(TIM_SlaveConfigTypeDef));

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR3;
	if (HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sMasterConfig, 0, sizeof(TIM_MasterConfigTypeDef));

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}


/* TIM4 init function (capture) */
static void board_tape_capture__MX_TIM4_Init(void)
{
	TIM_ClockConfigTypeDef  sClockSourceConfig;
	TIM_SlaveConfigTypeDef  sSlaveConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_IC_InitTypeDef      sConfigIC;

	// Init timer hardware for capture

	xuac->Board_HW->data->CfgMode = tape_cfg_capture;

	memset(&htim4, 0, sizeof(TIM_HandleTypeDef));

	htim4.Instance = TIM4;
#if defined(STM32F407xx)
	htim4.Init.Prescaler = 5; // F407: 84/6=14 MHz
#elif defined(STM32F103xB)
	htim4.Init.Prescaler = 3; // F103: 72/4=18 MHz
#else
	#error "Target STM32Fxxx device not specified."
#endif
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 0xFFFF;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sClockSourceConfig, 0, sizeof(TIM_ClockConfigTypeDef));

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_IC_Init(&htim4) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sSlaveConfig, 0, sizeof(TIM_SlaveConfigTypeDef));

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
	sSlaveConfig.InputTrigger = TIM_TS_TI1F_ED;
	sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sSlaveConfig.TriggerFilter = 8;
	if (HAL_TIM_SlaveConfigSynchronization(&htim4, &sSlaveConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sMasterConfig, 0, sizeof(TIM_MasterConfigTypeDef));

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sConfigIC, 0, sizeof(TIM_IC_InitTypeDef));

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 3; // 0011b = f_{SAMPLING} = f_{CK_INT}, N=8
	if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	memset(&sConfigIC, 0, sizeof(TIM_IC_InitTypeDef));

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
	sConfigIC.ICFilter = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	xuac->Debug->dbg(DBGL_TAPTIM,"__TIM4_Init-C\r\n");
}


/* TIM2 init function (write) */
static void board_tape_write__MX_TIM2_Init(eSignalEdge_t FirstSignalEdge)
{
	TIM_SlaveConfigTypeDef  sSlaveConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef      sConfigOC;

	// Init timer hardware for writing

	xuac->Board_HW->data->CfgMode = tape_cfg_write;

	memset(&htim2, 0, sizeof(TIM_HandleTypeDef));

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 0xFFFF; // ARR -> TIM_Base_SetConfig()
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR2;
	if (HAL_TIM_SlaveConfigSynchronization(&htim2, &sSlaveConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	if (FirstSignalEdge == Unspecified)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sConfigOC.OCMode = TIM_OCMODE_FORCED_INACTIVE;
	sConfigOC.Pulse = 0xFFFF; // CCR1 -> TIM_OC1_SetConfig()
	sConfigOC.OCPolarity =
		(FirstSignalEdge == RisingEdge) ? TIM_OCPOLARITY_HIGH : TIM_OCPOLARITY_LOW;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE; // Only for PWM
	if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = 0xFFFF; // CCR1 -> TIM_OC1_SetConfig()
	sConfigOC.OCPolarity =
		(FirstSignalEdge == RisingEdge) ? TIM_OCPOLARITY_HIGH : TIM_OCPOLARITY_LOW;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE; // Only for PWM
	if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	xuac->Board_HW->Tape->HAL_TIM_MspPostInit(&htim2);
}


/* TIM3 init function (write) */
static void board_tape_write__MX_TIM3_Init(void)
{
	TIM_SlaveConfigTypeDef  sSlaveConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	// Init timer hardware for writing

	xuac->Board_HW->data->CfgMode = tape_cfg_write;

	memset(&htim3, 0, sizeof(TIM_HandleTypeDef));

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	htim3.Init.Period = 0xFFFF; // ARR -> TIM_Base_SetConfig()
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR3;
	if (HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}


/* TIM4 init function (write) */
static void board_tape_write__MX_TIM4_Init(void)
{
	TIM_ClockConfigTypeDef  sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;

	// Init timer hardware for writing

	xuac->Board_HW->data->CfgMode = tape_cfg_write;

	memset(&htim4, 0, sizeof(TIM_HandleTypeDef));

	htim4.Instance = TIM4;
#if defined(STM32F407xx)
	htim4.Init.Prescaler = 5; // F407: 84/6=14 MHz
#elif defined(STM32F103xB)
	htim4.Init.Prescaler = 3; // F103: 72/4=18 MHz
#else
	#error "Target STM32Fxxx device not specified."
#endif
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 0xFFFF; // ARR -> TIM_Base_SetConfig()
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}
}


void HAL_TIM_Base_MspInit_Tape(TIM_HandleTypeDef* htim)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if (htim->Instance == TIM1)
	{
		if (xuac->Board_HW->data->CfgMode == tape_cfg_capture)
		{
			__HAL_RCC_TIM1_CLK_ENABLE(); // Enable peripheral clock
			xuac->Debug->dbg(DBGL_TAPTIM,"__TIM1_BI\r\n");
		}
	}
	else if (htim->Instance == TIM2)
	{
		__HAL_RCC_TIM2_CLK_ENABLE(); // Enable peripheral clock

		if (xuac->Board_HW->data->CfgMode == tape_cfg_write)
		{
			// Init peripheral interrupt
			HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
			HAL_NVIC_EnableIRQ(TIM2_IRQn);
		}
	}
	else if (htim->Instance == TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE(); // Enable peripheral clock
	}
	else if (htim->Instance == TIM4)
	{
		__HAL_RCC_TIM4_CLK_ENABLE(); // Enable peripheral clock

		if (xuac->Board_HW->data->CfgMode == tape_cfg_capture)
		{
			// Configure TIM4 GPIO: PB6 -> TIM4_CH1
			GPIO_InitStruct.Pin = PIN_IC;
#if defined(STM32F103xB)
			GPIO_InitStruct.Mode = GPIO_MODE_INPUT;            // F103
#elif defined(STM32F407xx)
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;            // F407
		    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // F407
		    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;         // F407
#else
	#error "Target STM32Fxxx device not specified."
#endif
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			HAL_GPIO_Init(PORT_IC, &GPIO_InitStruct);
			xuac->Debug->dbg(DBGL_TAPTIM,"__TIM4_BI-C\r\n");
		}

		// Init peripheral interrupt
		HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM4_IRQn);
	}
}


void HAL_TIM_MspPostInit_Tape(TIM_HandleTypeDef* htim)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if (htim->Instance == TIM1)
	{
		if (xuac->Board_HW->data->CfgMode == tape_cfg_capture)
		{
			// Configure TIM1 GPIO: PA8 -> TIM1_CH1

			memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

			GPIO_InitStruct.Pin = GPIO_PIN_8;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
#if defined(STM32F103xB)
			// Nothing for F103.
#elif defined(STM32F407xx)
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM1; // F407
#else
	#error "Target STM32Fxxx device not specified."
#endif
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			xuac->Debug->dbg(DBGL_TAPTIM,"__TIM1_PI\r\n");
		}
	}
	else if (htim->Instance == TIM2)
	{
		if (xuac->Board_HW->data->CfgMode == tape_cfg_write)
		{
			// Configure TIM2 GPIO: PA15 -> TIM2_CH1

			memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

			GPIO_InitStruct.Pin = PIN_OC;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_NOPULL;
#if defined(STM32F103xB)
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;      // F103
#elif defined(STM32F407xx)
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // F407
			GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;         // F407
#else
	#error "Target STM32Fxxx device not specified."
#endif
			HAL_GPIO_Init(PORT_OC, &GPIO_InitStruct);

		    // TIM2_CH1: PA0 -> PA15
#if defined(STM32F103xB)
		    __HAL_AFIO_REMAP_TIM2_PARTIAL_1();                 // F103
#elif defined(STM32F407xx)
			// Nothing for F407.
#else
	#error "Target STM32Fxxx device not specified."
#endif

		    xuac->Debug->dbg(DBGL_TAPTIM,"__TIM2_PI-W\r\n");
		}
	}
}


void HAL_TIM_Base_MspDeInit_Tape(TIM_HandleTypeDef* htim)
{
	if (htim->Instance == TIM1)
	{
		if (xuac->Board_HW->data->CfgMode == tape_cfg_capture)
		{
			__HAL_RCC_TIM1_CLK_DISABLE(); // Disable peripheral clock
			xuac->Debug->dbg(DBGL_TAPTIM,"__MX_TIM1_DeInit1-C\r\n");
		}
	}
	else if (htim->Instance == TIM2)
	{
		__HAL_RCC_TIM2_CLK_DISABLE(); // Disable peripheral clock

		if (xuac->Board_HW->data->CfgMode == tape_cfg_write)
		{
			HAL_NVIC_DisableIRQ(TIM2_IRQn); // Disable peripheral interrupt
		}
	}
	else if (htim->Instance == TIM3)
	{
		__HAL_RCC_TIM3_CLK_DISABLE(); // Disable peripheral clock
	}
	else if (htim->Instance == TIM4)
	{
		__HAL_RCC_TIM4_CLK_DISABLE(); // Disable peripheral clock

		if (xuac->Board_HW->data->CfgMode == tape_cfg_capture)
		{
			// Configure TIM4 GPIO: PB6 -> TIM4_CH1
			HAL_GPIO_DeInit(PORT_IC, PIN_IC);
			xuac->Debug->dbg(DBGL_TAPTIM,"__TIM4_DeInit1-C\r\n");
		}

		HAL_NVIC_DisableIRQ(TIM4_IRQn); // Disable peripheral interrupt
	}
}


void HAL_TIM_IC_MspDeInit_Tape(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM4)
	{
		if (xuac->Board_HW->data->CfgMode == tape_cfg_capture)
		{
			HAL_GPIO_DeInit(PORT_IC, PIN_IC); // T4C1

			// Configure pin as: Analog, Input, Output, EVENT_OUT, EXTI

			GPIO_InitTypeDef GPIO_InitStruct;

			memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

			GPIO_InitStruct.Pin = PIN_IC;
			GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
			HAL_GPIO_Init(PORT_IC, &GPIO_InitStruct);
			xuac->Debug->dbg(DBGL_TAPTIM,"__TIM4_DeInit2-C\r\n");
		}
	}
}


// DeInit the low level hardware: GPIO, CLOCK, NVIC and DMA
void HAL_TIM_OC_MspDeInit_Tape(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1)
	{
		if (xuac->Board_HW->data->CfgMode == tape_cfg_capture)
		{
			HAL_GPIO_DeInit(GPIOA, GPIO_PIN_8);

			// Configure pin as: Analog, Input, Output, EVENT_OUT, EXTI

			GPIO_InitTypeDef GPIO_InitStruct;

			memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

			GPIO_InitStruct.Pin = GPIO_PIN_8;
			GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			xuac->Debug->dbg(DBGL_TAPTIM,"__MX_TIM1_DeInit2-C\r\n");
		}
	}
	else if (htim->Instance == TIM2)
	{
		if (xuac->Board_HW->data->CfgMode == tape_cfg_write)
		{
#if defined(STM32F103xB)
			__HAL_AFIO_REMAP_TIM2_DISABLE(); // F103
#elif defined(STM32F407xx)
			// Nothing for F4.
#else
	#error "Target STM32Fxxx device not specified."
#endif

			HAL_GPIO_DeInit(PORT_OC, PIN_OC); // T2C1

			// Configure pin as: Analog, Input, Output, EVENT_OUT, EXTI

			GPIO_InitTypeDef GPIO_InitStruct;

			memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));

			GPIO_InitStruct.Pin = PIN_OC;
			GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
			HAL_GPIO_Init(PORT_OC, &GPIO_InitStruct);
		}
	}
}
