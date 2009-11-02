/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005-2007 Spiro Trikaliotis
 */

/*! **************************************************************
** \file include/WINDOWS/version.common.h \n
** \author Spiro Trikaliotis \n
** \version $Id: version.common.h,v 1.3.4.1 2009-11-02 21:07:14 fbriere Exp $ \n
** \n
** \brief Defining file version information for all OpenCBM tools
**
****************************************************************/

/* $Id: version.common.h,v 1.3.4.1 2009-11-02 21:07:14 fbriere Exp $ */

#include "version.h"

#undef VER_PRODUCTNAME_STR
#undef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION_STR
#undef VER_COMPANYNAME_STR

#define VER_LEGALCOPYRIGHT_STR      "(c) 2001-2006 Spiro Trikaliotis, based on cbm4linux, (c) 1999-2005 by Michael Klein, and others"
#define VER_COMPANYNAME_STR         "(Spiro Trikaliotis, private)"

#define VER_PRODUCTVERSION          CBM4WIN_VERSION_MAJOR,CBM4WIN_VERSION_MINOR,CBM4WIN_VERSION_SUBMINOR,CBM4WIN_VERSION_DEVEL
#define VER_FILEVERSION             VER_PRODUCTVERSION
#define VER_PRODUCTVERSION_STR      CBM4WIN_VERSION_STRING
#define VER_FILEVERSION_STR         VER_PRODUCTVERSION_STR
#define VER_LANGNEUTRAL
#define VER_PRODUCTNAME_STR         "CBM4WIN - Accessing CBM drives from Windows"
