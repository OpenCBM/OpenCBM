/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Copyright (C) 2006 Wolfgang Moser (http://d81.de)
 */

#ifndef __CBMRPM41_HEADER_INCLUDE_
#define __CBMRPM41_HEADER_INCLUDE_

#include "opencbm.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>

#include "arch.h"

#define CBMRPM41_DEBUG      // enable state logging and debugging


    /*
     * The following are some really ugly macro definitions, so that a
     * structural storage declaration can be shared between the ca65
     * assembler and ANSI-C. The structural declaration is made within
     * the interface definition header file named cbmforng.idh, it
     * contains several macro calls which are defined here.
     */
#   define _CMT(str)
#   define _BEGINSTRUCT(parm)       struct parm {
#   define _ENDSTRUCT()             };
#   define _OCTETARRAY(name, size)  unsigned char name[size];
#   define _OCTETDECL(name)         unsigned char name;

#   define _BEGINENUM(parm)         enum parm {
#   define _BEGIN_UX_ENUM(parm)     enum parm { ResetUxVectorTable = '0',
#   define _ENDENUM()               };
#   define _ENUMENTRY(asym,no,name) name = no,

#   include  "cbmrpm41.idh"

#   undef _CMT
#   undef _BEGINSTRUCT
#   undef _ENDSTRUCT
#   undef _OCTETARRAY
#   undef _OCTETDECL

#   undef _BEGINENUM
#   undef _BEGIN_UX_ENUM
#   undef _ENDENUM
#   undef _ENUMENTRY
#endif
