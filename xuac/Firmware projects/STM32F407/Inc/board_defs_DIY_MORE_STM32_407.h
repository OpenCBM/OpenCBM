/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __BOARD_DEFS_DIY_MORE_STM32_407_H
#define __BOARD_DEFS_DIY_MORE_STM32_407_H

#include "board_caps.h" // for BOARD_CAPABILITIES

// --- Board info definitions -------------------------------------------------

#define BOARD_NAME             (uint8_t *) "DIY MORE STM32-407"
#define MCU_NAME               (uint8_t *) "STM32F407VGT6 (ARM Cortex-M4)"

#define PLATFORM_NAME          (uint8_t *) "ZoomTape"
#define MCU_MHZ_STR            (uint8_t *) "168"
#define MCU_MHZ_VAL            (uint32_t )  168
#define TIMER_SPEED_MHZ_STR    (uint8_t *) "14"
#define TIMER_SPEED_MHZ_VAL    (uint32_t )  14
#define BUFFER_SIZE_STR        (uint8_t *) "100 KByte"
#define BUFFER_SIZE_VAL        (uint32_t )  102400 // Bytes
#define BUFFER_SIZE_VAL_STR    (uint8_t *) "102400" // Bytes
#define SAMPLING_RATE          (uint8_t *) "up to 20KS/s (20,000 samples per second)"
#define BOARD_CAPABILITIES     (uint32_t ) BOARD_CAPABILITY_TAPE

// --- Board I/O definitions: Tape pins ---------------------------------------

// Note: All tape pins are 5V tolerant.

// READ
#define PIN_IC               GPIO_PIN_6 // T4C1
#define PORT_IC              GPIOB

// WRITE
#define PIN_OC               GPIO_PIN_15 // T2C1 remapped
#define PORT_OC              GPIOA

// MOTOR
#define PIN_MOTOR            GPIO_PIN_7
#define PORT_MOTOR           GPIOD

// SENSE
#define PIN_SENSE            GPIO_PIN_3
#define PORT_SENSE           GPIOD
#define IRQn_Type_SENSE      EXTI3_IRQn
// Note: Check presence of EXTI3_IRQHandler() in stm32f4xx_it.c

// DISCONNECT
#define PIN_DISCON_IN        GPIO_PIN_12
#define PIN_DISCON_OUT       GPIO_PIN_11
#define PORT_DISCON_IN       GPIOC
#define PORT_DISCON_OUT      GPIOC
#define IRQn_Type_DISCON_IN  EXTI15_10_IRQn
// Note: Check presence of EXTI15_10_IRQHandler() in stm32f4xx_it.c

// --- Board I/O definitions: Status LED pins ---------------------------------

#define PIN_USER_LED         GPIO_PIN_0
#define PORT_USER_LED        GPIOE

// --- Board I/O definitions: Flash & debug pins ------------------------------

// PA9  ------> USART1_TX
// PA10 ------> USART1_RX

// --- Board I/O definitions: Signal generator pin ----------------------------

// Signal generator for input capture test.
// Connect to PORT_IC/PIN_IC for test.
//
// PA8  ------> TIM1_CH1

// --- Board I/O definitions: Other pins --------------------------------------

// Onboard USB OTG FS wired to:
// PA11 ------> USB_OTG_FS_DM
// PA12 ------> USB_OTG_FS_DP

// Onboard user button wired to:
// PD15 ------> K2 (located next to USB connector)
//
// (Button K1 wired to /RST, located at 'dimroe' text label
//  far away from USB connector.)

// BOOT1 wired to:
// PB2 ------> BOOT1

// ----------------------------------------------------------------------------

#endif /* __BOARD_DEFS_DIY_MORE_STM32_407_H */
