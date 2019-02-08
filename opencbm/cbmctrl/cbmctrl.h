/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2019 Spiro Trikaliotis
 */

/*! **************************************************************
** \file cbmctrl/cbmctrl.h \n
** \author Spiro Trikaliotis \n
** \n
** \brief Internal functions for cbmctrl
**
****************************************************************/

#ifndef CBMCTRL_H_
#define CBMCTRL_H_ 1

#include "opencbm.h"

int cbm_check_xp1541(CBM_FILE HandleDevice, unsigned char DeviceAddress, enum cbm_device_type_e CbmDeviceType, enum cbm_cable_type_e CableType, int verbose);

#endif /* #ifndef CBMCTRL_H_ */
