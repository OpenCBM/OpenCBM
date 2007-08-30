/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, version 
 *  2 of the License.
 *
 *  Copyright 2007 Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file flasher.h \n
** \author Spiro Trikaliotis \n
** \version $Id: flasher.h,v 1.2 2007-08-30 18:50:16 strik Exp $ \n
** \n
** \brief Flash the bootloader from the application space
**
****************************************************************/

#include "xu1541bios.h"

#define STATIC static

void spm_copy(uint8_t what, uint16_t address, uint16_t data);
void spm_end(void);
