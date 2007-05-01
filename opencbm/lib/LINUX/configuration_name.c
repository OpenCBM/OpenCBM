/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007      Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/LINUX/configuration_name.c \n
** \author Spiro Trikaliotis \n
** \version $Id: configuration_name.c,v 1.4 2007-05-01 17:51:38 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**        Read configuration file
**
****************************************************************/

#include "configuration.h"

#include <assert.h>
#include <string.h>

#ifdef OPENCBM_MAC
# define DEFAULT_FILENAME PREFIX "/etc/opencbm.conf"
#else
# define DEFAULT_FILENAME "/etc/opencbm.conf"
#endif

#define MAX_PATH_ 1024


/*! \brief \internal Get the default filename for the configuration file

 Get the default filename of the configuration file.

 \param Buffer
   Pointer to a buffer which will hold the filename.

 \param BufferLength
   The size of the buffer Buffer. It must be enough to hold the full
   filename, including the trailing zero.

 \return 
   Returns the input variable Buffer.
*/
const char *
configuration_get_default_filename(char Buffer[], unsigned int BufferLength)
{
    if (Buffer)
    {
        assert(sizeof(DEFAULT_FILENAME) <= BufferLength);
        strcpy(Buffer, DEFAULT_FILENAME);
    }

    return Buffer;
}
