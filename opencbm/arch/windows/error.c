/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis <cbm4win@trikaliotis.net>
 *
 */

#include <windows.h>

#include <stdarg.h>
#include <stdio.h>

#include "arch.h"


/*! \brief Format a returned error code to a string

 This function formats a returned error code into a string.

 \param ErrorCode
   The error number to be formatted.

 \return
   Returns a pointer to the string describing the ErrorCode.

 The buffer the return value points to can be safely used until
 another call to this function occurs.
*/

char *arch_strerror(unsigned int ErrorCode)
{
    static char ErrorMessageBuffer[2048];
    int n;

    // Write the error message into the buffer

    n = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
                      | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                      NULL,
                      ErrorCode,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                      (LPTSTR) &ErrorMessageBuffer,
                      sizeof(ErrorMessageBuffer)-1,
                      NULL);

    // make sure there is a trailing zero

    ErrorMessageBuffer[n] = 0;

    return ErrorMessageBuffer;
}

/*! \brief Format a returned error code to screen

 This function formats a returned error code into a string,
 and outputs it onto the screen.

 \param AUnused
   Unused, only for compatibility with Unix

 \param ErrorCode
   The error number to be formatted.

 \param Format
   Format specifier for additional error information.
   Can be NULL.
*/

void arch_error(int AUnused, unsigned int ErrorCode, const char *Format, ...)
{
    va_list ap;
    char ErrorMessageBuffer[2048];
    int m;
    int n;

    UNREFERENCED_PARAMETER(AUnused);

    // Write the error message into the buffer

    m = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
                      | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                      NULL,
                      ErrorCode,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                      (LPTSTR) ErrorMessageBuffer,
                      sizeof(ErrorMessageBuffer)-1,
                      NULL);

    // make sure there is a trailing zero

    ErrorMessageBuffer[m] = 0;


    // Now, append the optional string to the output

    if (Format && *Format)
    {
        va_start(ap, Format);

        n = _vsnprintf(&ErrorMessageBuffer[m], sizeof(ErrorMessageBuffer) - m, Format, ap);

        va_end(ap);

        if (n >= 0)
        {
            n += m;
        }

        if (n < 0 || n >= sizeof(ErrorMessageBuffer)-1)
        {
            n = sizeof(ErrorMessageBuffer)-2;
        }

        // make sure there is a trailing zero

        ErrorMessageBuffer[n] = 0;

        fprintf(stderr, "\nAUnused = %u, ErrorCode = %u: %s\n", AUnused, ErrorCode, ErrorMessageBuffer);

#if DBG

        ErrorMessageBuffer[n] = '\n';
        ErrorMessageBuffer[n+1] = 0;
        OutputDebugString(ErrorMessageBuffer);

#endif

    }
}
