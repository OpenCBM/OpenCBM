/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "board_info.h"
#include "requests.h" // USB bulk request IDs
#include "status.h" // USB bulk request status values
#include "xuac.h" // the inventory

// External variables

// Reference to the buffer for sending bulk data.
extern UserBuffer *TxBuffer;

// Forward declarations

void Board_Wait_Tx(void);

// Implementation

/**
  * @brief  Board_Wait_Tx
  *         Internal routine. Called by Board_Info() while waiting until
  *         a transfer is possible.
  *         Empty routine, may e.g. blink status LED, reset watchdog,
  *         or output a debug message.
  * @param  InfoRequest: The ID of the requested information.
  * @retval Info_Status_OK_Info_Sent
  *           - OK.
  *         Info_Status_ERROR_Unknown_Request
  *           - Unknown request ID.
  */
void Board_Wait_Tx(void)
{
	//xuac->Debug->dbg(1,"Board_Wait_Tx()\r\n");
	//HAL_GPIO_TogglePin(PORT_USER_LED, PIN_USER_LED);
}


/**
  * @brief  Board_Info
  *         Request information about the board.
  * @param  InfoRequest: The ID of the requested information.
  * @retval Info_Status_OK_Info_Sent
  *           - OK.
  *         Info_Status_ERROR_Unknown_Request
  *           - Unknown request ID.
  */
uint16_t Board_Info(uint32_t InfoRequest)
{
	uint32_t i;

	uint16_t ret = Info_Status_OK_Info_Sent;

	_TxOps *TxOps = xuac->USB_Ops->TxOps;

	xuac->Debug->dbg(DBGL_INFO,"BoardInfo(): Request=%u\r\n", InfoRequest);

	// Select MainUserBuffer for sending data.
	xuac->USB_Ops->TxOps->SetTransferBuffer(xuac->USB_Ops->TxOps, xuac->Memory->MainUserBuffer);

	// Clear the buffer.
	xuac->Memory->CommandBuffer->Clear(xuac->Memory->MainUserBuffer);

	xuac->USB_Ops->TxOps->InitTransfer(xuac->USB_Ops->TxOps);

	uint8_t *info = NULL;
	uint8_t  value32[5];
	uint32_t tmp;

	switch (InfoRequest) {
		case INFO_BOARD_NAME:
			info = xuac->Board_HW->info->Board_Name;
			break;
		case INFO_MCU_NAME:
			info = xuac->Board_HW->info->MCU_Name;
			break;
		case INFO_MCU_MHZ:
			info = xuac->Board_HW->info->MCU_MHZ_Str;
			break;
		case INFO_FIRMWARE_VERSION:
			info = xuac->Board_HW->info->Firmware_Version;
			break;
		case INFO_FIRMWARE_VERSION_MAJOR:
			info = xuac->Board_HW->info->Firmware_Version_Major_Str;
			break;
		case INFO_FIRMWARE_VERSION_MINOR:
			info = xuac->Board_HW->info->Firmware_Version_Minor_Str;
			break;
		case INFO_FIRMWARE_VERSION_SUB:
			info = xuac->Board_HW->info->Firmware_Version_Sub_Str;
			break;
		case INFO_TIMER_SPEED_MHZ:
			info = xuac->Board_HW->info->Timer_Speed_MHZ_Str;
			break;
		case INFO_BUFFER_SIZE_STR:
			info = xuac->Board_HW->info->Buffer_Size_Str;
			break;
		case INFO_BUFFER_SIZE_VAL_STR:
			info = xuac->Board_HW->info->Buffer_Size_Value_Str;
			break;
		case INFO_BOARD_CAPABILITIES:
			info = (uint8_t *) value32;
			tmp = xuac->Board_HW->info->Board_Capabilities;
			value32[0] = (tmp      ) & 0xFF;
			value32[1] = (tmp >>  8) & 0xFF;
			value32[2] = (tmp >> 16) & 0xFF;
			value32[3] = (tmp >> 24) & 0xFF;
			value32[4] = 0;
			//*((uint32_t *)((void *) value32)) = xuac->Board_HW->info->Board_Capabilities;
			break;
		case INFO_SAMPLING_RATE:
			info = xuac->Board_HW->info->Sampling_Rate;
			break;
    	default:
    		xuac->Debug->dbg(DBGL_INFO,"BoardInfo() - default\r\n");
    		ret = Info_Status_ERROR_Unknown_Request;
    }

	// Copy requested board info into transfer buffer.
	if (info != NULL)
	{
		i = 0;
		while ((*info != '\0') && (i < BOARD_INFO_MAX_SIZE))
		{
			TxBuffer->Push(TxBuffer, *info);
			info++;
			i++;
		}
	}

	// Send full packets from TxBuffer (size == APP_TX_DATA_SIZE)
	// until not enough data for a full packet is left.
	while (TxOps->TrySendPacket(TxOps) != Tx_Status_Too_Few_Data) { Board_Wait_Tx(); }

	// Send remaining bytes from TxBuffer (~including ZLP).
	while (TxOps->TrySendSmallPacket(TxOps) != Tx_Status_OK) { Board_Wait_Tx(); }

	// Restore default send buffer.
	TxOps->SetTransferBuffer(TxOps, xuac->Memory->CommandBuffer);

	return ret;
}
