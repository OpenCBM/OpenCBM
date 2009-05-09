/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2006 Spiro Trikaliotis
 */

/*! **************************************************************
** \file include/packon.h \n
** \author Spiro Trikaliotis \n
** \version $Id: packon.h,v 1.2 2009-05-09 17:42:21 strik Exp $ \n
** \n
** \brief Make sure that struct definitions are packed
**
** \comment The WDK compiler warns that this header changes
**          the packing after it ends. This is intentional,
**          and the whole purpose of this file!
****************************************************************/

#if (_MSC_VER >= 1200) // MSVC 6 or newer

 #pragma pack(push, packonoff, 1)

#elif (__GNUC__ >= 3)

 #pragma pack(1)

#else

 #pragma error "Unknown compiler!"

#endif
