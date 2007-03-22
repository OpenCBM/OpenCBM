/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005,2007 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINBUILD/getpluginaddress.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: getpluginaddress.c,v 1.2 2007-03-22 13:12:22 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**
****************************************************************/

#include "getpluginaddress.h"

SHARED_OBJECT_HANDLE
plugin_load(const char * name)
{
    return LoadLibrary(name);
}

void *
plugin_get_address(SHARED_OBJECT_HANDLE handle, const char * name)
{
    return GetProcAddress(handle, name);
}

void
plugin_unload(SHARED_OBJECT_HANDLE handle)
{
    FreeLibrary(handle);
}
