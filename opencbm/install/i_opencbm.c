/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2001-2004 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file install/i_opencbm.c \n
** \author Spiro Trikaliotis \n
** \version $Id: i_opencbm.c,v 1.1 2004-11-07 11:05:12 strik Exp $ \n
** \n
** \brief Functions for accessing the driver
**
****************************************************************/

/*! The name of the executable */
#ifndef DBG_PROGNAME
    #define DBG_PROGNAME "INSTCBM.EXE"
#endif // #ifndef DBG_PROGNAME

/*! Mark: We are in user-space (for debug.h) */
// #define DBG_USERMODE

// Include the functionality of the DLL into the install
// Application

#include "../dll/i_opencbm.c"
