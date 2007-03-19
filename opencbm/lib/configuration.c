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
** \file lib/configuration.c \n
** \author Spiro Trikaliotis \n
** \version $Id: configuration.c,v 1.1.2.2 2007-03-19 18:48:47 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**        Read configuration file
**
****************************************************************/

#include "configuration.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_PATH_ 1024

#define MAX_LINE_LENGTH 256


struct opencbm_configuration_s {
    FILE * File;
};

/*! \brief \internal Read a line of the configuration file

 Get the default filename of the configuration file.

 \param Handle
   Handle to the configuration file.

 \return 
   Returns a (static) buffer which hold the current line.

 \remark
   Comment lines (beginning with a '#') and comments at the end of a line are
   ignored. Additionally, SPACEs, TABs, CR and NL at the end of the line are
   ignored, too.

 \bug
   If a line is bigger than the maximum line length (MAX_LINE_LENGTH), it will
   be split up into portions of MAX_LINE_LENGTH.
*/
static
unsigned char *
configuration_read_line(opencbm_configuration_handle Handle)
{
    static unsigned char buffer[MAX_LINE_LENGTH];
    unsigned char *ret = NULL;

    do {
        /* If we already reached the end of file, abort here */

        if (feof(Handle->File))
            break;

        /* If we got an error, abort here */

        if (ferror(Handle->File))
            break;

        /* Read in a line */

        fgets(buffer, sizeof(buffer), Handle->File);

        /* If this is not a comment line, we finally found a line: */

        if (buffer[0] != '#')
        {
            unsigned char *p;

            ret = buffer;

            /* search for a comment a trim it if it exists */

            p = strchr(buffer, '#');

            if (p == NULL)
                p = buffer + strlen(buffer);

            while (p && (p > buffer))
            {
                *p = 0;

                /* trim any spaces from the right, if available */

                switch (*--p)
                {
                case ' ':
                case '\t':
                case 13:
                case 10:
                    break;

                default:
                    p = NULL;
                }
            }

            if (p == buffer)
                *p = 0;

            break;
        }

    } while (0);

    return ret;
}

/*! \brief Open the configuration file

 Opens the configuration file so it can be used later on with
 opencbm_configuration_get_data().

 \return
   Returns a handle to the configuration file which can be used
   in subsequent calls to the other configuration file functions.
*/
opencbm_configuration_handle
opencbm_configuration_open(void)
{
    opencbm_configuration_handle handle;

    do {
        unsigned char filenamebuffer[MAX_PATH_];

        handle = malloc(sizeof(*handle));

        if (!handle)
            break;

        memset(handle, 0, sizeof(*handle));

        handle->File = fopen(configuration_get_default_filename(filenamebuffer, sizeof(filenamebuffer)), "rt");

        if (handle->File == NULL)
            break;

    } while (0);

    return handle;
}


/*! \brief Close the configuration file

 Closes the configuration file after it has been used.

 \param Handle
   Handle to the opened configuration file, as obtained from
   opencbm_configuration_open().
*/
void
opencbm_configuration_close(opencbm_configuration_handle Handle)
{
    if (Handle)
    {
        if (Handle->File)
            fclose(Handle->File);

        free(Handle);
    }
}

/*! \brief Read data from the configuration file

 Closes the configuration file after it has been used.

 \param Handle
   Handle to the opened configuration file, as obtained from
   opencbm_configuration_open().
 
 \param Section
   A string which holds the name of the section from where to get the data.

 \param Entry
   A string which holds the name of the entry to get.

 \param ReturnBuffer
   A buffer which holds the return value on success. If the function returns
   with something different than 0, the buffer pointer to by ReturnBuffer will
   not be changed.
   Can be NULL if ReturnBufferLength is zero, too. Cf. note below.

 \param ReturnBufferLength
   The length of the buffer pointed to by ReturnBuffer.

 \return
   Returns 0 if the date entry was found. If ReturnBufferLength != 0, the
   return value is 0 only if the buffer was large enough to hold the data.

 \note
   If ReturnBufferLength is zero, this function only tests if the Entry exists
   in the given Section. In this case, this function returns 0; otherwise, it
   returns 1.
*/
int
opencbm_configuration_get_data(opencbm_configuration_handle Handle,
                               const unsigned char Section[], const unsigned char Entry[],
                               unsigned char ReturnBuffer[], unsigned int ReturnBufferLength)
{
    int error = 1;

    do
    {
        unsigned int found = 0;
        unsigned char *p;

        unsigned int section_length;
        unsigned int entry_length;
        unsigned int value_length;

        /* Check if there is a section and an entry given */

        if (Section == NULL || Entry == NULL)
            break;

        /* If the ReturnBuffer is null, its length has to be specified as null, too. */

        if (ReturnBuffer == NULL && ReturnBufferLength != 0)
            break;


        /* Precalculate the length of the given strings */

        section_length = strlen(Section);
        entry_length = strlen(Entry);

        /* First, check if we successfully opened the configuration file */

        if (Handle == NULL || Handle->File == NULL)
            break;

        /* Seek to the beginning of the file */

        fseek(Handle->File, 0, SEEK_SET);

        /* Search the right section */

        do {
            /* read one line */

            p = configuration_read_line(Handle);

            /* Check if we found the section */
            if (p && (p[0] == '[') && (strncmp(&p[1], Section, section_length) == 0) && (p[section_length + 1] == ']'))
            {
                found = 1;
            }

        } while (p && !found);

        /* If we did not find the section, abort now */

        if (!found)
            break;


        /* Search for the correct entry */

        found = 0;

        do {
            /* read one line */

            p = configuration_read_line(Handle);

            /* Check if we found the entry */
            if (p && (strncmp(p, Entry, entry_length) == 0) && (p[entry_length] == '='))
            {
                p = &p[entry_length + 1];

                found = 1;
            }

        } while (p && !found);

        /* If we did not find the entry, abort now */

        if (!found)
            break;

        /* If ReturnBufferLength is 0, we only wanted to find out if 
         * that entry existed. Thus, report "no error" and quit.
         */

        if (ReturnBufferLength == 0)
        {
            error = 0;
            break;
        }

        /*
         * Now, copy the data into the given pointer
         */

        /* Get the length of the data */

        value_length = strlen(p);

        /* Does the data fit into the ReturnBuffer? If not, abort */

        if (value_length > ReturnBufferLength)
            break;

        /* Copy the data into the given pointer */

        strcpy(ReturnBuffer, p);

        error = 0;

    } while(0);

    return error;
}
