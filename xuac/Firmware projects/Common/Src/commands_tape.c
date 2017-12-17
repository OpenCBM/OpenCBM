/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "commands_tape.h"
#include "requests.h" // USB bulk request IDs
#include "status.h" // USB bulk request status values
#include "xuac.h" // the inventory

// Implementation

bool isTapeCmd(uint8_t cmd)
{
	return ((XUAC_TAP_MOTOR_ON <= cmd) && (cmd <= XUAC_TAP_MOTOR_OFF));
}


void HandleBulkCmd_Tape(uint8_t cmd, uint8_t subcmd, uint32_t cmd_data,
                        uint8_t *device_status, uint16_t *request_status,
                        uint32_t *answer_data)
{
	*device_status = XUAC_IO_READY;
	*answer_data = 0;

	switch (cmd) {
		case XUAC_TAP_MOTOR_ON:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_MOTOR_ON\r\n");
			*request_status = xuac->TapeOps->MotorOn();
			break;
		case XUAC_TAP_GET_VER:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_GET_VER\r\n");
			*request_status = xuac->TapeOps->GetFirmwareVersion();
			break;
		case XUAC_TAP_PREPARE_CAPTURE:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_PREPARE_CAPTURE\r\n");
			*request_status = xuac->TapeOps->PrepareCapture();
			break;
		case XUAC_TAP_PREPARE_WRITE:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_PREPARE_WRITE\r\n");
			*request_status = xuac->TapeOps->PrepareWrite();
			break;
		case XUAC_TAP_GET_SENSE:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_GET_SENSE\r\n");
			*request_status = xuac->TapeOps->GetSense();
			break;
		case XUAC_TAP_WAIT_FOR_STOP_SENSE:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_WAIT_FOR_STOP_SENSE\r\n");
			*request_status = xuac->TapeOps->WaitForStopSense();
			break;
		case XUAC_TAP_WAIT_FOR_PLAY_SENSE:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_WAIT_FOR_PLAY_SENSE\r\n");
			*request_status = xuac->TapeOps->WaitForPlaySense();
			break;
		case XUAC_TAP_CAPTURE:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_CAPTURE\r\n");
			*request_status = xuac->TapeOps->Capture();
			break;
		case XUAC_TAP_WRITE:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_WRITE\r\n");
			*request_status = xuac->TapeOps->Write(cmd_data);
			break;
		case XUAC_TAP_MOTOR_OFF:
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - XUAC_TAP_MOTOR_OFF\r\n");
			*request_status = xuac->TapeOps->MotorOff();
			break;
		default:
			// Should not occur, BulkWorker() checked it.
			xuac->Debug->dbg(DBGL_USBFUNC,"HandleBulkCmd_Tape() - default\r\n");
			*request_status = Tape_Status_ERROR_Unsupported_Function;
	}
}
