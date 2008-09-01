/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2008 Spiro Trikaliotis
 */

/*! **************************************************************
** \file include/opencbm-plugin.h \n
** \author Spiro Trikaliotis \n
** \version $Id: opencbm-plugin.h,v 1.4 2008-09-01 18:41:50 strik Exp $ \n
** \n
** \brief Plugin DLL interface
**
****************************************************************/

#ifndef OPENCBM_PLUGIN_H
#define OPENCBM_PLUGIN_H

#include "opencbm.h"

typedef const char * CBMAPIDECL cbm_plugin_get_driver_name_t(int PortNumber);
typedef int          CBMAPIDECL cbm_plugin_driver_open_t(CBM_FILE *HandleDevice, int PortNumber);
typedef void         CBMAPIDECL cbm_plugin_driver_close_t(CBM_FILE HandleDevice);
typedef void         CBMAPIDECL cbm_plugin_lock_t(CBM_FILE HandleDevice);
typedef void         CBMAPIDECL cbm_plugin_unlock_t(CBM_FILE HandleDevice);
typedef int          CBMAPIDECL cbm_plugin_raw_write_t(CBM_FILE HandleDevice, const void *Buffer, size_t Count);
typedef int          CBMAPIDECL cbm_plugin_raw_read_t(CBM_FILE HandleDevice, void *Buffer, size_t Count);
typedef int          CBMAPIDECL cbm_plugin_open_t(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
typedef int          CBMAPIDECL cbm_plugin_close_t(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
typedef int          CBMAPIDECL cbm_plugin_listen_t(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
typedef int          CBMAPIDECL cbm_plugin_talk_t(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
typedef int          CBMAPIDECL cbm_plugin_unlisten_t(CBM_FILE HandleDevice);
typedef int          CBMAPIDECL cbm_plugin_untalk_t(CBM_FILE HandleDevice);
typedef int          CBMAPIDECL cbm_plugin_get_eoi_t(CBM_FILE HandleDevice);
typedef int          CBMAPIDECL cbm_plugin_clear_eoi_t(CBM_FILE HandleDevice);
typedef int          CBMAPIDECL cbm_plugin_reset_t(CBM_FILE HandleDevice);
typedef __u_char     CBMAPIDECL cbm_plugin_pp_read_t(CBM_FILE HandleDevice);
typedef void         CBMAPIDECL cbm_plugin_pp_write_t(CBM_FILE HandleDevice, __u_char Byte);
typedef int          CBMAPIDECL cbm_plugin_iec_poll_t(CBM_FILE HandleDevice);
typedef void         CBMAPIDECL cbm_plugin_iec_set_t(CBM_FILE HandleDevice, int Line);
typedef void         CBMAPIDECL cbm_plugin_iec_release_t(CBM_FILE HandleDevice, int Line);
typedef void         CBMAPIDECL cbm_plugin_iec_setrelease_t(CBM_FILE HandleDevice, int Set, int Release);
typedef int          CBMAPIDECL cbm_plugin_iec_wait_t(CBM_FILE HandleDevice, int Line, int State);
typedef __u_char     CBMAPIDECL cbm_plugin_parallel_burst_read_t(CBM_FILE HandleDevice);
typedef void         CBMAPIDECL cbm_plugin_parallel_burst_write_t(CBM_FILE HandleDevice, __u_char Value);
typedef int          CBMAPIDECL cbm_plugin_parallel_burst_read_track_t(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);
typedef int          CBMAPIDECL cbm_plugin_parallel_burst_write_track_t(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);


typedef void         CBMAPIDECL cbm_plugin_s1_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
typedef void         CBMAPIDECL cbm_plugin_s1_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

typedef void         CBMAPIDECL cbm_plugin_s2_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
typedef void         CBMAPIDECL cbm_plugin_s2_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

typedef void         CBMAPIDECL cbm_plugin_pp_dc_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
typedef void         CBMAPIDECL cbm_plugin_pp_dc_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

typedef void         CBMAPIDECL cbm_plugin_pp_cc_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
typedef void         CBMAPIDECL cbm_plugin_pp_cc_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);


typedef int          CBMAPIDECL cbm_plugin_iec_dbg_read_t(CBM_FILE HandleDevice);
typedef int          CBMAPIDECL cbm_plugin_iec_dbg_write_t(CBM_FILE HandleDevice, unsigned char Value);

#ifdef WIN32
    /*
     * special functions for installing on Windows
     * These functions are not used in the normal process, only for installation.
     * Thus, they are missing in opencbm_plugin_s and opencbm_plugin_t.
     */

    // int (*p_opencbm_install_getopt)(int argc, char * const argv[], const char *optstring);

    typedef int          CBMAPIDECL cbm_plugin_install_getopt_long_callback_t(int Argc, char * const Argv[], const char *Optstring, const struct option *Longopts);

    typedef 
    enum osversion_e
    {
        WINUNSUPPORTED, //!< an unsupported operating system
        WINNT3,         //!< Windows NT 3.x (does the driver work there?
        WINNT4,         //!< Windows NT 4.x
        WIN2000,        //!< Windows 2000 (NT 5.0)
        WINXP,          //!< Windows XP (NT 5.1)
        WINVISTA,       //!< Windows Vista (NT 6.0)
        WINNEWER        //!< newer than Windows Vista
    } osversion_t;

    typedef
    struct opencbm_plugin_install_neededfiles_s opencbm_plugin_install_neededfiles_t;

    typedef
    struct cbm_install_parameter_plugin_s 
    {
        struct cbm_install_parameter_plugin_s *Next;

        char * FileName;

        char * Name;

        void * OptionMemory;

        opencbm_plugin_install_neededfiles_t * NeededFiles;

    } cbm_install_parameter_plugin_t;


    typedef
    struct cbm_install_parameter_plugin_s cbm_install_parameter_plugin_t;

    /*! \brief The parameter which are given on the command-line */
    typedef
    struct cbm_install_parameter_s
    {
        /*! Do not execute anything */
        BOOL NoExecute;

        /*! Need administrator privileges */
        BOOL AdminNeeded;

        /*! Find out of more than one "execute" parameter is given */
        BOOL ExecuteParameterGiven;

        /*! Install OpenCBM, that is, no other command has been given */
        BOOL Install;

        /*! --remove was given */
        BOOL Remove;

        /*! --nocopy was given */
        BOOL NoCopy;

        /*! --update was given */
        BOOL Update;

        /*! --check was given */
        BOOL CheckInstall;

        /*! --buffer was given */
        BOOL OutputDebuggingBuffer;

        /*! --debugflags was given */
        BOOL DebugFlagsDriverWereGiven;

        /*! --debugflags, a second parameter (for the DLL) was given */
        BOOL DebugFlagsDllWereGiven;

        /*! --debugflags, a third parameter (for INSTCBM itself) was given */
        BOOL DebugFlagsInstallWereGiven;

        /*! if --debugflags was given: the number which was there */
        ULONG DebugFlagsDriver;

        /*! if --debugflags with 2 parameters was given: the number which was there */
        ULONG DebugFlagsDll;

        /*! if --debugflags with 3 parameters was given: the number which was there */
        ULONG DebugFlagsInstall;

        /*! The type of the OS version */
        osversion_t OsVersion;

        /*! pointer to an array of pointers to the strings containg the names of the plugins.
         * Convention:
         * - the last entry is marked with a NULL pointer.
         * - If Plugins is not NULL, there have been two malloc()s.
         *   To free the data, one has to call:
         *   free(Plugins[0]);
         *   free(Plugins);
         */
        // @@@ char ** PluginNames;
        cbm_install_parameter_plugin_t * PluginList;

        /*! if set, there was no explicit plugin name given, but the list is a default one */
        BOOL NoExplicitPluginGiven;

    } cbm_install_parameter_t;

    typedef struct CbmPluginInstallProcessCommandlineData_s {
        int    Argc;
        char * const * Argv;
        void * OptionMemory;
        char **OptArg;
        int  * OptInd;
        int  * OptErr;
        int  * OptOpt;

        cbm_plugin_install_getopt_long_callback_t * GetoptLongCallback;
        cbm_install_parameter_t                   * InstallParameter;

    } CbmPluginInstallProcessCommandlineData_t;

    typedef unsigned int CBMAPIDECL cbm_plugin_install_process_commandline_t(CbmPluginInstallProcessCommandlineData_t *);

    typedef BOOL         CBMAPIDECL cbm_plugin_install_do_install_t(void * OptionMemory);
    typedef BOOL         CBMAPIDECL cbm_plugin_install_do_uninstall_t(void * OptionMemory);

    typedef
    enum opencbm_plugin_install_location_e
    {
        LOCAL_DIR,
        LOCAL_PLUGIN_DIR,
        SYSTEM_DIR,
        DRIVER_DIR,
        LIST_END
    } opencbm_plugin_install_location_t;

    typedef
    struct opencbm_plugin_install_neededfiles_s
    {
        opencbm_plugin_install_location_t FileLocation;
        const char                        Filename[100];
        const char                      * FileLocationString;
    } opencbm_plugin_install_neededfiles_t;

    typedef unsigned int CBMAPIDECL cbm_plugin_install_get_needed_files_t(CbmPluginInstallProcessCommandlineData_t * Data, opencbm_plugin_install_neededfiles_t * Destination);

    typedef BOOL CBMAPIDECL cbm_plugin_install_generic_t(const char * DefaultPluginname);
    typedef BOOL CBMAPIDECL cbm_plugin_install_plugin_data_t(const char * Pluginname, const char * Filepath, const CbmPluginInstallProcessCommandlineData_t * CommandlineData);

    typedef BOOL CBMAPIDECL cbm_plugin_get_all_plugin_names_callback_t(cbm_install_parameter_t * InstallParameter, const char * PluginName);
    typedef struct cbm_plugin_get_all_plugin_names_context_s {
        cbm_plugin_get_all_plugin_names_callback_t * Callback;
        cbm_install_parameter_t                    * InstallParameter;
    } cbm_plugin_get_all_plugin_names_context_t;

    typedef BOOL CBMAPIDECL cbm_plugin_get_all_plugin_names_t(cbm_plugin_get_all_plugin_names_context_t * Callback);

    typedef int  CBMAPIDECL cbm_plugin_self_init_plugin_t(void);

#endif


struct opencbm_plugin_s {
    cbm_plugin_get_driver_name_t            * cbm_plugin_get_driver_name;
    cbm_plugin_driver_open_t                * cbm_plugin_driver_open;
    cbm_plugin_driver_close_t               * cbm_plugin_driver_close;
    cbm_plugin_lock_t                       * cbm_plugin_lock;
    cbm_plugin_unlock_t                     * cbm_plugin_unlock;
    cbm_plugin_raw_write_t                  * cbm_plugin_raw_write;
    cbm_plugin_raw_read_t                   * cbm_plugin_raw_read;
    cbm_plugin_open_t                       * cbm_plugin_open;
    cbm_plugin_close_t                      * cbm_plugin_close;
    cbm_plugin_listen_t                     * cbm_plugin_listen;
    cbm_plugin_talk_t                       * cbm_plugin_talk;
    cbm_plugin_unlisten_t                   * cbm_plugin_unlisten;
    cbm_plugin_untalk_t                     * cbm_plugin_untalk;
    cbm_plugin_get_eoi_t                    * cbm_plugin_get_eoi;
    cbm_plugin_clear_eoi_t                  * cbm_plugin_clear_eoi;
    cbm_plugin_reset_t                      * cbm_plugin_reset;
    cbm_plugin_pp_read_t                    * cbm_plugin_pp_read;
    cbm_plugin_pp_write_t                   * cbm_plugin_pp_write;
    cbm_plugin_iec_poll_t                   * cbm_plugin_iec_poll;
    cbm_plugin_iec_set_t                    * cbm_plugin_iec_set;
    cbm_plugin_iec_release_t                * cbm_plugin_iec_release;
    cbm_plugin_iec_setrelease_t             * cbm_plugin_iec_setrelease;
    cbm_plugin_iec_wait_t                   * cbm_plugin_iec_wait;

    cbm_plugin_parallel_burst_read_t        * cbm_plugin_parallel_burst_read;
    cbm_plugin_parallel_burst_write_t       * cbm_plugin_parallel_burst_write;
    cbm_plugin_parallel_burst_read_track_t  * cbm_plugin_parallel_burst_read_track;
    cbm_plugin_parallel_burst_write_track_t * cbm_plugin_parallel_burst_write_track;

    cbm_plugin_iec_dbg_read_t               * cbm_plugin_iec_dbg_read;
    cbm_plugin_iec_dbg_write_t              * cbm_plugin_iec_dbg_write;
};

typedef struct opencbm_plugin_s opencbm_plugin_t;

#endif // #ifndef OPENCBM_PLUGIN_H
