/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __BOARD_DEFS_BLACKPILL_H
#define __BOARD_DEFS_BLACKPILL_H

#include "board_caps.h" // for BOARD_CAPABILITIES

// --- Board info definitions -------------------------------------------------

#define BOARD_NAME             (uint8_t *) "Black Pill"
#define MCU_NAME               (uint8_t *) "STM32F103C8T6 (ARM Cortex-M3)"

#define PLATFORM_NAME          (uint8_t *) "ZoomTape"
#define MCU_MHZ_STR            (uint8_t *) "72"
#define MCU_MHZ_VAL            (uint32_t )  72
#define TIMER_SPEED_MHZ_STR    (uint8_t *) "18"
#define TIMER_SPEED_MHZ_VAL    (uint32_t )  18
#define BUFFER_SIZE_STR        (uint8_t *) "8 KByte"
#define BUFFER_SIZE_VAL        (uint32_t )  8192 // Bytes
#define BUFFER_SIZE_VAL_STR    (uint8_t *) "8192" // Bytes
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
#define PIN_MOTOR            GPIO_PIN_13
#define PORT_MOTOR           GPIOB

// SENSE
#define PIN_SENSE            GPIO_PIN_4
#define PORT_SENSE           GPIOB
#define IRQn_Type_SENSE      EXTI4_IRQn
// Note: Check presence of EXTI4_IRQHandler() in stm32f1xx_it.c

// DISCONNECT
#define PIN_DISCON_IN        GPIO_PIN_14
#define PIN_DISCON_OUT       GPIO_PIN_15
#define PORT_DISCON_IN       GPIOB
#define PORT_DISCON_OUT      GPIOB
#define IRQn_Type_DISCON_IN  EXTI15_10_IRQn
// Note: Check presence of EXTI15_10_IRQHandler() in stm32f1xx_it.c

// --- Board I/O definitions: Status LED pin ----------------------------------

#define PIN_USER_LED         GPIO_PIN_12
#define PORT_USER_LED        GPIOB

// --- Board I/O definitions: Flash & debug pins ------------------------------

// PA9  ------> USART1_TX
// PA10 ------> USART1_RX

// --- Board I/O definitions: Signal generator pin ----------------------------

// Signal generator for input capture test.
// Connect to PORT_IC/PIN_IC for test.
//
// PA8  ------> TIM1_CH1

// --- Board I/O definitions: Other pins --------------------------------------

// Onboard button is wired to /RST.

// ----------------------------------------------------------------------------

#endif /* __BOARD_DEFS_BLACKPILL_H */
