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
** \file lib/LINUX/getpluginaddress.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: getpluginaddress.c,v 1.1.2.1 2007-03-14 11:32:10 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**
****************************************************************/

#include "getpluginaddress.h"

#include <dlfcn.h>

SHARED_OBJECT_HANDLE 
plugin_load(const char * name)
{
    return dlopen(name, RTLD_NOW);
}

void *
plugin_get_address(SHARED_OBJECT_HANDLE handle, const char * name)
{
    return dlsym(handle, name);
}

void
plugin_unload(SHARED_OBJECT_HANDLE handle)
{
    dlclose(handle);
}
