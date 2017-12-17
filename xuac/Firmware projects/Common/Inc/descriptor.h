/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __DESCRIPTOR_H_
#define __DESCRIPTOR_H_

// ToDo: Need other VID/PID !?
#define USBD_VID						(uint16_t)  0x16d0
#define USBD_PID_FS						(uint16_t)  0x0504
#define USBD_LANGID_STRING				(uint16_t)  0x0409
#define USBD_MANUFACTURER_STRING		(uint8_t *) "Arnd Menge and OpenCBM team"
#define USBD_PRODUCT_STRING_FS			(uint8_t *) "xuac adapter (ZoomTape)"
#define USBD_SERIALNUMBER_STRING_FS		(uint8_t *) "000"
#define USBD_CONFIGURATION_STRING_FS	(uint8_t *) "Standard Configuration"
#define USBD_INTERFACE_STRING_FS		(uint8_t *) "Standard Interface"

#endif /* __DESCRIPTOR_H_ */
