/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007,2008 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINBUILD/configuration_name.c \n
** \author Spiro Trikaliotis \n
** \version $Id: configuration_name.c,v 1.5 2008-10-09 17:14:26 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**        Read configuration file
**
****************************************************************/

#include "configuration.h"
#include "libmisc.h"
#include "version.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

/*! \brief @@@@@ \todo document */
#define DEFAULT_FILENAME "/System32/opencbm.conf"


/*! \brief Get the default filename for the configuration file

 Get the default filename of the configuration file.

 \return 
   Returns a newly allocated memory area with the default file name.
*/
const char *
configuration_get_default_filename(void)
{
    DWORD length;
    char * buffer = NULL;

    static const char addendum[] = DEFAULT_FILENAME;

    length = GetEnvironmentVariable("WINDIR", NULL, 0);

#ifdef USE_FAKE_WIN_DIRECTORY_AS_COPY_TARGET
    buffer = cbmlibmisc_strdup(USE_FAKE_WIN_DIRECTORY_AS_COPY_TARGET "\\");
#else
    while (length > 0) {

        DWORD length2;

        free(buffer);
        buffer = malloc(length + sizeof(addendum));

        length2 = GetEnvironmentVariable("WINDIR", buffer, length + 1);

        if (length2 > length) {
            length = length2;
        }
        else {
            length = 0;
        }
    }
#endif

    if (buffer) {
        char * tmpbuffer = cbmlibmisc_strcat(buffer, addendum);

        free(buffer);

        buffer = tmpbuffer;
    }

    return buffer;
}
