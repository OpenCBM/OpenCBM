/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __BOARD_CAPS_H_
#define __BOARD_CAPS_H_

// Possible board capabilities
//#define BOARD_CAPABILITY_DISK_CBM     (uint32_t) (1 << 0) // disk: CBM commands
//#define BOARD_CAPABILITY_DISK_NIB_PAR (uint32_t) (1 << 1) // disk: parallel nibbler
//#define BOARD_CAPABILITY_DISK_NIB_SRQ (uint32_t) (1 << 2) // disk: 1571 serial nibbler
//#define BOARD_CAPABILITY_DISK_IEEE488 (uint32_t) (1 << 3) // disk: GPIB (PET) parallel bus
#define BOARD_CAPABILITY_TAPE         (uint32_t) (1 << 4) // 153x tape support

#endif /* __BOARD_CAPS_H_ */
