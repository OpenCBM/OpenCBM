/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "usbd_ioreq.h"
#include "usbd_cdc.h" // for CDC_OUT_EP

#include "ctlreq.h"
#include "xuac.h" // the inventory

// Forward declarations

static uint8_t SetFeature   (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t ClearFeature (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t SetAddress   (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

// Register operations in inventory

CtlReq_Operations CtlReqOps =
{
	SetFeature,
	ClearFeature,
	SetAddress
};

// Implementation

static uint8_t SetFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	if ((req->bmRequest == 0x20) // USB_REQ_TYPE_CLASS (0x20) | "USB_EP_DIR_OUT" (0x00)
		&& (req->bRequest == 3)  // USB_REQ_SET_FEATURE // "XUAC_SHUTDOWN"
		&& (req->wValue == 0)
		&& (req->wIndex == 0)
		&& (req->wLength == 0))
	{
		//xuac->Debug->dbg(DBGL_CTLREQ,"SetFeature: XUAC_SHUTDOWN -----------------------\r\n");

		USBD_CtlSendStatus(pdev);

		return CtlReq_Handled;
	}

	return CtlReq_Not_Handled;
}

static uint8_t ClearFeature(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	if ((req->bmRequest == 2) // USB_REQ_RECIPIENT_ENDPOINT
		&& (req->bRequest == 1)  // USB_REQ_CLEAR_FEATURE
		&& (req->wValue == 0)    // USB_FEATURE_EP_HALT
		&& (req->wIndex == CDC_OUT_EP) // CDC_OUT_EP
		&& (req->wLength == 0))
	{
		uint8_t ep_addr = LOBYTE(req->wIndex);

		//xuac->Debug->dbg(DBGL_CTLREQ,"__ClearStallEP(%u) --------------------------\r\n", ep_addr);

		USBD_LL_ClearStallEP(pdev, ep_addr);
		pdev->pClass->Setup(pdev, req);
		USBD_CtlSendStatus(pdev);

		return CtlReq_Handled;
	}

	if ((req->bmRequest == 0xA0) // USB_REQ_TYPE_CLASS (0x20) | "USB_EP_DIR_IN" (0x80)
		&& (req->bRequest == 1)  // USB_REQ_CLEAR_FEATURE // "XUAC_INIT"
		&& (req->wValue == 0)
		&& (req->wIndex == 0)
		&& (req->wLength == 8))
	{
		//xuac->Debug->dbg(DBGL_CTLREQ,"ClearFeature: XUAC_INIT -----------------------\r\n");

		uint8_t replyBuf[8]; // XUAC_DEVINFO_SIZE=8
		memset(replyBuf, 0, sizeof(replyBuf));
		replyBuf[0] = 7; // XUAC_VERSION=7
		replyBuf[1] = (uint8_t) xuac->Board_HW->info->Board_Capabilities; // 32bit value

		USBD_CtlSendData(pdev, replyBuf, sizeof(replyBuf));

		return CtlReq_Handled;
	}

	return CtlReq_Not_Handled;
}

static uint8_t SetAddress(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
	if ((req->bmRequest == 0x20) // USB_REQ_TYPE_CLASS (0x20) | "USB_EP_DIR_OUT" (0x00)
		&& (req->bRequest == 5)  // USB_REQ_SET_ADDRESS // "XUAC_TAP_BREAK"
		&& (req->wValue == 0)    // USB_FEATURE_EP_HALT
		&& (req->wIndex == 0)
		&& (req->wLength == 0))
	{
		//xuac->Debug->dbg(DBGL_CTLREQ,"__Tape break -------------------------------\r\n", CDC_OUT_EP);

		uint8_t res = xuac->TapeOps->Tape_Reset();
		if (res == 1)
		{
			//xuac->Debug->dbg(DBGL_CTLREQ,"__StallEP(%u) -------------------------------\r\n", CDC_OUT_EP);
			USBD_LL_StallEP(pdev, CDC_OUT_EP);
		}

		USBD_CtlSendStatus(pdev);

		return CtlReq_Handled;
	}

	return CtlReq_Not_Handled;
}
