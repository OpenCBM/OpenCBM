/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __TRANSFER_H_
#define __TRANSFER_H_

#include <stdint.h>  // for uint8_t & uint32_t
#include "stdbool.h" // for bool
#include "UserBuffer.h"

// BufferAvailCheck:
// Before preparing an EP to receive a packet we can make
// sure the selected receive buffer can take it.
#define WithBufferAvailCheck    true
#define NoBufferAvailCheck      false

// Transfer status values.

#define Tx_Status_OK            1
#define Tx_Status_ERROR         255
#define Tx_Status_Not_Ready     (Tx_Status_ERROR - 1)
#define Tx_Status_Too_Few_Data  (Tx_Status_ERROR - 2)
#define Tx_Status_Too_Much_Data (Tx_Status_ERROR - 3)

// Structures for the inventory

typedef struct __RxOps _RxOps;
typedef struct __TxOps _TxOps;

typedef struct
{
	uint32_t ExpectedByteCount;
	uint32_t TransferByteCount;
	bool     TransferFinished;
	bool     PacketReceived;
	bool     BufferOverflow;
} RxData;

typedef struct __RxOps
{
	RxData    *data;
	void     (*InitTransfer)            (struct __RxOps *ops, uint32_t ExpectedByteCount);
	void     (*SetReceiveBuffer)        (struct __RxOps *ops, UserBuffer *NewRxBuffer);
	void     (*StartTransfer)           (struct __RxOps *ops, uint32_t ExpectedByteCount, bool BufferAvailCheck);
	bool     (*isTransferFinished)      (struct __RxOps *ops);
	bool     (*isPacketReceived)        (struct __RxOps *ops);
	void     (*TryReceivePacket)        (struct __RxOps *ops, bool BufferAvailCheck);
	int8_t   (*HandleReceive)           (struct __RxOps *ops, uint8_t* Buf, uint32_t *Len);
	bool     (*isBufferOverflow)        (struct __RxOps *ops);
	//void   (*StallRx)                   (struct __RxOps *ops);
	//void   (*ReleaseStallRx)            (struct __RxOps *ops);
} _RxOps;

typedef struct
{
	uint32_t   TxByteCounter;
} TxData;

typedef struct __TxOps
{
	TxData    *data;
	void     (*InitTransfer)       (struct __TxOps *ops);
	void     (*SetTransferBuffer)  (struct __TxOps *ops, UserBuffer *NewTxBuffer);
	uint8_t  (*TrySendPacket)      (struct __TxOps *ops);
	uint8_t  (*TrySendSmallPacket) (struct __TxOps *ops);
	void     (*SyncSendPacket)     (struct __TxOps *ops, uint8_t* Buf, uint16_t Len);
	uint8_t  (*HandleSend)         (struct __TxOps *ops, uint8_t* Buf, uint16_t Len);
} _TxOps;

#endif /* __TRANSFER_H_ */
