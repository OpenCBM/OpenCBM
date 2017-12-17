/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

// Note: This is early prototype code, alpha test version.
// ToDo: May need to rethink/optimize the code.

#include "stdbool.h"    // for boolean type

#include "usbd_ioreq.h" // for USBD_LL_StallEP()
#include "usbd_def.h"   // for USB_FS_MAX_PACKET_SIZE
#include "usbd_ioreq.h" // for USBD_HandleTypeDef
#include "usbd_cdc.h"   // for CDC_OUT_EP

#include "tape_153x.h"
#include "tape.h" // for eSignalEdge_t
#include "status.h" // USB bulk request status values
#include "xuac.h" // the inventory

// CBM 153x tape operations states for TSR (Tape State Register)
#define XUAC_TAP_DEVICE_CONFIG_READ  0x01 // tape configured for read (1 = true)
#define XUAC_TAP_DEVICE_CONFIG_WRITE 0x02 // tape configured for write (1 = true)
#define XUAC_TAP_CAPTURING           0x04 // tape read in progress  (1 = capture, 0 = stop capture)
#define XUAC_TAP_WRITING             0x08 // tape write in progress (1 = writing, 0 = stop write)
#define XUAC_TAP_MOTOR               0x10 // last tape MOTOR CONTROL setting remains active (1 = active)
#define XUAC_TAP_DISCONNECTED        0x80 // tape device was disconnected (1 = true)

#define TAPE_CONFIG_OPTION_BASIC      1 // Basic configuration is restored, motor off.
#define TAPE_CONFIG_OPTION_KEEP_MOTOR 2 // Basic configuration is restored, last tape MOTOR CONTROL setting remains active

// Tape firmware version
#define TapeFirmwareVersion 0x0001

// Tape hardware specific defaults
#define TAPE_SENSE_DELAY_MS    (uint32_t ) 10 // Default SENSE delay: 10 ms

// External variables

// For USBD_LL_StallEP() if tape write stops early.
extern USBD_HandleTypeDef hUsbDeviceFS;

// Global variables

// Quick references to the buffers for send & receive bulk data.
static UserBuffer *RxBuffer;
static UserBuffer *TxBuffer;

// Tape State Register: Current state of tape operations.
static volatile uint8_t TSR = 0;

// First signal edge (detected on READ, used on WRITE).
static volatile eSignalEdge_t FirstSignalEdge = Unspecified;

// Next expected signal edge while READing (for detecting lost edges).
static volatile eSignalEdge_t NextSignalEdge = Unspecified;

// Flag for BREAKing out of Tape_WaitForStopSense().
static volatile bool StopWaitForSense = false;
static volatile bool BreakAllLoops = false;

// Global status of tape operations.
static volatile uint16_t TapeStatus = Tape_Status_OK;

// Global 40bit timestamp from/for timer cascade.
static volatile uint32_t MSB2;
static volatile uint32_t MSB;
static volatile uint32_t LSB;

// Debug variable: Count the number of loaded/inserted timestamps.
static volatile uint32_t dbg_load_cnt = 0;

// Debug variable: Count the number of output compare events/timestamps.
static volatile uint32_t dbg_ti2cnt = 0;

// Forward declarations

static uint16_t Tape_GetFirmwareVersion         (void);
static uint16_t Tape_MotorOn                    (void);
static uint16_t Tape_MotorOff                   (void);
static uint16_t Tape_PrepareCapture             (void);
static uint16_t Tape_PrepareWrite               (void);
static uint16_t Tape_GetSense                   (void);
static uint16_t Tape_WaitForStopSense           (void);
static uint16_t Tape_WaitForPlaySense           (void);
static uint16_t Tape_isDeviceConnected          (void);
static uint16_t Tape_StartCapture               (void);
static uint16_t Tape_StartWrite                 (void);
static uint16_t Tape_Capture                    (void);
static uint16_t Tape_Write                      (uint32_t DataCount);
static void     Tape_StopCapture                (void);
static void     Tape_StopWrite                  (void);
static uint8_t  Tape_Reset                      (void);

static void     Tape_Wait_Tx                    (void);
static uint16_t Tape_TimestampFromReceiveBuffer (void);
static void     Tape_TimestampIntoSendBuffer    (void);
static uint16_t Tape_TimerCascade_Load_and_Insert_Timestamp(void);

static void     dbgout                          (uint32_t id);

// Register operations in inventory

extern _TapeTimerCtrl TapeTimerCtrl;

TapeData TheTapeData =
{
	TAPE_SENSE_DELAY_MS, /* uint32_t Sense_Delay_ms     */
	0,                   /* uint8_t  First_Write_Edge   */
	0,                   /* uint8_t  First_Capture_Edge */
	0,                   /* uint32_t Lost_Count         */
	0,                   /* uint32_t Discard_Count      */
	0                    /* uint32_t Overcapture_Count  */
};

Tape_Operations TapeOps_153x =
{
	&TheTapeData,
	Tape_GetFirmwareVersion,
	Tape_MotorOn,
	Tape_MotorOff,
	Tape_PrepareCapture,
	Tape_PrepareWrite,
	Tape_GetSense,
	Tape_WaitForStopSense,
	Tape_WaitForPlaySense,
	Tape_Capture,
	Tape_Write,
	Tape_Reset,
	&TapeTimerCtrl
};

// Implementation

/**
  * @brief  Tape_GetFirmwareVersion
  *         Returns the tape firmware version. Different versions may
  *         support different features.
  *
  * @retval The tape firmware version.
  */
static uint16_t Tape_GetFirmwareVersion(void)
{
	return TapeFirmwareVersion;
}


/**
  * @brief  Tape_isDeviceConnected
  *         Checks if the ZoomTape device is connected.
  *
  * @retval Tape_Status_OK
  *           - ZoomTape device is connected.
  *         Tape_Status_ERROR_Device_Disconnected
  *           - ZoomTape device is disconnected.
  */
static uint16_t Tape_isDeviceConnected(void)
{
	GPIO_PinState PinState = HAL_GPIO_ReadPin(PORT_DISCON_IN, PIN_DISCON_IN);

	// GPIO_PIN_SET   = 1 = Tape_Status_ERROR_Device_Disconnected
	// GPIO_PIN_RESET = 0 = Tape_Status_OK

	if (PinState == GPIO_PIN_SET)
		return Tape_Status_ERROR_Device_Disconnected;

	return Tape_Status_OK;
}


/**
  * @brief  Tape_isDeviceConfigured
  *         Checks the flags in the tape state register (TSR) and returns
  *         if ZoomTape device is configured for tape capture or write.
  *
  * @retval Tape_Status_OK_Device_Configured_for_Read
  *           - ZoomTape device is configured for tape capture
  *             (Tape_PrepareCapture() was called).
  *         Tape_Status_OK_Device_Configured_for_Write
  *           - ZoomTape device is configured for tape write
  *             (Tape_PrepareWrite() was called).
  */
static uint16_t Tape_isDeviceConfigured(void)
{
	if (TSR & XUAC_TAP_DEVICE_CONFIG_READ)
		return Tape_Status_OK_Device_Configured_for_Read;

	if (TSR & XUAC_TAP_DEVICE_CONFIG_WRITE)
		return Tape_Status_OK_Device_Configured_for_Write;

	return Tape_Status_ERROR_Device_Not_Configured;
}


/**
  * @brief  Tape_ClearTapeFlags
  *
  *         Reset device configuration flags set by
  *         - Tape_PrepareCapture(),
  *         - Tape_PrepareWrite().
  *
  *         Reset tape capture/write in progress flags set by
  *         - Tape_StartCapture(),
  *         - Tape_StartWrite().
  *
  *         Reset WaitForSense stop flag set by
  *         - Tape_Reset().
  *
  * @retval -
  */
static void Tape_ClearTapeFlags(void)
{
	// Clear device configuration flags.
	TSR &= (uint8_t)~XUAC_TAP_DEVICE_CONFIG_READ;
	TSR &= (uint8_t)~XUAC_TAP_DEVICE_CONFIG_WRITE;

	// Clear tape capture/write in progress flags.
	TSR &= (uint8_t)~XUAC_TAP_CAPTURING;
	TSR &= (uint8_t)~XUAC_TAP_WRITING;

	// Clear WaitForSense stop flag.
	StopWaitForSense = false;
}


/**
  * @brief  Tape_MotorOn
  *         Turn on the tape drive motor.
  *
  * @retval Tape_Status_OK_Motor_On
  *           - Motor turned on successfully.
  *         Tape_Status_ERROR_Device_Disconnected
  *           - Motor cannot be turned on, because the ZoomTape device
  *             is not connected.
  */
static uint16_t Tape_MotorOn(void)
{
	if (Tape_isDeviceConnected() == Tape_Status_ERROR_Device_Disconnected)
		return Tape_Status_ERROR_Device_Disconnected;

	// Set MOTOR CONTROL flag.
	TSR |= XUAC_TAP_MOTOR;

	// Turn motor on.
	xuac->Board_HW->Tape->MOTOR_On();

	return Tape_Status_OK_Motor_On;
}


/**
  * @brief  Tape_MotorOff
  *         Turn off the tape drive motor.
  *
  * @retval Tape_Status_OK_Motor_Off
  *           - Motor turned off successfully.
  *         Tape_Status_ERROR_Device_Disconnected
  *           - Motor cannot be turned off, because the ZoomTape device
  *             is not connected.
  */
static uint16_t Tape_MotorOff(void)
{
	if (Tape_isDeviceConnected() == Tape_Status_ERROR_Device_Disconnected)
		return Tape_Status_ERROR_Device_Disconnected;

	// Set MOTOR CONTROL flag.
	TSR |= XUAC_TAP_MOTOR;

	// Turn motor off.
	xuac->Board_HW->Tape->MOTOR_Off();

	return Tape_Status_OK_Motor_Off;
}


/**
  * @brief  HAL_GPIO_EXTI_Callback
  *
  *         Pin change interrupt: SENSE
  *         Stop tape capture/write if user presses <STOP>.
  *
  *         Pin change interrupt: device disconnect.
  *         Stop tape operations if user disconnects tape hardware.
  *
  *         In either case the TapeStatus flag is set accordingly:
  *         - Tape_Status_ERROR_Write_Interrupted_By_Stop
  *         - Tape_Status_ERROR_Device_Disconnected
  *
  * @param  GPIO_Pin: The I/O pin on which the pin change interrupt
  *                   occurred (PIN_SENSE or PIN_DISCON_IN).
  *
  * @retval -
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == PIN_SENSE)
	{
		// User pressed <STOP>.

		if (TSR & XUAC_TAP_CAPTURING)
		{
			xuac->Debug->dbg(DBGL_TAPE,"HAL_GPIO_EXTI_Callback(): SENSE - Tape_StopCapture()\r\n");

			Tape_StopCapture();
		}
		else if (TSR & XUAC_TAP_WRITING)
		{
			xuac->Debug->dbg(DBGL_TAPE,"HAL_GPIO_EXTI_Callback(): SENSE - Tape_StopWrite()\r\n");

			Tape_StopWrite();
			//USBD_LL_StallEP(&hUsbDeviceFS, 0x04);
			TapeStatus = Tape_Status_ERROR_Write_Interrupted_By_Stop;
		}
		else
		{
			xuac->Debug->dbg(DBGL_TAPE,"HAL_GPIO_EXTI_Callback(): SENSE - no read/write\r\n");
		}
	}
	else if (GPIO_Pin == PIN_DISCON_IN)
	{
		// Device was disconnected.

		TapeStatus = Tape_Status_ERROR_Device_Disconnected;

		if (TSR & XUAC_TAP_CAPTURING)
		{
			xuac->Debug->dbg(DBGL_TAPE,"HAL_GPIO_EXTI_Callback(): Disconnect\r\n");

			Tape_StopCapture();
		}
		else if (TSR & XUAC_TAP_WRITING)
		{
			xuac->Debug->dbg(DBGL_TAPE,"HAL_GPIO_EXTI_Callback(): Disconnect\r\n");

			Tape_StopWrite();
			//USBD_LL_StallEP(&hUsbDeviceFS, 0x04);
		}
		else
		{
			xuac->Debug->dbg(DBGL_TAPE,"HAL_GPIO_EXTI_Callback(): Disconnect - no read/write\r\n");
		}
	}
	else
	{
		xuac->Debug->dbg(DBGL_TAPE,"HAL_GPIO_EXTI_Callback(): GPIO_PIN_?\r\n");
	}
}


/**
  * @brief  Tape_PrepareCapture
  *         Init the platform for tape capture, configure the hardware.
  *         Set flag in the tape state register (TSR) if successful:
  *         XUAC_TAP_DEVICE_CONFIG_READ.
  *
  * @retval Tape_Status_OK_Device_Configured_for_Read
  *           - Initialization was successful.
  *         Tape_Status_ERROR_Device_Disconnected
  *           - The tape device was disconnected. Check cabling.
  */
static uint16_t Tape_PrepareCapture(void)
{
	// Init global TapeStatus flag.
	TapeStatus = Tape_Status_OK;

	// Reset device configuration flags,
	// reset tape capture/write in progress flags,
	// reset WaitForSense stop flag.
	Tape_ClearTapeFlags();

	// Turn motor on. Also check device disconnect status.
	if (Tape_MotorOn() == Tape_Status_ERROR_Device_Disconnected)
		return Tape_Status_ERROR_Device_Disconnected;

	// Prepare for input pin change: SENSE
	xuac->Board_HW->Tape->Prepare_SENSE();

	// Prepare input capture timer cascade.
	xuac->TapeOps->TimerCtrl->Capture->Setup();

	// First signal edge will be automatically detected, hence the
	// expected polarity of next following signal edge is not yet known.
	FirstSignalEdge = Unspecified;
	NextSignalEdge = Unspecified;

	// Reset lost signal count.
	xuac->TapeOps->data->Lost_Signal_Edges_Count = 0;

	// Reset discarded signals (full send buffer).
	xuac->TapeOps->data->Discarded_Signal_Count = 0;

	// Reset overcapture count.
	xuac->TapeOps->data->Overcapture_Count = 0;

	// Device is now configured for tape read operations.
	TSR |= (uint8_t)XUAC_TAP_DEVICE_CONFIG_READ;

	return Tape_Status_OK_Device_Configured_for_Read;
}


/**
  * @brief  Tape_PrepareWrite
  *         Init the platform for tape write, configure the hardware.
  *         Set flag in the tape state register (TSR) if successful:
  *         XUAC_TAP_DEVICE_CONFIG_WRITE.
  *
  * @retval Tape_Status_OK_Device_Configured_for_Read
  *           - Initialization was successful.
  *         Tape_Status_ERROR_Device_Disconnected
  *           - The tape device was disconnected. Check cabling.
  */
static uint16_t Tape_PrepareWrite(void)
{
	// Init global TapeStatus flag.
	TapeStatus = Tape_Status_OK;

	// Reset device configuration flags,
	// reset tape capture/write in progress flags,
	// reset WaitForSense stop flag.
	Tape_ClearTapeFlags();

	// Turn motor on. Also check device disconnect status.
	if (Tape_MotorOn() == Tape_Status_ERROR_Device_Disconnected)
		return Tape_Status_ERROR_Device_Disconnected;

	// Prepare input pin change: SENSE
	xuac->Board_HW->Tape->Prepare_SENSE();
	//HAL_NVIC_DisableIRQ(IRQn_Type_SENSE);

	// Clear debug variables.
	dbg_load_cnt = 0; // Count the number of loaded/inserted timestamps.
	dbg_ti2cnt = 0;   // Count the number of output compare events/timestamps.

	// Clear MainUserBuffer.
	xuac->Memory->MainUserBuffer->Clear(xuac->Memory->MainUserBuffer);

	// Prepare output compare timer cascade.
	// Set correct polarity according to uploaded configuration.
	FirstSignalEdge = (eSignalEdge_t) xuac->TapeOps->data->First_Write_Edge;
	xuac->TapeOps->TimerCtrl->Write->Setup(FirstSignalEdge);

	// Debug output: first signal edge.
	if      (FirstSignalEdge == Unspecified) xuac->Debug->dbg(DBGL_TAPE,"Tape_PrepareWrite(): FirstEdge=Unspecified(%u)\r\n", FirstSignalEdge);
	else if (FirstSignalEdge == RisingEdge)  xuac->Debug->dbg(DBGL_TAPE,"Tape_PrepareWrite(): FirstEdge=Rising(%u)\r\n", FirstSignalEdge);
	else if (FirstSignalEdge == FallingEdge) xuac->Debug->dbg(DBGL_TAPE,"Tape_PrepareWrite(): FirstEdge=Falling(%u)\r\n", FirstSignalEdge);
	else xuac->Debug->dbg(DBGL_TAPE,"Tape_PrepareWrite(): FirstEdge=error(%u)\r\n", FirstSignalEdge);
	//xuac->Debug->dbg(DBGL_TAPE,"Tape_PrepareWrite(): FirstEdge=%u\r\n", FirstSignalEdge);

	// Device is now configured for tape write operations.
	TSR |= (uint8_t)XUAC_TAP_DEVICE_CONFIG_WRITE;

	return Tape_Status_OK_Device_Configured_for_Write;
}


/**
  * @brief  Tape_GetSense
  *         Get SENSE line state.
  *
  * @retval Tape_Status_OK_Sense_On_Stop
  *           - No tape button pressed down. Tape drive is stopped.
  *         Tape_Status_OK_Sense_On_Play
  *           - A button is pressed down on tape drive.
  *         Tape_Status_ERROR_Device_Not_Configured
  *           - Hardware was not initialized (SENSE pin).
  *             Call Tape_PrepareCapture() or Tape_PrepareWrite() first.
  *         Tape_Status_ERROR_Device_Disconnected
  *           - The tape device was disconnected. Check cabling.
  */
static uint16_t Tape_GetSense(void)
{
	if (Tape_isDeviceConfigured() == Tape_Status_ERROR_Device_Not_Configured)
		return Tape_Status_ERROR_Device_Not_Configured;

	if (Tape_isDeviceConnected() == Tape_Status_ERROR_Device_Disconnected)
		return Tape_Status_ERROR_Device_Disconnected;

	GPIO_PinState PinState = HAL_GPIO_ReadPin(PORT_SENSE, PIN_SENSE);

	if (PinState == GPIO_PIN_SET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_GetSense(): GPIO_PIN_SET = 1 = <STOP>\r\n");
	if (PinState == GPIO_PIN_RESET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_GetSense(): GPIO_PIN_RESET = 0 = <PLAY>\r\n");

	// GPIO_PIN_SET   = 1 = Tape_Status_OK_Sense_On_Stop
	// GPIO_PIN_RESET = 0 = Tape_Status_OK_Sense_On_Play
	return ((PinState == GPIO_PIN_SET) ? Tape_Status_OK_Sense_On_Stop : Tape_Status_OK_Sense_On_Play);
}


/**
  * @brief  Tape_WaitForStopSense
  *         Wait for the tape drive to be stopped (all buttons released/up),
  *         or the ZoomTape board is disconnected,
  *         or an external break signal is received (user presses CTRL+C).
  *
  * @retval Tape_Status_OK_Sense_On_Stop
  *           - Tape drive was stopped. No button is pressed down.
  *         Tape_Status_ERROR_Device_Not_Configured
  *           - Hardware was not initialized (SENSE pin).
  *             Call Tape_PrepareCapture() or Tape_PrepareWrite() first.
  *         Tape_Status_ERROR_Device_Disconnected
  *           - The tape device was disconnected. Check cabling.
  *         Tape_Status_ERROR_External_Break
  *           - External break signal received (user pressed CTRL+C).
  */
static uint16_t Tape_WaitForStopSense(void)
{
	bool StopSenseDetected = false;
	GPIO_PinState PinState = HAL_GPIO_ReadPin(PORT_SENSE, PIN_SENSE);

	if (PinState == GPIO_PIN_SET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_WaitForStopSense()_0: GPIO_PIN_SET = 1 = <STOP>\r\n");
	if (PinState == GPIO_PIN_RESET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_WaitForStopSense()_0: GPIO_PIN_RESET = 0 = <PLAY>\r\n");

	if (Tape_isDeviceConfigured() == Tape_Status_ERROR_Device_Not_Configured)
		return Tape_Status_ERROR_Device_Not_Configured;

	// Assume tape device is currently connected. Tape_GetSense() checked this.

	while ((Tape_isDeviceConnected() != Tape_Status_ERROR_Device_Disconnected)
		&& (StopSenseDetected == false)
		&& (StopWaitForSense == false))
	{
		// GPIO_PIN_SET   = 1 = Tape_Status_OK_Sense_On_Stop
		// GPIO_PIN_RESET = 0 = Tape_Status_OK_Sense_On_Play
		PinState = HAL_GPIO_ReadPin(PORT_SENSE, PIN_SENSE);
		StopSenseDetected = (PinState == GPIO_PIN_SET);
	}

	if (PinState == GPIO_PIN_SET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_WaitForStopSense()_1: GPIO_PIN_SET = 1 = <STOP>\r\n");
	if (PinState == GPIO_PIN_RESET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_WaitForStopSense()_1: GPIO_PIN_RESET = 0 = <PLAY>\r\n");

	if (StopWaitForSense)
		return Tape_Status_ERROR_External_Break; // StopWaitForSense flag cleared on next PrepareRead/PrepareWrite.

	if (StopSenseDetected)
		return Tape_Status_OK_Sense_On_Stop;

	return Tape_Status_ERROR_Device_Disconnected;
}


/**
  * @brief  Tape_WaitForPlaySense
  *         Wait for a tape drive button to be pressed down,
  *         or the ZoomTape board is disconnected,
  *         or an external break signal is received (user presses CTRL+C).
  *
  * @retval Tape_Status_OK_Sense_On_Play
  *           - A tape drive button was pressed down.
  *         Tape_Status_ERROR_Device_Not_Configured
  *           - Hardware was not initialized (SENSE pin).
  *             Call Tape_PrepareCapture() or Tape_PrepareWrite() first.
  *         Tape_Status_ERROR_Device_Disconnected
  *           - The tape device was disconnected. Check cabling.
  *         Tape_Status_ERROR_External_Break
  *           - External break signal received (user pressed CTRL+C).
  */
static uint16_t Tape_WaitForPlaySense(void)
{
	bool StopSenseDetected = true;
	GPIO_PinState PinState = HAL_GPIO_ReadPin(PORT_SENSE, PIN_SENSE);

	if (PinState == GPIO_PIN_SET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_WaitForPlaySense()_0: GPIO_PIN_SET = 1 = <STOP>\r\n");
	if (PinState == GPIO_PIN_RESET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_WaitForPlaySense()_0: GPIO_PIN_RESET = 0 = <PLAY>\r\n");

	if (Tape_isDeviceConfigured() == Tape_Status_ERROR_Device_Not_Configured)
		return Tape_Status_ERROR_Device_Not_Configured;

	// Assume tape device is currently connected. Tape_GetSense() checked this.

	while ((Tape_isDeviceConnected() != Tape_Status_ERROR_Device_Disconnected)
		&& (StopSenseDetected == true)
		&& (StopWaitForSense == false))
	{
		// GPIO_PIN_SET   = 1 = Tape_Status_OK_Sense_On_Stop
		// GPIO_PIN_RESET = 0 = Tape_Status_OK_Sense_On_Play
		PinState = HAL_GPIO_ReadPin(PORT_SENSE, PIN_SENSE);
		StopSenseDetected = (PinState == GPIO_PIN_SET);
	}

	if (PinState == GPIO_PIN_SET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_WaitForPlaySense()_1: GPIO_PIN_SET = 1 = <STOP>\r\n");
	if (PinState == GPIO_PIN_RESET)	xuac->Debug->dbg(DBGL_TAPE,"Tape_WaitForPlaySense()_1: GPIO_PIN_RESET = 0 = <PLAY>\r\n");

	if (StopWaitForSense)
		return Tape_Status_ERROR_External_Break; // StopWaitForSense flag will be cleared on next PrepareRead/PrepareWrite.

	if (!StopSenseDetected)
		return Tape_Status_OK_Sense_On_Play;

	return Tape_Status_ERROR_Device_Disconnected;
}


/**
  * @brief  Tape_StartCapture
  *         Start actual tape capture: Make sure ZoomTape is connected and
  *         configured, and tape device is on <PLAY>. Enable DISCONNECT and
  *         SENSE interrupts, set tape capture in progess flag, start timer
  *         cascade for receiving timestamps.
  *
  * @retval The current global tape status value:
  *         - Tape_Status_OK (initial value set by Tape_PrepareCapture())
  *         - Tape_Status_ERROR_Device_Not_Configured
  *         - Tape_Status_ERROR_Device_Disconnected
  *         - Tape_Status_ERROR_Write_Interrupted_By_Stop
  *         - Tape_Status_ERROR_Sense_Not_On_Play
  *         - Tape_Status_ERROR_External_Break
  */
static uint16_t Tape_StartCapture(void)
{
	// Make sure we are configured for capture.
	if (Tape_isDeviceConfigured() != Tape_Status_OK_Device_Configured_for_Read)
		return Tape_Status_ERROR_Device_Not_Configured;

	// Set tape capture in progess flag.
	TSR |= (uint8_t)XUAC_TAP_CAPTURING;

	// Enable external interrupt: DISCONNECT
	xuac->Board_HW->Tape->Enable_DISCONNECT();

	// Enable external interrupt: SENSE
	xuac->Board_HW->Tape->Enable_SENSE();

	// Start timer cascade.
	xuac->TapeOps->TimerCtrl->Capture->Start();

	// Make sure the tape is still connected.
	if (Tape_isDeviceConnected() == Tape_Status_ERROR_Device_Disconnected)
	{
		Tape_StopCapture();
		return Tape_Status_ERROR_Device_Disconnected;
	}

	// Make sure the tape is still on <PLAY>.
	if (Tape_GetSense() != Tape_Status_OK_Sense_On_Play)
	{
		Tape_StopCapture();
		return Tape_Status_ERROR_Sense_Not_On_Play;
	}

	// TapeStatus was initialized to Tape_Status_OK by Tape_PrepareCapture().
	return TapeStatus;
}


/**
  * @brief  Tape_StartWrite
  *         Start actual tape write: Make sure ZoomTape is connected and
  *         configured, and tape device is on <RECORD>. Enable DISCONNECT and
  *         SENSE interrupts, set tape write in progess flag, start timer
  *         cascade for handling timestamps.
  *
  * @retval The current global tape status value:
  *
  *         - Tape_Status_OK (initial value set by Tape_PrepareCapture())
  *         - Tape_Status_ERROR_Device_Not_Configured
  *         - Tape_Status_ERROR_Device_Disconnected
  *         - Tape_Status_ERROR_Write_Interrupted_By_Stop
  *         - Tape_Status_ERROR_Sense_Not_On_Play
  *         - Tape_Status_ERROR_External_Break
  *
  *         A timestamp is loaded into the output compare timer cascade
  *         by calls to
  *         - Tape_TimerCascade_Load_and_Insert_Timestamp()
  *         - Tape_TimestampFromReceiveBuffer()
  *         prior to this routine and may have set a different status value.
  */
static uint16_t Tape_StartWrite(void)
{
	// Make sure we are configured for write.
	if (Tape_isDeviceConfigured() != Tape_Status_OK_Device_Configured_for_Write)
		return Tape_Status_ERROR_Device_Not_Configured;

	// Set tape write in progress flag.
	TSR |= (uint8_t)XUAC_TAP_WRITING;

	// Enable external interrupt: DISCONNECT
	xuac->Board_HW->Tape->Enable_DISCONNECT();

	// Enable external interrupt: SENSE
	xuac->Board_HW->Tape->Enable_SENSE();

	// Start output compare timer cascade.
	xuac->TapeOps->TimerCtrl->Write->Start();

	// Make sure the tape is still connected.
	if (Tape_isDeviceConnected() == Tape_Status_ERROR_Device_Disconnected)
	{
		Tape_StopWrite();
		return Tape_Status_ERROR_Device_Disconnected;
	}

	// Make sure the tape is still on <RECORD>.
	if (Tape_GetSense() != Tape_Status_OK_Sense_On_Play)
	{
		Tape_StopWrite();
		return Tape_Status_ERROR_Sense_Not_On_Record;
	}

	// Return current global tape status value.
	return TapeStatus;
}


/**
  * @brief  Tape_Wait_Tx
  *         Internal routine. Called while Tape_Capture() waits for the USB
  *         hardware (IN EP) to be ready to send capture data to the host.
  *
  *         May send debug output and/or blink status LED.
  * @retval -
  */
static void Tape_Wait_Tx(void)
{
	xuac->Debug->dbg(DBGL_TAPE2,"Tape_Wait_Tx()\r\n");
	//HAL_GPIO_TogglePin(PORT_USER_LED, PIN_USER_LED);
}


/**
  * @brief  Tape_TimestampFromReceiveBuffer
  *         Internal routine. Get a timestamp from the USB receive buffer
  *         and store it into LSB/MSB/MSB2 global variables.
  *         Called while writing to tape.
  *
  * @retval Tape_Status_OK
  *           - Ok, got a timestamp from receive buffer.
  *         Tape_Status_ERROR_RecvBufferEmpty
  *           - Could not get a timestamp. Not enough data in receive buffer.
  *             Either last timestamp was written to tape (tape write finished)
  *             or we got a buffer underrun (error).
  */
static uint16_t Tape_TimestampFromReceiveBuffer(void)
{
	if (RxBuffer->Entries(RxBuffer) < 2)
	{
		// Return error if only partial or no timestamp in buffer.
		return Tape_Status_ERROR_RecvBufferEmpty;
	}

	// Get timestamp.
	uint8_t data = RxBuffer->Pop(RxBuffer);
	uint8_t data2 = RxBuffer->Pop(RxBuffer);

	if ((data & 0x80) == 0)
	{
		// Short signal.
		MSB2 = 0;
		MSB = 0;
		LSB = data;
		LSB = (LSB << 8) + data2;
	}
	else
	{
		// Long signal.

		// Return error if only partial timestamp in buffer.
		if (RxBuffer->Entries(RxBuffer) < 3)
		{
			return Tape_Status_ERROR_RecvBufferEmpty;
		}

		MSB2 = data & 0x7F;
		MSB = data2;
		MSB = (MSB << 8) + RxBuffer->Pop(RxBuffer);
		LSB = RxBuffer->Pop(RxBuffer);
		LSB = (LSB << 8) + RxBuffer->Pop(RxBuffer);
	}

	//xuac->Debug->dbg(2,"TS:[%x-%x-%x]\r\n", MSB2, MSB, LSB);

	return Tape_Status_OK;
}


/**
  * @brief  Tape_TimestampIntoSendBuffer
  *         Internal routine. Get the timestamp from LSB/MSB/MSB2 global
  *         variables and store it into the USB send buffer.
  *         Called while reading from tape.
  *         If the send buffer is full the timestamp will be discarded
  *         and the global TapeStatus variable will be set to
  *         Tape_Status_ERROR_Buffer_Full.
  *
  * @retval -
  */
static void Tape_TimestampIntoSendBuffer(void)
{
	// Store timestamp in the buffer.

	if ((MSB2 != 0) || (MSB != 0) || (LSB >= 0x8000))
	{
		// Long signal.
		// MSB of 5-byte timestamp must be 1 (restricts deltas to max 9.5 hours).

		if (TxBuffer->Avail(TxBuffer) < 5)
		{
			// Error: Send buffer full.
			// Tape data comes in faster than we can send it to the host.
			// This may happen when there is noise on the tape or the host
			// is too slow.
			// In order to get a "full" tape dump for later analysis we
			// will DISCARD the current signal here and go on with the next
			// signal.

			xuac->Debug->dbg(DBGL_TAPE,"Tape_Capture(): Buffer full - discarding signal!\r\n");

			// Keep track of discarded signals.
			xuac->TapeOps->data->Discarded_Signal_Count++;

			// Set the status.
			//TapeStatus = Tape_Status_ERROR_Buffer_Full;

			//Tape_StopCapture();

			return;
		}

		// Store MSB/MSB2 in send buffer.
		TxBuffer->Push(TxBuffer, (MSB2 & 0xff) | 0x80);
		TxBuffer->Push(TxBuffer, (MSB >> 8) & 0xff);
		TxBuffer->Push(TxBuffer, MSB & 0xff);
	}

	if (TxBuffer->Avail(TxBuffer) < 2)
	{
		// Error: Send buffer full.
		// Tape data comes in faster than we can send it to the host.
		// This may happen when there is noise on the tape or the host
		// is too slow.
		// In order to get a "full" tape dump for later analysis we
		// will DISCARD the current signal here and go on with the next
		// signal.

		xuac->Debug->dbg(DBGL_TAPE,"Tape_Capture(): Buffer full - discarding signal!\r\n");

		// Keep track of discarded signals.
		xuac->TapeOps->data->Discarded_Signal_Count++;

		// Set the status.
		//TapeStatus = Tape_Status_ERROR_Buffer_Full;

		//Tape_StopCapture();

		return;
	}

	// Store LSB in send buffer.
	TxBuffer->Push(TxBuffer, LSB >> 8);
	TxBuffer->Push(TxBuffer, LSB & 0xff);
}


/**
  * @brief  HAL_TIM_IC_CaptureCallback
  *
  *         Timer4 input capture event callback while reading tape.
  *         If everything is ok the generated timestamp from the timer
  *         cascade will be stored into the send buffer.
  *         Special handling if noise is detected.
  *
  * @param  htim: Pointer to the timer handle associated with the timer that
  *               generated the capture event.
  *
  * @retval -
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	// The Overcapture flag is set to TRUE if a capture event on both
	// capture channels is detected.
	bool Overcapture = false;

	if (htim->Instance == TIM4)
	{
		// Timer4 generated an input capture event.

		// Timer4 was just resetted to 0 by hardware, Timer2 & Timer3 updated.
		// We should have enough time for reading the counter values now
		// and reset the timers (if there is no noise).

		MSB = __HAL_TIM_GET_COUNTER(&htim3);
		MSB2 = __HAL_TIM_GET_COUNTER(&htim2);
		__HAL_TIM_SET_COUNTER(&htim3, 0);
		__HAL_TIM_SET_COUNTER(&htim2, 0);

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			// We got an expected falling signal edge.
			LSB = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_1);

			// Check if we have a capture event on the other channel.
			Overcapture = __HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC2);
		}
		else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			// We got an expected rising signal edge.
			LSB = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_2);

			// Check if we have a capture event on the other channel.
			Overcapture = __HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1);
		}

		if (Overcapture)
		{
			// Noise handling:
			//
			// We have capture events on both channels. Not good.
			// Too many signals in too short time, e.g. noise on tape.
			// Discard corrupt timestamps.
			// This will reduce captured noise in CAP image.
			// At this point here we may have received further capture
			// events after clearing timers 3 & 4 above - introducing
			// false timer 3 increments while receiving noise. Hence we
			// disable capture interrupts, clear pending interrupt flags,
			// reset the timers, and enable capture interrupts again.
			//
			__HAL_TIM_DISABLE_IT(&htim4, TIM_IT_CC1 | TIM_IT_CC2);
			__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_CC1 | TIM_IT_CC2);
			__HAL_TIM_SET_COUNTER(&htim3, 0);
			__HAL_TIM_SET_COUNTER(&htim2, 0);
			__HAL_TIM_ENABLE_IT(&htim4, TIM_IT_CC1 | TIM_IT_CC2);
			xuac->TapeOps->data->Overcapture_Count++;
			//xuac->Debug->dbg(DBGL_TAPE,"__Overcapture\r\n");
			return;
		}

		if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			if (FirstSignalEdge == Unspecified)
			{
				// Remember first encountered signal edge.
				FirstSignalEdge = FallingEdge;
			}
			else if (NextSignalEdge != FallingEdge)
			{
				// We lost a signal edge.
				// Default behaviour: Discard this edge and return.
				xuac->TapeOps->data->Lost_Signal_Edges_Count++;
				return;
			}

			// Next expected signal edge will be rising.
			NextSignalEdge = RisingEdge;
		}
		else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
		{
			if (FirstSignalEdge == Unspecified)
			{
				// Remember first encountered signal edge.
				FirstSignalEdge = RisingEdge;
			}
			else if (NextSignalEdge != RisingEdge)
			{
				// We lost a signal edge.
				// Default behaviour: Discard this edge and return.
				xuac->TapeOps->data->Lost_Signal_Edges_Count++;
				return;
			}

			// Next expected signal edge will be falling.
			NextSignalEdge = FallingEdge;
		}

		// The input capture event triggered a Timer4 reset, which in turn
		// triggered an update event for Timer3. We need to undo this Timer3
		// update event in the timer cascade.

		if (MSB == 0)
		{
			MSB = 0xFFFF;
			MSB2--;
		}
		else MSB--;

		//xuac->Debug->dbg(DBGL_TAPE,"%d:[%d.%d.%d/%d]\r\n", htim->Channel, MSB2, MSB, LSB1, LSB2);

		// Store the timestamp from the timer cascade into the send buffer.
		Tape_TimestampIntoSendBuffer();
	}
}


/**
  * @brief  dbgout
  *         Internal routine. Debug output of timer cascade information.
  *         Let's see if the timer cascade was initialized correctly,
  *         especially on new hardware that is not supported so far.
  *
  * @param  id: Identification number for debug output.
  *
  * @retval Tape_Status_OK
  *         Tape_Status_ERROR_usbRecvByte
  */
static void dbgout(uint32_t id)
{
	uint32_t m2_cnt = __HAL_TIM_GET_COUNTER(&htim2);
	uint32_t m_cnt  = __HAL_TIM_GET_COUNTER(&htim3);
	uint32_t l_cnt  = __HAL_TIM_GET_COUNTER(&htim4);
	uint32_t m2_arr = __HAL_TIM_GET_AUTORELOAD(&htim2);
	uint32_t m_arr  = __HAL_TIM_GET_AUTORELOAD(&htim3);
	uint32_t l_arr  = __HAL_TIM_GET_AUTORELOAD(&htim4);
	xuac->Debug->dbg(DBGL_TAPE,"#%d:[%x-%x-%x/%x-%x-%x]\r\n", id, m2_cnt, m_cnt, l_cnt, m2_arr, m_arr, l_arr);
	//HAL_Delay(1);
}


/**
  * @brief  Tape_TimerCascade_Load_and_Insert_Timestamp
  *
  *         Internal routine. Get a timestamp from the USB receive buffer
  *         (stored into LSB/MSB/MSB2 global variables) and load it into
  *         the timer cascade. Called while writing to tape.
  *
  *
  * @retval Tape_Status_OK
  *         Tape_Status_ERROR_usbRecvByte
  */
static uint16_t Tape_TimerCascade_Load_and_Insert_Timestamp(void)
{
	// Get a timestamp from the USB receive buffer and store it
	// into LSB/MSB/MSB2 global variables.
	uint16_t result = Tape_TimestampFromReceiveBuffer(); // -> MSB2.MSB.LSB

	if (result != Tape_Status_OK)
	{
		// Could not get a timestamp. Not enough data in receive buffer.
		// Either last timestamp was written to tape (tape write finished)
		// or we got a buffer underrun (error).
		return result;
	}

	// Debug variable: Count the number of loaded/inserted timestamps.
	dbg_load_cnt++;

	// Make sure tape write is flagged.
	// if ((TSR & XUAC_TAP_WRITING) == 0) { return; }

	// The timer cascade is running, it may have already passed a small
	// LSB value. tapwrite ensures minimum signal lengths (timestamp values).
	if (((MSB == 0) && (MSB2 == 0)) || (LSB >= 0x8000))
	{
		// The LSB value of the timestamp is large enough.
		__HAL_TIM_SET_AUTORELOAD(&htim4, LSB);
		// Reset Timer4 to 16bit 0xFFFF when it reaches LSB.
		LSB = 0;
	}
	else
	{
		// Pull 0x8000 and remember.
		__HAL_TIM_SET_AUTORELOAD(&htim4, 0x8000);
		LSB += 0x8000;
	}

	// Load remaining timestamp values into running timer cascade.
	// Set Timer2 output compare match value.
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	__HAL_TIM_SET_COUNTER(&htim3, MSB);
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, MSB2+1);

	return Tape_Status_OK;
}


/**
  * @brief  HAL_TIM_OC_DelayElapsedCallback
  *
  *         Timer2 output compare match interrupt callback while writing tape.
  *         If everything is ok the next timestamp from the USB receive buffer
  *         will be loaded into the timer cascade.
  *         Tape write will be stopped on error.
  *
  * @param  htim: Pointer to the timer handle associated with the timer that
  *               generated the output compare event.
  *
  * @retval -
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	// Debug variable: Count the number of output compare events/timestamps.
	dbg_ti2cnt++;

	// Tape write: Timer2 output compare match.
	// There is no other Timer on OC generating this event.
	if (TSR & XUAC_TAP_WRITING) /*&& (htim->Instance == TIM2) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)*/
	{
		// Load next timestamp into timer cascade.
		uint16_t res = Tape_TimerCascade_Load_and_Insert_Timestamp();

		if (res != Tape_Status_OK)
		{
			Tape_StopWrite();
			TapeStatus = res;
			return;
		}

		// Clear parallel Timer4 update event if not handled yet.
		__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
	}
}


/**
  * @brief  HAL_TIM_PeriodElapsedCallback
  *
  *         Handle low 16bit value of timestamp.
  *
  * @param  htim: Pointer to the timer handle associated with the timer that
  *               generated the event.
  *
  * @retval -
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if ((TSR & XUAC_TAP_WRITING) && (htim->Instance == TIM4))
	{
		if (LSB == 0)
		{
			// Reset Timer4 to 16bit 0xFFFF.
			__HAL_TIM_SET_AUTORELOAD(&htim4, 0xFFFF);
		}
		else
		{
			// Pulled 0x8000, insert remainder.
			__HAL_TIM_SET_AUTORELOAD(&htim4, LSB);
			LSB = 0;
		}
	}
	else
	{
		xuac->Debug->dbg(DBGL_TAPE,"HAL_TIM_PeriodElapsedCallback(): else\r\n");
	}
}


/**
  * @brief  Tape_Capture
  *         Starts and handles the actual tape capture process.
  *
  * @retval
  */
static uint16_t Tape_Capture(void)
{
	// Feed the watchdog.
	xuac->Watchdog->Reset();

	// Set shortcuts: send operations & transfer buffer.
	_TxOps *TxOps = xuac->USB_Ops->TxOps;
	TxBuffer = xuac->Memory->MainUserBuffer;

	// Init a new transfer.
	xuac->USB_Ops->TxOps->InitTransfer(xuac->USB_Ops->TxOps);

	// Use supplied UserBuffer to cache the incoming tape data.
	xuac->USB_Ops->TxOps->SetTransferBuffer(xuac->USB_Ops->TxOps, TxBuffer);

	// Try to avoid SENSE signal noise.
	HAL_Delay(xuac->TapeOps->data->Sense_Delay_ms);

	// Set capture flag if all conditions met.
	TapeStatus = Tape_StartCapture();

	while (TSR & XUAC_TAP_CAPTURING)
	{
		// Try to send a packet while capturing.
		// We need enough CPU time for this.
		TxOps->TrySendPacket(TxOps);

		// Feed the watchdog while capturing.
		xuac->Watchdog->Reset();
	}

	// Finished receiving tape data.

	// Store first detected signal edge in inventory.
	xuac->TapeOps->data->First_Capture_Edge = (uint8_t) FirstSignalEdge;

	// Debug output. Let's see if everything is ok.
	xuac->Debug->dbg(DBGL_TAPE,"Tape_Capture(): First=%s, #Lost=%d, #Discarded=%d, #Overcapture=%d\r\n",
		(FirstSignalEdge==FallingEdge) ? "Falling" : "Rising",
		xuac->TapeOps->data->Lost_Signal_Edges_Count,
		xuac->TapeOps->data->Discarded_Signal_Count,
		xuac->TapeOps->data->Overcapture_Count);

	// Empty the UserBuffer when user stopped capture with Ctrl+C.
	if (TapeStatus == Tape_Status_ERROR_External_Break)
	{
		TxBuffer->Clear(TxBuffer);
	}

	// Now send remaining timestamps from UserBuffer to host.

	// Send full packets from TxBuffer.
	while (TxOps->TrySendPacket(TxOps) != Tx_Status_Too_Few_Data) { Tape_Wait_Tx(); }

	// Send remaining bytes from TxBuffer (including ZLP).
	while (TxOps->TrySendSmallPacket(TxOps) != Tx_Status_OK) { Tape_Wait_Tx(); }

	// Restore default send buffer.
	TxOps->SetTransferBuffer(TxOps, xuac->Memory->CommandBuffer);

	// Everything ok if Tape_Status_OK, return Tape_Status_OK_Capture_Finished.
	// If error occurred return specific error reason.
	return ((TapeStatus == Tape_Status_OK) ? Tape_Status_OK_Capture_Finished : TapeStatus);
}


/**
  * @brief  Tape_Write
  *         Starts and handles the actual tape write process.
  *
  * @param  ExpectedByteCount: The expected number of bytes to be received
  *                            via USB from the host. Will be checked after
  *                            tape write completes.
  * @retval
  */
static uint16_t Tape_Write(uint32_t ExpectedByteCount)
{
	// Feed the watchdog.
	xuac->Watchdog->Reset();

	// Set shortcuts: receive operations & transfer buffer.
	_RxOps *RxOps = xuac->USB_Ops->RxOps;
	RxBuffer = xuac->Memory->MainUserBuffer;

	// Select MainUserBuffer for receiving data.
	xuac->USB_Ops->RxOps->SetReceiveBuffer(xuac->USB_Ops->RxOps, RxBuffer);

	xuac->Debug->dbg(DBGL_TAPE,"Tape_Write(): RxBuffer [%u/%u]\r\n", (uint32_t)RxBuffer->Entries(RxBuffer), (uint32_t)RxBuffer->Capacity(RxBuffer));

	// Start/continue filling data into MainUserBuffer.
	xuac->USB_Ops->RxOps->StartTransfer(xuac->USB_Ops->RxOps, ExpectedByteCount, WithBufferAvailCheck);

	// If buffer empty: Wait here until a packet arrives.
	// Try to avoid SENSE signal noise.
	HAL_Delay(xuac->TapeOps->data->Sense_Delay_ms);

	do {

		dbgout(0); // Output timer cascade information.

		// Load timestamp into output compare timer cascade.
		/*result =*/ Tape_TimerCascade_Load_and_Insert_Timestamp();

		// ToDo: Handle error here (buffer empty) ?
		//if (result != Tape_Status_OK) { }

		dbgout(1); // Output timer cascade information.

		// Set write flag if everything ok.
		TapeStatus = Tape_StartWrite();

		// ToDo: Handle errors here.
		//if (result != Tape_Status_OK) { }

		// Loop until last signal was written to tape.
		while (TSR & XUAC_TAP_WRITING)
		{
			// Feed the watchdog while writing.
			xuac->Watchdog->Reset();

			// Release receive buffer if transfer not finished and
			// enough space for a packet.
			RxOps->TryReceivePacket(RxOps, WithBufferAvailCheck);
		}

		// Debug output. Let's see if everything is ok.
		xuac->Debug->dbg(DBGL_TAPE,"Tape_Write(): <End> ti2cnt=0x%x, load_cnt=0x%x\r\n", dbg_ti2cnt, dbg_load_cnt);
		HAL_Delay(2);
		xuac->Debug->dbg(DBGL_TAPE,"Tape_Write(): <End> Expected=0x%x, RxCount=0x%x [%u/%u]\r\n",
			ExpectedByteCount,
			RxOps->data->TransferByteCount,
			RxBuffer->Entries(RxBuffer),
			RxBuffer->Capacity(RxBuffer));
		HAL_Delay(2);

		// Check if tape write was successful.
		// ToDo: Rethink the checks here.
		// ToDo: Handle "RxByteCounter<5" or "DeltaCount==0" ?
		if ((TapeStatus == Tape_Status_ERROR_RecvBufferEmpty)
			&& (RxBuffer->Entries(RxBuffer) == 0)
			&& (RxOps->data->TransferByteCount == ExpectedByteCount)
			&& (RxOps->isTransferFinished(RxOps)))
		{
			TapeStatus = Tape_Status_OK;
		}
		else // ToDo: if (!RxOps->isTransferFinished(RxOps))
		{
			// Stall OUT EP if tape write stopped early.
			xuac->Debug->dbg(DBGL_TAPE,"Tape_Write(): StallEP(%u)\r\n", CDC_OUT_EP);
			USBD_LL_StallEP(&hUsbDeviceFS, CDC_OUT_EP);
		}

	} while(0);

	// Note:
	// Ctrl+C may have arrived after releasing the endpoint, hence the
	// endpoint may still be released when leaving this function.

	// Everything ok if Tape_Status_OK, return Tape_Status_OK_Write_Finished.
	// If error occurred return specific error reason.
	return ((TapeStatus == Tape_Status_OK) ? Tape_Status_OK_Write_Finished : TapeStatus);
}


/**
  * @brief  Tape_StopCapture
  *         Stops a tape capture in progress.
  *
  *         Clears the tape capture in progress flag, turns the motor off,
  *         stops the timer cascade, disables DISCONNECT and SENSE interrupts.
  *
  * @retval -
  */
static void Tape_StopCapture(void)
{
	// Flag tape capture stop.
	TSR &= (uint8_t)~XUAC_TAP_CAPTURING;

	// Turn motor off.
	// Note: Tape_MotorOff() checks for DISCONNECT.

	// Set MOTOR CONTROL flag.
	TSR |= XUAC_TAP_MOTOR;

	// Turn motor off.
	xuac->Board_HW->Tape->MOTOR_Off();

	// Stop timer cascade.
	xuac->TapeOps->TimerCtrl->Capture->Stop();

	// Disable external interrupt: DISCONNECT
	xuac->Board_HW->Tape->Disable_DISCONNECT();

	// Disable external interrupt: SENSE
	xuac->Board_HW->Tape->Disable_SENSE();
}


/**
  * @brief  Tape_StopWrite
  *         Stops a tape write in progress.
  *
  *         Clears the tape write in progress flag, turns the motor off,
  *         stops the timer cascade, disables DISCONNECT and SENSE interrupts.
  *
  * @retval -
  */
static void Tape_StopWrite(void)
{
	xuac->Debug->dbg(DBGL_TAPE,"Tape_StopWrite()\r\n");

	// Flag tape write stop.
	TSR &= (uint8_t)~XUAC_TAP_WRITING;

	// Turn motor off.
	// Note: Tape_MotorOff() checks for DISCONNECT.

	// Set MOTOR CONTROL flag.
	TSR |= XUAC_TAP_MOTOR;

	// Turn motor off.
	xuac->Board_HW->Tape->MOTOR_Off();

	// Stop timer cascade.
	xuac->TapeOps->TimerCtrl->Write->Stop();

	// Disable external interrupt: DISCONNECT
	xuac->Board_HW->Tape->Disable_DISCONNECT();

	// Disable external interrupt: SENSE
	xuac->Board_HW->Tape->Disable_SENSE();
}


/**
  * @brief  Tape_Reset
  *         External break received. Stop tape capture & write in progress,
  *         and break out of wait loops (see Tape_WaitForPlaySense() and
  *         Tape_WaitForStopSense()).
  *
  * @retval 1 if tape write was stopped, 0 otherwise.
  */
static uint8_t Tape_Reset(void)
{
	uint8_t result = 0;

	// ToDo: Use BreakAllLoops instead of StopWaitForSense ?
	StopWaitForSense = true; // Stop WaitForSense loops.
	BreakAllLoops = true; // Break out of all loops.

	// Flag tape capture/write stop.
	if (TSR & XUAC_TAP_CAPTURING) Tape_StopCapture();
	if (TSR & XUAC_TAP_WRITING)
	{
		//result = 1; // Stall OUT EP, see SetAddress() in ctlreq.c.
		Tape_StopWrite();
	}

	// Set global TapeStatus so Tape_Capture() and Tape_Write() know
	// what happened.
	TapeStatus = Tape_Status_ERROR_External_Break;

	return result;
}
