/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "value.h"
#include "requests.h" // USB bulk request IDs
#include "status.h" // USB bulk request status values
#include "xuac.h" // the inventory

// Forward declarations

static uint16_t SetValue32(uint8_t ValueID, uint32_t data);
static uint16_t GetValue32(uint8_t ValueID, uint32_t *data);

// Register operations in inventory

Data_Exchange_Operations DataExchangeOps =
{
	SetValue32,
	GetValue32
};

// Implementation

static uint16_t SetValue32(uint8_t ValueID, uint32_t data)
{
	uint16_t result = XUAC_Status_OK;

	switch (ValueID) {
		case XUAC_TAP_SET_SENSE_DELAY:
			xuac->Debug->dbg(DBGL_USBFUNC,"SetValue32() - XUAC_TAP_SET_SENSE_DELAY\r\n");
			xuac->TapeOps->data->Sense_Delay_ms = data;
			break;
		case XUAC_TAP_SET_FIRST_WRITE_EDGE:
			xuac->Debug->dbg(DBGL_USBFUNC,"SetValue32() - XUAC_TAP_SET_FIRST_WRITE_EDGE\r\n");
			xuac->TapeOps->data->First_Write_Edge = data;
			break;
		case XUAC_SET_DBG_LEVEL:
			xuac->Debug->dbg(DBGL_USBFUNC,"SetValue32() - XUAC_SET_DBG_LEVEL\r\n");
			xuac->Debug->dbg_level = data;
			break;
		default:
			xuac->Debug->dbg(DBGL_USBFUNC,"SetValue32() - default\r\n");
			result = XUAC_Status_ERROR_Unsupported_Function; // _SubFunction!
	}

	return result;
}


static uint16_t GetValue32(uint8_t ValueID, uint32_t *data)
{
	uint16_t result = XUAC_Status_OK;

	switch (ValueID) {
		case XUAC_TAP_GET_SENSE_DELAY:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - XUAC_TAP_GET_SENSE_DELAY\r\n");
			*data = xuac->TapeOps->data->Sense_Delay_ms;
			break;
		case XUAC_TAP_GET_FIRST_WRITE_EDGE:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - XUAC_TAP_GET_FIRST_WRITE_EDGE\r\n");
			*data = xuac->TapeOps->data->First_Write_Edge;
			break;
		case XUAC_TAP_GET_FIRST_CAPTURE_EDGE:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - XUAC_TAP_GET_FIRST_CAPTURE_EDGE\r\n");
			*data = xuac->TapeOps->data->First_Capture_Edge;
			break;
		case XUAC_TAP_GET_LOST_COUNT:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - XUAC_TAP_GET_LOST_COUNT\r\n");
			*data = xuac->TapeOps->data->Lost_Signal_Edges_Count;
			break;
		case XUAC_TAP_GET_DISCARD_COUNT:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - XUAC_TAP_GET_DISCARD_COUNT\r\n");
			*data = xuac->TapeOps->data->Discarded_Signal_Count;
			break;
		case XUAC_TAP_GET_OVERCAPTURE_COUNT:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - XUAC_TAP_GET_OVERCAPTURE_COUNT\r\n");
			*data = xuac->TapeOps->data->Overcapture_Count;
			break;
		case XUAC_MEMORY_GET_BUFFER_COUNT:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - XUAC_MEMORY_GET_BUFFER_COUNT\r\n");
			*data = xuac->Memory->MainUserBuffer->Entries(xuac->Memory->MainUserBuffer);
			break;
		case XUAC_GET_DBG_LEVEL:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - XUAC_GET_DBG_LEVEL\r\n");
			*data = xuac->Debug->dbg_level;
			break;
		default:
			xuac->Debug->dbg(DBGL_USBFUNC,"GetValue32() - default\r\n");
			result = XUAC_Status_ERROR_Unsupported_Function; // _SubFunction!
	}

	return result;
}
