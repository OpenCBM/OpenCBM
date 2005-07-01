/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINVICEBUILD/vice_comm.h \n
** \author Spiro Trikaliotis \n
** \version $Id: vice_comm.h,v 1.1 2005-07-01 12:22:16 strik Exp $ \n
** \n
** \brief Library interface for communicating with VICE.
**
****************************************************************/

#include <windows.h>

typedef
enum viceregs_e { reg_pc = 0, reg_a = 1, reg_x = 2, reg_y = 3, reg_sp = 4, reg_flags = 5 } viceregs;

extern int vicereadregister(viceregs which);
extern void vicewriteregister_when_at(int value);
extern void vicewriteregister(viceregs which, int value);
extern void vicewritememory(unsigned int address, int length, const char *buffer);
extern void vicereadmemory(unsigned int address, int length, char *buffer);
extern void vicepreparereadmemory(unsigned int address, int length);
extern void vicepause(void);
extern void viceresume(void);
extern BOOLEAN viceinit(void);
extern void vicerelease(void);
extern void vicereset(void);
extern void vicetrap(UINT address);
extern void vicewaittrap(void);
