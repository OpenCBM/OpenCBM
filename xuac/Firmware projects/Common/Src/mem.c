/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "usbd_cdc_if.h" // for CDC_DATA_FS_MAX_PACKET_SIZE
#include "mem.h"
#include "status.h" // USB bulk request status values
#include "xuac.h" // the inventory

#define COMMAND_BUFFER_SIZE CDC_DATA_FS_MAX_PACKET_SIZE

// Allocate main buffer for USB data transfers.
static uint8_t MainBuffer[BUFFER_SIZE_VAL];

// Allocate command buffer for receiving USB bulk requests.
static uint8_t CommandBuffer[COMMAND_BUFFER_SIZE];

// Reference to custom defined buffer for receiving bulk data.
extern UserBuffer *volatile RxBuffer;

// Register main buffer -------------------------------------------------------

// Set main buffer parameters.
UserBufferData MainBufferData =
{
	MainBuffer,      /* *buffer  */
	0,               /* readPos  */
	0,               /* writePos */
	0,               /* entries  */
	BUFFER_SIZE_VAL, /* capacity */
};

// Set main buffer operations.
UserBuffer MainUserBuffer =
{
	&MainBufferData,
	UserBuffer_Capacity,
	UserBuffer_Entries,
	UserBuffer_Avail,
	UserBuffer_PushN,
	UserBuffer_PopN,
	UserBuffer_PushN_safe,
	UserBuffer_PopN_safe,
	UserBuffer_Push,
	UserBuffer_Pop,
	UserBuffer_Clear
};

// Register command buffer ----------------------------------------------------

// Set command buffer parameters.
UserBufferData CommandBufferData =
{
	CommandBuffer,       /* *buffer  */
	0,                   /* readPos  */
	0,                   /* writePos */
	0,                   /* entries  */
	COMMAND_BUFFER_SIZE, /* capacity */
};

// Set command buffer operations.
UserBuffer CommandUserBuffer =
{
	&CommandBufferData,
	UserBuffer_Capacity,
	UserBuffer_Entries,
	UserBuffer_Avail,
	UserBuffer_PushN,
	UserBuffer_PopN,
	UserBuffer_PushN_safe,
	UserBuffer_PopN_safe,
	UserBuffer_Push,
	UserBuffer_Pop,
	UserBuffer_Clear
};

// ----------------------------------------------------------------------------

// Register main & command buffers in inventory
_Memory TheMemory =
{
	&MainUserBuffer,
	&CommandUserBuffer,
	Memory_MainUserBuffer_Clear,
	Memory_MainUserBuffer_isFull,
	Memory_MainUserBuffer_Fill
};

// ----------------------------------------------------------------------------

// Implementation

uint16_t Memory_MainUserBuffer_Clear(void)
{
	xuac->Memory->MainUserBuffer->Clear(xuac->Memory->MainUserBuffer);
	return Memory_Status_OK_Cleared;
}

uint16_t Memory_MainUserBuffer_isFull(void)
{
	if (xuac->Memory->MainUserBuffer->Avail(xuac->Memory->MainUserBuffer) == 0)
	{
		return Memory_Status_OK_Buffer_Full;
	}
	return Memory_Status_OK_Buffer_Not_Full;
}

// Internal function.
// Called while the application waits to receive data from host.
void Memory_Wait_Rx(void)
{
	xuac->Debug->dbg(DBGL_MEM2,"Memory_Wait_Rx()\r\n");
	//HAL_GPIO_TogglePin(PORT_USER_LED, PIN_USER_LED);
}

uint16_t Memory_MainUserBuffer_Fill(uint32_t NumBytes)
{
	uint16_t res = Memory_Status_OK_Buffer_Filled;

	// Feed the watchdog.
	//xuac->Watchdog->Reset();

	_RxOps *RxOps = xuac->USB_Ops->RxOps;
	UserBuffer *RxBuffer = xuac->Memory->MainUserBuffer;

	// Select MainUserBuffer for receiving data.
	xuac->USB_Ops->RxOps->SetReceiveBuffer(xuac->USB_Ops->RxOps, xuac->Memory->MainUserBuffer);

	xuac->Debug->dbg(DBGL_MEM,"Fill(): <start> [%u/%u]\r\n", (uint32_t)RxBuffer->Entries(RxBuffer), (uint32_t)RxBuffer->Capacity(RxBuffer));

	// Start/continue filling data into MainUserBuffer.
	xuac->USB_Ops->RxOps->StartTransfer(xuac->USB_Ops->RxOps, NumBytes, NoBufferAvailCheck);

	// Fill incoming data into MainUserBuffer until
	// USB transfer is finished.
	// Host application has to prevent buffer overflow:
	// buffer overflows are handled as errors and data is lost.
	while (!(RxOps->isTransferFinished(RxOps))) //&& (!(BreakAllLoops))
	{
		xuac->Watchdog->Reset(); // Feed the watchdog.
		// Try to prepare EP for receiving a packet.
		RxOps->TryReceivePacket(RxOps, NoBufferAvailCheck);
		//Memory_Wait_Rx();
	}

	xuac->Debug->dbg(DBGL_MEM,"Fill(): <end> [%u/%u]\r\n", (uint32_t)RxBuffer->Entries(RxBuffer), (uint32_t)RxBuffer->Capacity(RxBuffer));

	// Check if buffer overflow occurred (data lost).
	if (RxOps->isBufferOverflow(RxOps))
	{
		res = Memory_Status_ERROR_Not_Enough_Space;
	}

	// Return status.
	return res;
}
