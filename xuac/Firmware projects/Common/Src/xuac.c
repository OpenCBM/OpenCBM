/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "xuac.h"

// Assemble XUAC inventory.

extern Board_HW_Operations      Board_HW_Ops;
extern CtlReq_Operations        CtlReqOps;
extern Tape_Operations          TapeOps_153x;
extern Watchdog_Operations      WatchdogOps;
extern ErrorHandler_Operations  ErrorHandlerOps;
extern Debug_Operations         DebugOps;
extern _RxOps                   TheRxOps;
extern _TxOps                   TheTxOps;
extern _Memory                  TheMemory;
extern Data_Exchange_Operations DataExchangeOps;
extern StatusLED_Operations     StatusLED_Ops;

extern void Init(void);

USB_Operations USB_Ops =
{
	&CtlReqOps,
	BulkWorker,
	HandleBulkCmd,
	HandleBulkCmd_Tape,
	isTapeCmd,
	&TheRxOps,
	&TheTxOps
};

xuac_Operations xuac_Ops =
{
	Init,
	&TapeOps_153x,
	&WatchdogOps,
	&ErrorHandlerOps,
	&Board_HW_Ops,
	&DebugOps,
	&USB_Ops,
	&TheMemory,
	&DataExchangeOps,
	&StatusLED_Ops
};

// Set global inventory reference.
xuac_Operations *xuac  = &xuac_Ops;
