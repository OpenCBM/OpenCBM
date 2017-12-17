/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

// Note: This is early prototype code, alpha test version.
// ToDo: May need to rethink/optimize the whole USB transfer.

#include "transfer.h"
#include "usbd_cdc_if.h"
#include "xuac.h" // the inventory

// We need access to the USB device handle.
extern USBD_HandleTypeDef hUsbDeviceFS;

// We need access to USB CDC transmit buffer. Needed below.
#define APP_TX_DATA_SIZE  USB_FS_MAX_PACKET_SIZE
extern uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

// The custom defined buffers for send & receive bulk data.
// There will be only one registered buffer of each type (send & receive)
// for USB bulk transfers at any time.
UserBuffer *RxBuffer = NULL;
UserBuffer *TxBuffer = NULL;

// Forward declarations

static void    Rx_InitTransfer       (_RxOps *ops, uint32_t ExpectedByteCount);
static void    Rx_SetReceiveBuffer   (_RxOps *ops, UserBuffer *NewRxBuffer);
static void    Rx_StartTransfer      (_RxOps *ops, uint32_t ExpectedByteCount, bool BufferAvailCheck);
static bool    Rx_isTransferFinished (_RxOps *ops);
static bool    Rx_isPacketReceived   (_RxOps *ops);
static void    Rx_TryReceivePacket   (_RxOps *ops, bool BufferAvailCheck);
static int8_t  Rx_HandleReceive      (_RxOps *ops, uint8_t* Buf, uint32_t *Len);
static bool    Rx_isBufferOverflow   (_RxOps *ops);

static void    Tx_InitTransfer       (_TxOps *ops);
static void    Tx_SetTransferBuffer  (_TxOps *ops, UserBuffer *NewTxBuffer);
static uint8_t Tx_TrySendPacket      (_TxOps *ops);
static uint8_t Tx_TrySendSmallPacket (_TxOps *ops);
static void    Tx_SyncSendPacket     (_TxOps *ops, uint8_t* Buf, uint16_t Len);
static uint8_t Tx_HandleSend         (_TxOps *ops, uint8_t* Buf, uint16_t Len);

static void    WaitTx (void);

// Register operations in inventory

// USB receive operations

static RxData TheRxData =
{
	0,     /* ExpectedByteCount */
	0,     /* TransferByteCount */
	true,  /* TransferFinished */
	true,  /* PacketReceived */
	false  /* BufferOverflow */
};

_RxOps TheRxOps =
{
	&TheRxData,
	Rx_InitTransfer,
	Rx_SetReceiveBuffer,
	Rx_StartTransfer,
	Rx_isTransferFinished,
	Rx_isPacketReceived,
	Rx_TryReceivePacket,
	Rx_HandleReceive,
	Rx_isBufferOverflow
};

// USB send operations

static TxData TheTxData =
{
	0 /* TxByteCounter */
};

_TxOps TheTxOps =
{
	&TheTxData,
	Tx_InitTransfer,
	Tx_SetTransferBuffer,
	Tx_TrySendPacket,
	Tx_TrySendSmallPacket,
	Tx_SyncSendPacket,
	Tx_HandleSend
};

// Implementation

/**
  * @brief  Rx_InitTransfer
  *         Init a new transfer without actually starting it.
  *         A receive buffer must be set before this function is called.
  * @param  ops: Pointer to a _RxOps structure to be initialized.
  * @param  ExpectedByteCount: Expected number of bytes to be received.
  * @retval -
  */
static void Rx_InitTransfer(_RxOps *ops, uint32_t ExpectedByteCount)
{
	// A transfer may be started with a non-empty receive buffer,
	// e.g. when the receive buffer is filled in multiple steps.
	// In other cases a non-empty receive buffer may *not* be ok,
	// hence we send a debug info about this.
	if (RxBuffer->Entries(RxBuffer) > 0)
	{
		xuac->Debug->dbg(DBGL_XFER,"Rx_InitTransfer: RxBuffer not empty!\r\n");
	}

	if (!(ops->data->TransferFinished))
	{
		// The previous transfer was not finished.
		// Packets from last transfer may arrive unexpectedly.
		// We send a debug info about this.
		xuac->Debug->dbg(DBGL_XFER,"Rx_InitTransfer: Previous transfer NOT finished!\r\n");
	}

	// Init variables needed for transfer management.

	// A transfer is finished when the expected amount of data was received.
	ops->data->ExpectedByteCount = ExpectedByteCount;

	// Reset counter of received data bytes, will later be compared against
	// expected amount above.
	ops->data->TransferByteCount = 0;

	// Reset flag. Transfer is not yet finished.
	ops->data->TransferFinished = false;

	// Reset flag. A packet was not yet received.
	// - TryReceivePacket() will prepare the OUT EP to receive a new packet
	//   only if a packet was received before.
	// - At device initialization the OUT EP is prepared to receive a packet
	//   in USBD_CDC_Init().
	ops->data->PacketReceived = false;

	// Reset flag. No buffer overflow detected so far.
	ops->data->BufferOverflow = false;
}


/**
  * @brief  Tx_InitTransfer
  *         Init a new transfer without actually starting it.
  * @param  ops: Pointer to a _TxOps structure to be initialized.
  * @retval -
  */
static void Tx_InitTransfer(_TxOps *ops)
{
	// Reset counter of sent data bytes.
	ops->data->TxByteCounter = 0;
}


/**
  * @brief  Rx_SetReceiveBuffer
  *         Register a new receive buffer for the next USB bulk transfer.
  *         Different receive buffers are necessary because e.g. we need to
  *         receive USB bulk requests in one receive buffer while caching
  *         submitted data in another receive buffer.
  * @param  ops: Pointer to _RxOps structure associated with the transfer.
  * @param  NewRxBuffer: Pointer to a new receive buffer (UserBuffer structure).
  * @retval -
  */
static void Rx_SetReceiveBuffer(_RxOps *ops, UserBuffer *NewRxBuffer)
{
	// Register the new receive buffer in the global variable.
	RxBuffer = NewRxBuffer;

	// Reset flag. No buffer overflow detected so far.
	ops->data->BufferOverflow = false;

	//xuac->Debug->dbg(DBGL_XFER,"Rx_SetReceiveBuffer: @RxBuffer=0x%x\r\n", RxBuffer);
}


/**
  * @brief  Rx_StartTransfer
  *         Initialize and start a new USB bulk transfer to receive data.
  * @param  ops: Pointer to _RxOps structure associated with the transfer.
  * @param  ExpectedByteCount: Expected number of bytes to be received.
  * @param  BufferAvailCheck: Before preparing an OUT EP to receive a packet
  *         we can make sure the selected receive buffer can take it.
  * @retval -
  */
static void Rx_StartTransfer(_RxOps *ops, uint32_t ExpectedByteCount, bool BufferAvailCheck)
{
	// Init variables needed for transfer management.
	ops->InitTransfer(ops, ExpectedByteCount);

	// Prepare OUT EP to receive a packet.
	// TryReceivePacket() does this only if a packet was received,
	// hence we need to set the flag to 'true' here when we start
	// a new transfer.
	ops->data->PacketReceived = true;
	ops->TryReceivePacket(ops, BufferAvailCheck);
}


/**
  * @brief  Rx_isTransferFinished
  *         Check if a USB bulk transfer (receive data) is flagged as finished.
  * @param  ops: Pointer to _RxOps structure associated with the transfer.
  * @retval TRUE if the USB bulk transfer (receive data) is flagged as finished,
  *         FALSE otherwise.
  */
static bool Rx_isTransferFinished(_RxOps *ops)
{
	return ops->data->TransferFinished;
}


/**
  * @brief  Rx_isPacketReceived
  *         Check if a USB packet was received.
  * @param  ops: Pointer to _RxOps structure associated with the transfer.
  * @retval TRUE if a USB packet was received,
  *         FALSE otherwise.
  */
static bool Rx_isPacketReceived(_RxOps *ops)
{
	return ops->data->PacketReceived;
}


/**
  * @brief  Rx_TryReceivePacket
  *         Prepare OUT EP to receive a packet if a few conditions are met:
  *         - OUT EP is not already prepared to receive a packet,
  *         - transfer is not already finished,
  *         - the selected receive buffer can take a packet.
  * @param  ops: Pointer to a _RxOps structure that is to be used.
  * @retval -
  */
static void Rx_TryReceivePacket(_RxOps *ops, bool BufferAvailCheck)
{
	if (!(ops->isPacketReceived(ops)))
	{
		// No packet received so far,
		// hence OUT EP is already prepared to receive a packet.
		return;
	}

	if (ops->isTransferFinished(ops))
	{
		// Transfer already finished.
		// No more packets expected for this transfer.
		return;
	}

	if (BufferAvailCheck)
	{
		// If BufferAvailCheck is TRUE:
		// Before preparing an EP to receive a packet we make
		// sure the selected receive buffer can take it.
		if (USB_FS_MAX_PACKET_SIZE > RxBuffer->Avail(RxBuffer))
		{
			// Not enough space available for a packet.
			return;
		}
	}

	// Set flag: No packet received so far (=FALSE).
	// Needs to be set *before* the OUT EP is prepared below.
	ops->data->PacketReceived = false;

	// Prepare OUT EP to receive a packet (->HandleReceive()).
	USBD_CDC_ReceivePacket(&hUsbDeviceFS);
}


//volatile uint32_t times = 0;

/**
  * @brief  Rx_HandleReceive
  *         We received an USB packet, now we need to handle it
  *         - Usually data will be copied to registered RxBuffer.
  *         - Unexpected packets will be discarded.
  *         - Transfer will be marked as finished if no data (ZLP) or
  *           less than a full packet was received, or all expected data
  *           was received (ExpectedByteCount).
  * @param  ops: Pointer to _RxOps structure associated with the transfer.
  * @param  Buf: Pointer to buffer containing the packet data.
  * @param  Len: Pointer to number of data bytes in the packet.
  * @retval Always USBD_OK.
  */
static int8_t Rx_HandleReceive(_RxOps *ops, uint8_t* Buf, uint32_t *Len)
{
//	xuac->Debug->dbg(DBGL_XFER,"Rx_HandleReceive(): Len=%u\r\n", *Len);

	// Set flag: We received a packet.
	// Note: Rx_TryReceivePacket() waits for this flag.
	ops->data->PacketReceived = true;

	if (ops->isTransferFinished(ops))
	{
		// Unexpected packet received: transfer is already finished.
		// This should not happen.
		xuac->Debug->dbg(DBGL_XFER,"HandleReceive(): Transfer already finished!\r\n");

		// Current behaviour: Discard packet.
		return (USBD_OK);
	}

	if (*Len == 0)
	{
		// We received a ZLP. Mark transfer as finished.
		xuac->Debug->dbg(DBGL_XFER,"HandleReceive(): Received ZLP.\r\n");
		ops->data->TransferFinished = true;
		return (USBD_OK);
	}

	if (*Len > RxBuffer->Avail(RxBuffer))
	{
		// Buffer overflow error. Send debug message.
		xuac->Debug->dbg(DBGL_XFER,"HandleReceive(): RxBuffer full error.\r\n");

		// Set overflow flag, count (below) & discard data.
		ops->data->BufferOverflow = true;
	}
	else
	{
		// Store packet data in selected receive buffer.
		// Need to use safe PushN operation, because USB IRQ has low priority 1
		// and may be interrupted.
		RxBuffer->PushN_safe(RxBuffer, Buf, *Len);
	}

	//xuac->Debug->dbg(DBGL_XFER,"<0>%d -> 0x%x [CB=0x%x], @buf=0x%x:0x%x\r\n", *Len, RxBuffer, xuac->Memory->CommandBuffer, RxBuffer->data->buffer, xuac->Memory->CommandBuffer->data->buffer);
	//xuac->Debug->dbg(DBGL_XFER,"<1>#RxBuffer=%d\r\n", RxBuffer->Entries(RxBuffer));

	// Count number of transferred bytes. We need to know when all data was
	// transferred and the transfer is finished.
	ops->data->TransferByteCount += *Len;

	/*if (RxBuffer != NULL)
	{
		if (++times%1000==0) xuac->Debug->dbg(DBGL_XFER,"HandleReceive(): [%u:%u:%u]\r\n", *Len, RxBuffer->Entries(RxBuffer), ops->data->RxByteCounter);
	}*/

	if ((*Len < USB_FS_MAX_PACKET_SIZE)
	   || (ops->data->TransferByteCount == ops->data->ExpectedByteCount))
	{
		// Received a small packet or all expected data.
		// Mark transfer as finished.
		//xuac->Debug->dbg(1,"HandleReceive(): Transfer finished.\r\n");
		ops->data->TransferFinished = true;
	}

	return (USBD_OK);
}


/**
  * @brief  Rx_isBufferOverflow
  *         Check if a RxBuffer overflow occurred in the current USB transfer
  *         (not enough space in RxBuffer for a packet that was received).
  * @param  ops: Pointer to _RxOps structure associated with the transfer.
  * @retval TRUE if RxBuffer overflowed, FALSE otherwise.
  */
static bool Rx_isBufferOverflow(_RxOps *ops)
{
	return ops->data->BufferOverflow;
}


/**
  * @brief  Tx_SetTransferBuffer
  *         Register a new transfer buffer for the next USB bulk transfer.
  * @param  ops: Pointer to _TxOps structure associated with the transfer.
  * @param  NewTxBuffer: Pointer to a new transfer buffer (UserBuffer structure).
  * @retval -
  */
static void Tx_SetTransferBuffer(_TxOps *ops, UserBuffer *NewTxBuffer)
{
	TxBuffer = NewTxBuffer;
}


/**
  * @brief  Tx_TrySendPacket
  *         Prepare IN EP to send a full packet if a few conditions are met:
  *         - ready to send a packet (last transfer finished, hence UserTxBufferFS
  *           is safe to reuse),
  *         - enough data in TxBuffer for a full packet.
  *
  *         @note
  *         This routine is non-blocking.
  *
  * @param  ops: Pointer to _TxOps structure associated with the transfer.
  * @retval Tx_Status_OK: if IN EP was prepared to send a packet.
  *         Tx_Status_Not_Ready: if not ready to send a packet (last transfer not finished).
  *         Tx_Status_Too_Few_Data: if not enough data in TxBuffer for a full packet.
  */
static uint8_t Tx_TrySendPacket(_TxOps *ops)
{
	// Ready to send?
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState != 0) { return Tx_Status_Not_Ready; }

	// Enough data bytes for a full packet in TxBuffer?
	if (TxBuffer->Entries(TxBuffer) < USB_FS_MAX_PACKET_SIZE) { return Tx_Status_Too_Few_Data; }

	// Copy data: TxBuffer -> UserTxBufferFS
	TxBuffer->PopN_safe(TxBuffer, UserTxBufferFS, USB_FS_MAX_PACKET_SIZE);

	// Send data asynchronously (->Tx_HandleSend()).
	CDC_Transmit_FS(UserTxBufferFS, USB_FS_MAX_PACKET_SIZE);

	return Tx_Status_OK;
}


/**
  * @brief  Tx_TrySendSmallPacket
  *         Try to (asynchronously) send a small USB packet (prepare IN EP
  *         to send the packet) and copy data from registered TxBuffer to
  *         USB CDC buffer if a few conditions are met:
  *         - ready to send a packet (last transfer finished, hence UserTxBufferFS
  *           is safe to reuse),
  *         - enough data in TxBuffer for a full packet.
  *
  *         @note
  *         Sending a small packet will finish the USB transfer.
  *
  *         @note
  *         This routine is non-blocking.
  *
  * @param  ops: Pointer to _TxOps structure associated with the transfer.
  * @retval Tx_Status_OK: if IN EP was prepared to send a packet.
  *         Tx_Status_Not_Ready: if not ready to send a packet (last transfer not finished).
  *         Tx_Status_Too_Much_Data: if too much data in TxBuffer (full packet or more).
  */
static uint8_t Tx_TrySendSmallPacket(_TxOps *ops)
{
	// Ready to send?
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	if (hcdc->TxState != 0)
	{
		return Tx_Status_Not_Ready;
	}

	// Return if too many data bytes in TxBuffer.
	if (TxBuffer->Entries(TxBuffer) >= USB_FS_MAX_PACKET_SIZE)
	{
		return Tx_Status_Too_Much_Data;
	}

	// Copy data: TxBuffer -> UserTxBufferFS (used by USB CDC)
	uint16_t Len = (uint16_t) TxBuffer->Entries(TxBuffer);
	if (Len > 0)
	{
		TxBuffer->PopN_safe(TxBuffer, UserTxBufferFS, Len);
	}

	// Send data asynchronously (->Tx_HandleSend()).
	CDC_Transmit_FS(UserTxBufferFS, Len);

	return Tx_Status_OK;
}


/**
  * @brief  WaitTx
  *         Internal routine. Called by Tx_SyncSendPacket() while waiting
  *         until a transfer is possible and until the transfer is finished.
  *         Empty routine, may e.g. blink status LED, reset watchdog,
  *         or output a debug message.
  * @retval -
  */
static void WaitTx(void)
{
	//xuac->Debug->dbg(DBGL_XFER,"WaitTx()\r\n");
	//HAL_GPIO_TogglePin(PORT_USER_LED, PIN_USER_LED);
	//HAL_Delay(50);
}


/**
  * @brief  Tx_SyncSendPacket
  *         Try to (synchronously) send a USB packet (prepare IN EP to send
  *         the packet) with the data supplied in 'Buf'.
  *
  *         @note
  *         Sending a small packet will finish the USB transfer.
  *
  *         @note
  *         Blocks until transfer possible and blocks until transfer finished.
  *
  * @param  ops: Pointer to _TxOps structure associated with the transfer.
  * @param  Buf: Pointer to buffer containing the packet data.
  * @param  Len: Number of data bytes to be sent.
  * @retval -
  */
static void Tx_SyncSendPacket(_TxOps *ops, uint8_t* Buf, uint16_t Len)
{
	// Ready to send?
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	while (hcdc->TxState) { WaitTx(); }

	// Send the packet (->Tx_HandleSend()).
	CDC_Transmit_FS(Buf, Len);

	// Make sure data is sent.
	while (hcdc->TxState) { WaitTx(); }
}

//uint32_t timesSend = 0;

/**
  * @brief  Tx_HandleSend
  *         Send a USB packet (prepare IN EP to send the packet) with the
  *         data supplied in 'Buf'.
  *
  *         @note
  *         This routine is only called from CDC_Transmit_FS().
  *
  * @param  ops: Pointer to _RxOps structure associated with the transfer.
  * @param  Buf: Pointer to buffer containing the packet data.
  * @param  Len: Pointer to number of data bytes in the packet.
  * @retval Result of the underlying USB CDC routine.
  */
static uint8_t Tx_HandleSend(_TxOps *ops, uint8_t* Buf, uint16_t Len)
{
	uint8_t result = USBD_OK;

	ops->data->TxByteCounter += Len;
	//if (++timesSend%1000==0) xuac->Debug->dbg(DBGL_XFER,"HandleSend(): [%u:%u:%u]\r\n", Len, TxBuffer->Entries(TxBuffer), ops->data->TxByteCounter);

	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
	result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);

	return result;
}
