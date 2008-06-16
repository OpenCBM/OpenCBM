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
** \version $Id: configuration_name.c,v 1.5 2008-06-16 19:24:26 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**        Read configuration file
**
****************************************************************/

#include "configuration.h"
#include "libmisc.h"

#include <assert.h>
#include <string.h>

#ifdef OPENCBM_MAC
# define DEFAULT_FILENAME PREFIX "/etc/opencbm.conf"
#else
# define DEFAULT_FILENAME "/etc/opencbm.conf"
#endif


/*! \brief \internal Get the default filename for the configuration file

 Get the default filename of the configuration file.

 \return 
   Returns a newly allocated memory area with the default file name.
*/
const char *
configuration_get_default_filename(void)
{
    return cbmlibmisc_strdup(DEFAULT_FILENAME);
}
