/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007,2008,2012 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINDOWS/configuration_name.c \n
** \author Spiro Trikaliotis \n
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

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

/*! \brief The filename of the configuration file */
#define FILENAME_CONFIGFILE "opencbm.conf"


/*! \internal \brief Get a possible filename for the configuration file

 This function returns the possible filenames for the configuration
 file, one after the other.
 
 For this, the function is called with an index, starting with zero, 
 being incremented with every call. If the list is exhaused, the function
 returns NULL.

 \return 
   pointer to a newly allocated memory area which contains the
   candidate for the given index. If there is no candidate with
   the given index, the returned value is NULL.

 \remark
   There is not a single default filename for the configuration;
   instead, there are some locations that are searched one after
   the other. This function returns the candidates for these locations.

 \remark
   The caller is responsible for free()ing the returned buffer pointer.
*/
static char *
get_windir()
{
    char * buffer = NULL;

    DWORD length;

    length = GetEnvironmentVariable("WINDIR", NULL, 0);

    while (length > 0) {

        DWORD length2;

        free(buffer);
        buffer = malloc(length + 1);

        length2 = GetEnvironmentVariable("WINDIR", buffer, length + 1);

        if (length2 > length) {
            length = length2;
        }
        else {
            length = 0;
        }
    }
    return buffer;
}

static char *
configuration_get_default_filename_candidate(unsigned int index)
{
    char * buffer = NULL;

    switch (index)
    {
        case 0:
            buffer = cbmlibmisc_strdup(FILENAME_CONFIGFILE);
            break;
        case 1:
            {
            char * windir = get_windir();
            buffer = cbmlibmisc_strcat(windir, "/System32/" FILENAME_CONFIGFILE);
            free(windir);
            }
            break;
        default:
            /* no more candidates */
            buffer = NULL;
            break;
    }

    return buffer;
}

static int
FileExists(const char * filename)
{
    DWORD attrib = GetFileAttributes(filename);

    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

/*! \brief Get the default filename for the configuration file

 Get the default filename of the configuration file.

 \return 
   Returns a newly allocated memory area with the default file name.
*/
const char *
configuration_get_default_filename(void)
{
    unsigned int index = 0;
    char * string_candidate = NULL;

    while (NULL != (string_candidate = configuration_get_default_filename_candidate(index)))
    {
        if (FileExists(string_candidate)) {
            break;
        };

        free(string_candidate);
        ++index;
    };

    return string_candidate;
}

