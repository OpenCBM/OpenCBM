/* Name: oddebug.c
 * Project: AVR library
 * Author: Christian Starkjohann
 * Creation Date: 2005-01-16
 * Tabsize: 4
 * Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: Proprietary, free under certain conditions. See Documentation.
 * This Revision: $Id: oddebug.c,v 1.1 2007-02-18 19:47:31 harbaum Exp $
 */

#include "iarcompat.h"
#ifndef __IAR_SYSTEMS_ICC__
#   include <avr/io.h>
#endif
#include "oddebug.h"

#if DEBUG_LEVEL > 0

#warning "Debugging is turned on! Never compile production devices with debugging!"

static void uartPutc(char c)
{
    while(!(ODDBG_USR & (1 << ODDBG_UDRE)));    /* wait for data register empty */
    ODDBG_UDR = c;
}

static uchar    hexAscii(uchar h)
{
    h &= 0xf;
    if(h >= 10)
        h += 'a' - (uchar)10 - '0';
    h += '0';
    return h;
}

static void printHex(uchar c)
{
    uartPutc(hexAscii(c >> 4));
    uartPutc(hexAscii(c));
}

void    odDebug(uchar prefix, uchar *data, uchar len)
{
    printHex(prefix);
    uartPutc(':');
    while(len--){
        uartPutc(' ');
        printHex(*data++);
    }
    uartPutc('\r');
    uartPutc('\n');
}

#endif
