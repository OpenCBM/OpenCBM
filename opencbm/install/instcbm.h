/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004, 2008 Spiro Trikaliotis
 *
 */

/*! **************************************************************
** \file instcbm.h \n
** \author Spiro Trikaliotis \n
** \version $Id: instcbm.h,v 1.11 2008-06-16 19:24:23 strik Exp $ \n
** \n
** \brief Header for installation routines
**
****************************************************************/

#ifndef INSTCBM_H
#define INSTCBM_H

extern char *get_plugin_filename(char *Plugin);

typedef struct cbm_install_parameter_s cbm_install_parameter_t;
typedef struct CbmPluginInstallProcessCommandlineData_s CbmPluginInstallProcessCommandlineData_t;
typedef struct cbm_install_parameter_plugin_s cbm_install_parameter_plugin_t;

extern BOOL get_all_plugins(cbm_install_parameter_t * InstallParameter);
extern BOOL ProcessPluginCommandline(const char * const Plugin, cbm_install_parameter_t * Parameter, int Argc, char * const Argv[]);

extern void PluginListFree(cbm_install_parameter_t * InstallParameter);

typedef BOOL PluginForAll_Callback_t(cbm_install_parameter_plugin_t * PluginInstallParameter, void * Context);

extern BOOL PluginForAll(cbm_install_parameter_t * InstallParameter, PluginForAll_Callback_t * Callback, void * Context);

#endif // #ifndef INSTCBM_H
