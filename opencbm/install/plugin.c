/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2007,2008 Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file plugin.c \n
** \author Spiro Trikaliotis \n
** \version $Id: plugin.c,v 1.2 2008-09-01 18:41:50 strik Exp $ \n
** \n
** \brief Program to install and uninstall the OPENCBM driver; handling of plugins
**
****************************************************************/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include <getopt.h>

#include "archlib.h"
#include "cbmioctl.h"
#include "configuration.h"
#include "version.h"
#include "arch.h"
#include "i_opencbm.h"

#include "libmisc.h"

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "INSTCBM.EXE"

/*! This file is "like" debug.c, that is, define some variables */
#define DBG_IS_DEBUG_C

#include "debug.h"

#include "instcbm.h"

#include "opencbm-plugin.h"

#define PLUGIN_PREFIX "opencbm-"
#define PLUGIN_SUFFIX ".dll"

static char *
malloc_plugin_file_name(const char *plugin_name)
{
    char * filename = NULL;

    FUNC_ENTER();

    do {
        /*
         * remark: sizeof(PLUGIN_PREFIX) and sizeof(PLUGIN_SUFFIX) already contain the terminating null;
         * thus, there is no need to add 1 to this. In fact, we are already one char to long!
         */
        filename = malloc(sizeof(PLUGIN_PREFIX) + sizeof(PLUGIN_SUFFIX) + strlen(plugin_name));

        if (filename == NULL)
            break;

        strcpy(filename, PLUGIN_PREFIX);
        strcat(filename, plugin_name);
        strcat(filename, PLUGIN_SUFFIX);
    } while (0);

    FUNC_LEAVE_STRING(filename);
}

static BOOL
PluginAlreadyInList(const char * const PluginName, cbm_install_parameter_t * InstallParameter)
{
    BOOL exists = FALSE;
    cbm_install_parameter_plugin_t * currentPlugin;

    FUNC_ENTER();

    DBG_ASSERT(InstallParameter != NULL);
    DBG_ASSERT(PluginName != NULL);

    for (currentPlugin = InstallParameter->PluginList; currentPlugin != NULL; currentPlugin = currentPlugin->Next) {
        if (strcmp(currentPlugin->Name, PluginName) == 0) {
            exists = TRUE;
            break;
        }
    }

    FUNC_LEAVE_BOOL(exists);
}

static void
PluginAdd(cbm_install_parameter_t * InstallParameter, cbm_install_parameter_plugin_t * Plugin)
{
    cbm_install_parameter_plugin_t * previousPlugin;

    FUNC_ENTER();

    DBG_ASSERT(InstallParameter != NULL);
    DBG_ASSERT(Plugin != NULL);

    Plugin->Next = NULL;

    previousPlugin = (cbm_install_parameter_plugin_t *) & InstallParameter->PluginList;

    while (previousPlugin->Next) {
        previousPlugin = previousPlugin->Next;
    }

    DBG_ASSERT(previousPlugin);

    previousPlugin->Next = Plugin;

    FUNC_LEAVE();
}

void
PluginListFree(cbm_install_parameter_t * InstallParameter)
{
    cbm_install_parameter_plugin_t * currentPlugin;
    cbm_install_parameter_plugin_t * nextPlugin;

    FUNC_ENTER();

    DBG_ASSERT(InstallParameter != NULL);

    nextPlugin = InstallParameter->PluginList;

    while (nextPlugin != NULL) {
        currentPlugin = nextPlugin;

        nextPlugin = currentPlugin->Next;

        free(currentPlugin->Name);
        free(currentPlugin->FileName);
        free(currentPlugin->OptionMemory);
        free(currentPlugin);
    }

    FUNC_LEAVE();
}

static BOOL
plugin_exists(const char * plugin_name)
{
    BOOL exists = FALSE;
    char * filename = NULL;;

    FUNC_ENTER();

    /* try to open opencbm-XXX.dll, with XXX being the plugin_name */

    do {
        FILE * file;

        filename = malloc_plugin_file_name(plugin_name);

        if (filename == NULL)
            break;

        file = fopen(filename, "r");

        if (file) {
            fclose(file);
            exists = TRUE;
        }
        else {
            fprintf(stderr, "Plugin '%s' does not exist, aborting...\n", plugin_name);
        }

    } while (0);

    if (filename)
        free(filename);

    FUNC_LEAVE_BOOL(exists);
}

static int CBMAPIDECL
getopt_long_callback(int Argc, char * const Argv[], const char *Optstring, const struct option *Longopts)
{
    int retValue;

    FUNC_ENTER();

    retValue = getopt_long(Argc, Argv, Optstring, Longopts, NULL);

    FUNC_LEAVE_INT(retValue);
}

static cbm_install_parameter_plugin_t *
GetPluginData(const char * const PluginName, cbm_install_parameter_t * InstallParameter, int Argc, char * const Argv[])
{
    HINSTANCE library = NULL;
    char *plugin_file_name = NULL;
    void *option_memory = NULL;

    BOOL error = TRUE;

    cbm_install_parameter_plugin_t * returnValue = NULL;

    FUNC_ENTER();

    do {
        unsigned int option_memory_size = 0;

        cbm_plugin_install_process_commandline_t * cbm_plugin_install_process_commandline;
        cbm_plugin_install_get_needed_files_t * cbm_plugin_install_get_needed_files;

        cbm_install_parameter_plugin_t * pluginData;

        CbmPluginInstallProcessCommandlineData_t commandLineData;

        memset(&commandLineData, 0, sizeof(commandLineData));

        if (PluginAlreadyInList(PluginName, InstallParameter)) {
            fprintf(stderr, "Please, do not add plugin '%s' multiple times!\n", PluginName);
            break;
        }

        pluginData = malloc(sizeof(*pluginData));
        if ( ! pluginData)
            break;

        memset(pluginData, 0, sizeof(*pluginData));

        plugin_file_name = malloc_plugin_file_name(PluginName);

        if (plugin_file_name == NULL)
            break;

        /*
         * Load the DLL. Make sure that we do not get a warning dialog
         * if a dependancy DLL is not found.
         */

        {
            UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

            library = LoadLibrary(plugin_file_name);

            SetErrorMode(oldErrorMode);
        }

        if (library == NULL) {
            fprintf(stderr, "Error loading plugin '%s', ABORTING!.\n\n", PluginName);
            DBG_ERROR((DBG_PREFIX "Error loading plugin '%s', ABORTING!", PluginName));
            break;
        }

        cbm_plugin_install_process_commandline = (void *) GetProcAddress(library, "cbm_plugin_install_process_commandline");
        cbm_plugin_install_get_needed_files = (void *) GetProcAddress(library, "cbm_plugin_install_get_needed_files");

        if (0 == (cbm_plugin_install_process_commandline && cbm_plugin_install_get_needed_files)) 
            break;

        option_memory_size = cbm_plugin_install_process_commandline(&commandLineData);

        /* make sure option_memory_size is at least 1,
         * because malloc(0) can return either NULL or a valid memory block.
         * This way, we are consistent, regardless of how malloc() handles this.
         */

        option_memory_size = (option_memory_size > 0) ? option_memory_size : 1;

        option_memory = malloc(option_memory_size);

        if (option_memory == NULL)
            break;

        memset(option_memory, 0, option_memory_size);

        commandLineData.Argc               = Argc;
        commandLineData.Argv               = Argv;
        commandLineData.OptArg             = &optarg;
        commandLineData.OptInd             = &optind;
        commandLineData.OptErr             = &opterr;
        commandLineData.OptOpt             = &optopt;
        commandLineData.OptionMemory       = option_memory;
        commandLineData.GetoptLongCallback = &getopt_long_callback;
        commandLineData.InstallParameter   = InstallParameter;

        error = cbm_plugin_install_process_commandline(&commandLineData);

        if ( ! error && pluginData) {
            pluginData->Name = cbmlibmisc_strdup(PluginName);
            pluginData->FileName = cbmlibmisc_strdup(plugin_file_name);

            if (pluginData->Name == NULL || pluginData->FileName == NULL) {
                error = TRUE;
            }
        }

        if (error) {
            free(pluginData->FileName);
            free(pluginData->Name);
            free(pluginData);

            free(commandLineData.OptionMemory);

            break;
        }

        pluginData->OptionMemory = commandLineData.OptionMemory;

        error = TRUE; // assume error unless proven otherwise

        {
            int needed_files_length = cbm_plugin_install_get_needed_files(&commandLineData, NULL);

            pluginData->NeededFiles = malloc(needed_files_length);

            if (NULL == pluginData->NeededFiles)
                break;

            cbm_plugin_install_get_needed_files(&commandLineData, pluginData->NeededFiles);

            error = FALSE;
        }

        returnValue = pluginData;

    } while (0);

    if (library) {
        FreeLibrary(library);
    }

    free(plugin_file_name);

    if (error) {
        free(option_memory);
    }

    FUNC_LEAVE_PTR(returnValue, cbm_install_parameter_plugin_t *);
}

BOOL
ProcessPluginCommandline(const char * const PluginName, cbm_install_parameter_t * InstallParameter, int Argc, char * const Argv[])
{
    HINSTANCE library = NULL;
    char *plugin_file_name = NULL;
    void *option_memory = NULL;

    cbm_install_parameter_plugin_t *pluginParameter;

    BOOL error = TRUE;

    FUNC_ENTER();

    pluginParameter = GetPluginData(PluginName, InstallParameter, Argc, Argv);

    error = pluginParameter == NULL ? TRUE : FALSE;

    if ( ! error) {
        PluginAdd(InstallParameter, pluginParameter);
    }

    FUNC_LEAVE_BOOL(error);
}

BOOL
get_all_plugins(cbm_install_parameter_t * InstallParameter)
{
    BOOL error = FALSE;
    WIN32_FIND_DATA finddata;
    HANDLE findhandle;

    const char *findfilename = NULL; 

    FUNC_ENTER();

    do {
        findfilename = malloc_plugin_file_name("*");

        if ( ! findfilename) {
            error = TRUE;
            break;
        }

        findhandle = FindFirstFile(findfilename, &finddata);

        do {
            if (findhandle != INVALID_HANDLE_VALUE) {
                BOOL quit = FALSE;

                do {
                    if (finddata.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
                        /* extract the plugin name from the file name */

                        cbm_install_parameter_plugin_t * pluginParameter = NULL;

                        int len = strlen(finddata.cFileName) + 1;
                        char *pluginName = malloc(len);

                        if (pluginName == NULL) {
                            error = TRUE;
                            break;
                        }

                        strcpy(pluginName, finddata.cFileName + sizeof(PLUGIN_PREFIX) - 1);
                        pluginName[strlen(pluginName) - (sizeof(PLUGIN_SUFFIX) - 1)] = '\0';

//                      fprintf(stderr, "Found plugin '%s'.\n", pluginName);

                        pluginParameter = GetPluginData(pluginName, InstallParameter, 1, NULL);

                        if (pluginParameter) {
                            PluginAdd(InstallParameter, pluginParameter);
                        }
                        else {
                            error = TRUE;
                        }

                        free(pluginName);
                    }

                    if (FindNextFile(findhandle, &finddata) == 0) {
                        quit = TRUE;
                    }
                } while ( ! quit);

                FindClose(findhandle);

            }
        } while (0);
    } while (0);

    FUNC_LEAVE_BOOL(error);
}

static BOOL CBMAPIDECL
get_all_installed_plugins_callback(cbm_install_parameter_t * InstallParameter, const char * PluginName)
{
    cbm_install_parameter_plugin_t *pluginParameter;

    BOOL error = TRUE;

    FUNC_ENTER();

    pluginParameter = GetPluginData(PluginName, InstallParameter, 1, NULL);

    error = pluginParameter == NULL ? TRUE : FALSE;

    if ( ! error ) {
        PluginAdd(InstallParameter, pluginParameter);
    }

    FUNC_LEAVE_BOOL(error);
}

BOOL
get_all_installed_plugins(cbm_install_parameter_t * InstallParameter)
{
    HMODULE openCbmDllHandle = NULL;
    BOOL error = TRUE;
    opencbm_configuration_handle configuration_handle = NULL;

    FUNC_ENTER();

    do {
        cbm_plugin_get_all_plugin_names_context_t cbm_plugin_get_all_plugin_names_context;
        cbm_plugin_get_all_plugin_names_t * cbm_plugin_get_all_plugin_names;

        openCbmDllHandle = LoadLocalOpenCBMDll();
        if (openCbmDllHandle  == NULL) {
            DBG_PRINT((DBG_PREFIX "Could not open the OpenCBM DLL."));
            fprintf(stderr, "Could not open the OpenCBM DLL.");
            break;
        }

        cbm_plugin_get_all_plugin_names = (cbm_plugin_get_all_plugin_names_t *) 
            GetProcAddress(openCbmDllHandle, "cbm_plugin_get_all_plugin_names");

        if ( ! cbm_plugin_get_all_plugin_names ) {
            break;
        }

        cbm_plugin_get_all_plugin_names_context.Callback = get_all_installed_plugins_callback;
        cbm_plugin_get_all_plugin_names_context.InstallParameter = InstallParameter;

        if ( cbm_plugin_get_all_plugin_names(&cbm_plugin_get_all_plugin_names_context) ) {
            break;
        }

        error = FALSE;

    } while (0);

    if (openCbmDllHandle) {
        FreeLibrary(openCbmDllHandle);
    }

    opencbm_configuration_close(configuration_handle);

    FUNC_LEAVE_BOOL(error);
}

BOOL PluginForAll(cbm_install_parameter_t * InstallParameter, PluginForAll_Callback_t * Callback, void * Context)
{
    cbm_install_parameter_plugin_t * plugin = NULL;

    BOOL error = FALSE;

    FUNC_ENTER();

    for (plugin = InstallParameter->PluginList; (plugin != NULL) && ! error; plugin = plugin->Next) {
        printf("Using plugin: '%s' with filename '%s'.\n", plugin->Name, plugin->FileName);
        error = Callback(plugin, Context);
    }

    FUNC_LEAVE_BOOL(error);
}
