/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "BulkWorker.h"
#include "xuac.h" // the inventory

// Local command and answer buffer sizes.
#define XUAC_CMD_LEN     (uint8_t) 6
#define XUAC_ANSWER_LEN  (uint8_t) 7

// Local command and answer buffers.
static uint8_t cmdBuf[XUAC_CMD_LEN];
static uint8_t answerBuf[XUAC_ANSWER_LEN];

// Received USB bulk command.
static uint8_t  cmd;
static uint8_t  subcmd;
static uint32_t cmd_data;

// USB bulk answer.
static uint8_t  device_status;
static uint16_t request_status;
static uint32_t answer_data;

void BulkWorker(void)
{
	// ToDo: Verify received command. Use CRC hardware component.
	if (xuac->Memory->CommandBuffer->Entries(xuac->Memory->CommandBuffer) < XUAC_CMD_LEN)
	{
		return; // No valid command received.
	}

	// Get command from buffer and decode it.
	xuac->Memory->CommandBuffer->PopN(xuac->Memory->CommandBuffer, cmdBuf, sizeof(cmdBuf));
	cmd = cmdBuf[0];
	subcmd = cmdBuf[1];
	cmd_data = *((uint32_t *) (cmdBuf+2));

	xuac->Debug->dbg(DBGL_USBFUNC,"Command received: %u:%u:%u\r\n", cmd, subcmd, cmd_data);

	// Handle the command.
	if (xuac->USB_Ops->isTapeCommand(cmd))
	{
    	xuac->USB_Ops->HandleBulkCommand_Tape(cmd, subcmd, cmd_data, &device_status, &request_status, &answer_data);
	}
	else
	{
		xuac->USB_Ops->HandleBulkCommand(cmd, subcmd, cmd_data, &device_status, &request_status, &answer_data);
	}

	// Assemble request answer.
	*((uint8_t  *)(answerBuf  )) = device_status;
	*((uint16_t *)(answerBuf+1)) = request_status;
	*((uint32_t *)(answerBuf+3)) = answer_data;

	// Prepare receiving next command before sending the status.

	// Set buffer for receiving next command.
	xuac->USB_Ops->RxOps->SetReceiveBuffer(xuac->USB_Ops->RxOps, xuac->Memory->CommandBuffer);

	// Clear buffer for receiving next command.
	xuac->Memory->CommandBuffer->Clear(xuac->Memory->CommandBuffer);

	// Prepare EP to receive a packet with a new command.
	// EP may still be released here when the user stopped the last
	// Tape_Write() with a CTRL+C.
	xuac->USB_Ops->RxOps->StartTransfer(xuac->USB_Ops->RxOps, XUAC_CMD_LEN, WithBufferAvailCheck);

	// Send status to host (synchronously).
	xuac->USB_Ops->TxOps->SyncSendPacket(xuac->USB_Ops->TxOps, answerBuf, sizeof(answerBuf));

	xuac->Debug->dbg(DBGL_USBFUNC,"Status: %02x:%04x:%08x\r\n", device_status, request_status, answer_data);
	HAL_Delay(2);
	xuac->Debug->dbg(DBGL_USBFUNC,"Awaiting command.\r\n");
}
