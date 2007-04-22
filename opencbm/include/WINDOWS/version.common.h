/* $Id: version.common.h,v 1.4 2007-04-22 10:32:35 strik Exp $ */

#include "version.h"

#undef VER_PRODUCTNAME_STR
#undef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION_STR
#undef VER_COMPANYNAME_STR

#define VER_LEGALCOPYRIGHT_STR      "(c) 2001-2007 Spiro Trikaliotis, based on cbm4linux, (c) 1999-2005 by Michael Klein, and others"
#define VER_COMPANYNAME_STR         "(Spiro Trikaliotis, private)"

#define VER_PRODUCTVERSION          OPENCBM_VERSION_MAJOR,OPENCBM_VERSION_MINOR,OPENCBM_VERSION_SUBMINOR,OPENCBM_VERSION_DEVEL
#define VER_FILEVERSION             VER_PRODUCTVERSION
#define VER_PRODUCTVERSION_STR      OPENCBM_VERSION_STRING
#define VER_FILEVERSION_STR         VER_PRODUCTVERSION_STR
#define VER_LANGNEUTRAL
#define VER_PRODUCTNAME_STR         "OpenCBM - Accessing CBM drives from Windows"
