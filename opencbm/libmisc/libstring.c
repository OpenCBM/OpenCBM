/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file libmisc/libstring.c \n
** \author Spiro Trikaliotis \n
** \version $Id: libstring.c,v 1.1 2007-05-01 17:51:38 strik Exp $ \n
** \n
** \brief Helper function for string handling
**
****************************************************************/

#include "arch.h"

#include <stdlib.h>

char *
cbmlibmisc_strdup(const char * const OldString)
{
    char *newString = NULL;

    if (OldString)
    {
        int len = strlen(OldString) + 1;

        newString = malloc(len);

        if (newString)
            memcpy(newString, OldString, len);
    }
    return newString;
}

char *
cbmlibmisc_strndup(const char * const OldString, size_t Length)
{
    char * newString = NULL;

    if (OldString)
    {
        size_t len = strlen(OldString);

        len = len < Length ? len : Length;

        newString = malloc(len + 1);

        if (newString)
        {
            memcpy(newString, OldString, len);
            newString[len] = 0;
        }
    }
    return newString;
}

void
cbmlibmisc_strfree(char * String)
{
    if (String)
        free(String);
}

char *
cbmlibmisc_strcat(char * first, const char * second)
{
    char * ret = first;

    if (first != NULL && second != NULL)
    {
        int length_first  = strlen(first);
        int length_second = strlen(second);

        int length = length_first + length_second + 1;

        char *p = malloc(length);

        if (p != NULL)
        {
            memcpy(p, first, length_first);
            memcpy(p + length_first, second, length_second);
            p[length_first + length_second] = 0;
        }
    }

    return ret;
}
