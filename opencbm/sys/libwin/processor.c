/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005 Spiro Trikaliotis <cbm4win@trikaliotis.net>
 *
 */

/*! ************************************************************** 
** \file sys/libwin/processor.c \n
** \author Spiro Trikaliotis \n
** \version $Id: processor.c,v 1.1 2005-08-24 18:18:05 strik Exp $ \n
** \n
** \brief Functions for determining processor number
**
****************************************************************/

#include <ntddk.h>
#include "cbm_driver.h"

/*! \brief Wrapper for KeGetCurrentProcessorNumber()

 See KeGetCurrentProcessorNumber()
 
 This function is needed as KeGetCurrentProcessorNumber() is
 only defined in NTDDK.H, not in WDM.H. Anyway, for debugging
 purposes, we need to access it from anywhere..
*/
ULONG
CbmGetCurrentProcessorNumber(VOID)
{
#ifdef COMPILE_W98_API
    return 0;
#else
    return KeGetCurrentProcessorNumber();
#endif
}

ULONG
CbmGetNumberProcessors(VOID)
{
#ifdef COMPILE_W98_API

    return 1;

#elif COMPILE_W2K_API

    return *KeNumberProcessors;

#else

    return KeNumberProcessors;

#endif
}
