/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "commands.h"
#include "requests.h" // USB bulk request IDs
#include "status.h" // USB bulk request status values
#include "xuac.h" // the inventory

void HandleBulkCmd(uint8_t cmd, uint8_t subcmd, uint32_t cmd_data,
                   uint8_t *device_status, uint16_t *request_status,
                   uint32_t *answer_data)
{
	*device_status = XUAC_IO_READY;
	*answer_data = 0;

	switch (cmd) {
		case XUAC_BOARD_INFO:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd() - XUAC_BOARD_INFO\r\n");
			*request_status = xuac->Board_HW->Board_Info(cmd_data);
			break;
		case XUAC_MEMORY_TRANSFERBUFFER_CLEAR:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd() - XUAC_MEMORY_MAINUSERBUFFER_CLEAR\r\n");
			*request_status = xuac->Memory->MainUserBuffer_Clear();
			break;
		case XUAC_MEMORY_TRANSFERBUFFER_ISFULL:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd() - XUAC_MEMORY_MAINUSERBUFFER_ISFULL\r\n");
			*request_status = xuac->Memory->MainUserBuffer_isFull();
			break;
		case XUAC_MEMORY_TRANSFERBUFFER_FILL:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd() - XUAC_MEMORY_MAINUSERBUFFER_FILL\r\n");
			*request_status = xuac->Memory->MainUserBuffer_Fill(cmd_data);
			break;
		case XUAC_SET_VALUE32:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd() - XUAC_SET_VALUE32\r\n");
			*request_status = xuac->DataExchangeOps->SetValue32(subcmd, cmd_data);
			break;
		case XUAC_GET_VALUE32:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd() - XUAC_GET_VALUE32\r\n");
			*request_status = xuac->DataExchangeOps->GetValue32(subcmd, answer_data);
			break;
		default:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd() - default\r\n");
			*request_status = XUAC_Status_ERROR_Unsupported_Function;
	}
}
