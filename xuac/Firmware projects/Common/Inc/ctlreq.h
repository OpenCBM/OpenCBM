/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __CTLREQ_H_
#define __CTLREQ_H_

#include "usbd_def.h" // for USBD_HandleTypeDef, USBD_SetupReqTypedef
#include <stdint.h> // for uint8_t, uint16_t, uint32_t

#define CtlReq_Not_Handled 0
#define CtlReq_Handled     1

// Structure for the inventory

typedef struct
{
	uint8_t (*SetFeature)   (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
	uint8_t (*ClearFeature) (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
	uint8_t (*SetAddress)   (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
} CtlReq_Operations;

#endif /* __CTLREQ_H_ */
