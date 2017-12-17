/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __XUAC_H_
#define __XUAC_H_

#include <stdint.h> // for uint8_t, uint16_t, uint32_t
#include "stdbool.h" // for boolean type

// Include STM firmware

#include "fw.h"

// Include board info

#include "board_defs.h"
#include "version.h"

// Include all structures needed for the inventory.

#include "ctlreq.h"
#include "BulkWorker.h"
#include "commands.h"
#include "commands_tape.h"
#include "transfer.h"
#include "UserBuffer.h"
#include "tape_153x.h"
#include "tape_timer.h"
#include "watchdog.h"
#include "error_handler.h"
#include "board_tape.h"
#include "board.h"
#include "dbg.h"
#include "mem.h"
#include "value.h"
#include "status_led.h"

// Structures for the inventory

// ToDo: Rethink structure, identifiers and types.

typedef struct _USB_Operations
{
	//void               *PostInit; // set Priority
	CtlReq_Operations  *CtlReq;
	void              (*BulkWorker)             (void);
	void              (*HandleBulkCommand)      (uint8_t cmd, uint8_t subcmd, uint32_t cmd_data, uint8_t *device_status, uint16_t *request_status, uint32_t *answer_data);
	void              (*HandleBulkCommand_Tape) (uint8_t cmd, uint8_t subcmd, uint32_t cmd_data, uint8_t *device_status, uint16_t *request_status, uint32_t *answer_data);
	bool              (*isTapeCommand)          (uint8_t cmd);
	_RxOps             *RxOps;
	_TxOps             *TxOps;
} USB_Operations;

// ToDo: Rethink structure, identifiers and types.

typedef struct
{
	void                    (*Init) (void);
	Tape_Operations          *TapeOps;
	Watchdog_Operations      *Watchdog;
	ErrorHandler_Operations  *ErrorHandler;
	Board_HW_Operations      *Board_HW;
	Debug_Operations         *Debug;
	USB_Operations           *USB_Ops;
	_Memory                  *Memory;
	Data_Exchange_Operations *DataExchangeOps;
	StatusLED_Operations     *StatusLED_Ops;
} xuac_Operations;

// Available variables.

extern _RxOps          *RxOps;
extern _TxOps          *TxOps;
extern xuac_Operations *xuac; // The inventory.

#endif /* __XUAC_H_ */
