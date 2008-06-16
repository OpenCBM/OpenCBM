/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 2001-2005,2007,2008 Spiro Trikaliotis
 *
*/

/*! ************************************************************** 
** \file lib/WINBUILD/archlib.c \n
** \author Michael Klein, Spiro Trikaliotis \n
** \version $Id: archlib.c,v 1.18 2008-06-16 19:24:26 strik Exp $ \n
** \n
** \brief Shared library / DLL for accessing the driver, windows specific code
**
****************************************************************/

#include <windows.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! Mark: We are building the DLL */
#define DBG_DLL

/*! The name of the executable */
#define DBG_PROGNAME "OPENCBM.DLL"

/*! This file is "like" debug.c, that is, define some variables */
#define DBG_IS_DEBUG_C

#include "configuration.h"
#include "debug.h"


/*! mark: We are building the DLL */
#define DLL
#include "i_opencbm.h"

/*! mark: we are exporting plugin entries */
#define OPENCBM_PLUGIN 1
#include "archlib.h"
#include "libmisc.h"

/*! \brief DLL initialization und unloading

 This function is called whenever the DLL is loaded or unloaded.
 It ensures that the driver is loaded to be able to call its
 functions.

 \param Module
   Handle of the module; this is not used.

 \param Reason
   DLL_PROCESS_ATTACH if the DLL is loaded,
   DLL_PROCESS_DETACH if it is unloaded.

 \param Reserved
   Not used.

 \return 
   Returns TRUE on success, else FALSE.

 If this function returns FALSE, windows reports that loading the DLL
 was not successful. If the DLL is linked statically, the executable
 refuses to load with STATUS_DLL_INIT_FAILED (0xC0000142)
*/

BOOL WINAPI
DllMain(IN HANDLE Module, IN DWORD Reason, IN LPVOID Reserved)
{
    static BOOL bIsOpen = FALSE;

    FUNC_ENTER();

#if DBG

    if (Reason == DLL_PROCESS_ATTACH)
    {
        // Read the debugging flags from the registry

//        cbm_get_debugging_flags();
    }

#endif

    FUNC_LEAVE_BOOL(TRUE);
}


BOOL CBMAPIDECL cbm_plugin_install_generic(const char * DefaultPluginname)
{
    BOOL error = TRUE;

    opencbm_configuration_handle configuration_handle;

    const char * configurationFilename = configuration_get_default_filename();

    FUNC_ENTER();

    do {
        /* create the INI file, if not present, and create the needed keys */

        if (configurationFilename == NULL) {
            break;
        }

        configuration_handle = opencbm_configuration_create(configurationFilename);

        if (configuration_handle == NULL) {
            break;
        }

        if (DefaultPluginname != NULL) {
            error = opencbm_configuration_set_data(configuration_handle, 
                       "plugins", "default", DefaultPluginname);
        }

        error = opencbm_configuration_close(configuration_handle) || error;

    } while (0);

    cbmlibmisc_strfree(configurationFilename);

    FUNC_LEAVE_BOOL(error);
}

BOOL CBMAPIDECL cbm_plugin_install_plugin_data(const char * Pluginname, const char * Filepath, const CbmPluginInstallProcessCommandlineData_t * CommandlineData)
{
    BOOL error = TRUE;

    opencbm_configuration_handle configuration_handle;

    const char * configurationFilename = configuration_get_default_filename();

    FUNC_ENTER();

    do {
        /* create the INI file, if not present, and create the needed keys */

        if (configurationFilename == NULL) {
            break;
        }

        configuration_handle = opencbm_configuration_open(configurationFilename);

        error = opencbm_configuration_set_data(configuration_handle, 
                   Pluginname, "location", Filepath);

        if (error == 0) {
            // call self-init
            CommandlineData->OptionMemory;
        }

        error = opencbm_configuration_close(configuration_handle) || error;

    } while (0);

    cbmlibmisc_strfree(configurationFilename);

    FUNC_LEAVE_BOOL(error);
}
