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
** \file include/configuration.h \n
** \author Spiro Trikaliotis \n
** \version $Id: configuration.h,v 1.1 2008-06-16 19:24:22 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver
**        Read configuration file
**
****************************************************************/

struct opencbm_configuration_s;

typedef struct opencbm_configuration_s opencbm_configuration_t;
typedef opencbm_configuration_t *opencbm_configuration_handle;


extern const char *configuration_get_default_filename(void);

extern opencbm_configuration_handle opencbm_configuration_open(const char * Filename);
extern opencbm_configuration_handle opencbm_configuration_create(const char * Filename);
extern int                          opencbm_configuration_close(opencbm_configuration_handle Handle);
extern int                          opencbm_configuration_get_data(opencbm_configuration_handle Handle, const char Section[], const char Entry[], char ** ReturnBuffer);
extern int                          opencbm_configuration_set_data(opencbm_configuration_handle Handle, const char Section[], const char Entry[], const char Value[]);
