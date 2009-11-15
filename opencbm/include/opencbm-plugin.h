/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2008-2009 Spiro Trikaliotis
 */

/*! **************************************************************
** \file include/opencbm-plugin.h \n
** \author Spiro Trikaliotis \n
** \version $Id: opencbm-plugin.h,v 1.6 2009-11-15 20:55:40 strik Exp $ \n
** \n
** \brief Plugin DLL interface
**
****************************************************************/

#ifndef OPENCBM_PLUGIN_H
#define OPENCBM_PLUGIN_H

#include "opencbm.h"

/*! \brief @@@@@ \todo document

 \param PortNumber

 \return
*/
typedef const char * CBMAPIDECL cbm_plugin_get_driver_name_t(int PortNumber);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param PortNumber

 \return
*/
typedef int CBMAPIDECL cbm_plugin_driver_open_t(CBM_FILE *HandleDevice, int PortNumber);

/*! \brief @@@@@ \todo document

 \param HandleDevice
*/
typedef void CBMAPIDECL cbm_plugin_driver_close_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice
*/
typedef void CBMAPIDECL cbm_plugin_lock_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice
*/
typedef void CBMAPIDECL cbm_plugin_unlock_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Count

 \return
*/
typedef int CBMAPIDECL cbm_plugin_raw_write_t(CBM_FILE HandleDevice, const void *Buffer, size_t Count);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Count

 \return
*/
typedef int CBMAPIDECL cbm_plugin_raw_read_t(CBM_FILE HandleDevice, void *Buffer, size_t Count);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param DeviceAddress

 \param SecondaryAddress

 \return
*/
typedef int CBMAPIDECL cbm_plugin_open_t(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param DeviceAddress

 \param SecondaryAddress

 \return
*/
typedef int CBMAPIDECL cbm_plugin_close_t(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param DeviceAddress

 \param SecondaryAddress

 \return
*/
typedef int CBMAPIDECL cbm_plugin_listen_t(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param DeviceAddress

 \param SecondaryAddress

 \return
*/
typedef int CBMAPIDECL cbm_plugin_talk_t(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL cbm_plugin_unlisten_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL cbm_plugin_untalk_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL cbm_plugin_get_eoi_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL cbm_plugin_clear_eoi_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL cbm_plugin_reset_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef __u_char CBMAPIDECL cbm_plugin_pp_read_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Byte
*/
typedef void CBMAPIDECL cbm_plugin_pp_write_t(CBM_FILE HandleDevice, __u_char Byte);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL cbm_plugin_iec_poll_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Line

*/
typedef void CBMAPIDECL cbm_plugin_iec_set_t(CBM_FILE HandleDevice, int Line);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Line
*/
typedef void CBMAPIDECL cbm_plugin_iec_release_t(CBM_FILE HandleDevice, int Line);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Line
*/
typedef void CBMAPIDECL cbm_plugin_iec_setrelease_t(CBM_FILE HandleDevice, int Set, int Release);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Line

 \param State

 \return
*/
typedef int CBMAPIDECL cbm_plugin_iec_wait_t(CBM_FILE HandleDevice, int Line, int State);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef __u_char CBMAPIDECL cbm_plugin_parallel_burst_read_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Value
*/
typedef void CBMAPIDECL cbm_plugin_parallel_burst_write_t(CBM_FILE HandleDevice, __u_char Value);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL cbm_plugin_parallel_burst_read_track_t(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL cbm_plugin_parallel_burst_read_track_var_t(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Buffer

 \param Length

 \return
*/
typedef int CBMAPIDECL cbm_plugin_parallel_burst_write_track_t(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param data

 \param size
*/
typedef void CBMAPIDECL cbm_plugin_s1_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param data

 \param size
*/
typedef void CBMAPIDECL cbm_plugin_s1_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param data

 \param size
*/

typedef void CBMAPIDECL cbm_plugin_s2_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param data

 \param size
*/
typedef void CBMAPIDECL cbm_plugin_s2_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param data

 \param size
*/
typedef void CBMAPIDECL cbm_plugin_pp_dc_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param data

 \param size
*/
typedef void CBMAPIDECL cbm_plugin_pp_dc_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param data

 \param size
*/
typedef void CBMAPIDECL cbm_plugin_pp_cc_read_n_t (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param data

 \param size
*/
typedef void CBMAPIDECL cbm_plugin_pp_cc_write_n_t(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);


/*! \brief @@@@@ \todo document

 \param HandleDevice

 \return
*/
typedef int CBMAPIDECL cbm_plugin_iec_dbg_read_t(CBM_FILE HandleDevice);

/*! \brief @@@@@ \todo document

 \param HandleDevice

 \param Value

 \return
*/
typedef int CBMAPIDECL cbm_plugin_iec_dbg_write_t(CBM_FILE HandleDevice, unsigned char Value);

#ifdef WIN32
    /*
     * special functions for installing on Windows
     * These functions are not used in the normal process, only for installation.
     * Thus, they are missing in opencbm_plugin_s and opencbm_plugin_t.
     */

    // int (*p_opencbm_install_getopt)(int argc, char * const argv[], const char *optstring);

    

    /*! \brief @@@@@ \todo document

     \param Argc

     \param Argv

     \param OptString

     \param Longopts

     \return
    */
    typedef int CBMAPIDECL cbm_plugin_install_getopt_long_callback_t(int Argc, char * const Argv[], const char *Optstring, const struct option *Longopts);

    /*! \brief remember the operating system version */
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

    /*! \brief describe the files needed for installation */
    typedef
    struct opencbm_plugin_install_neededfiles_s opencbm_plugin_install_neededfiles_t;

    /*! \brief describe a plugin */
    typedef
    struct cbm_install_parameter_plugin_s 
    {
        struct cbm_install_parameter_plugin_s *Next;        /*!< pointer to the next plugin */
        char * FileName;                                    /*!< file name of the plugin. This is a decorated plugin name (Name). */
        char * Name;                                        /*!< plugin name */
        void * OptionMemory;                                /*!< pointer to same memory where the plugin can store options. */
        opencbm_plugin_install_neededfiles_t * NeededFiles; /*!< description of the needed files for this plugin. */

    } cbm_install_parameter_plugin_t;


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

    /*! \brief describe necessary information to process command line in a plugin

      This struct contains all necessary data to allow the plugin to process
      the command line parameters given to instcbm on its own.

      The instcbm tools gives this structure to the plugin. The structure includes
      some pointers necessary to use the getopt_long() function of instcbm.exe.
      This way, the plugin does not need to include this function on its own.
    */
    typedef struct CbmPluginInstallProcessCommandlineData_s {
        int    Argc;          /*!< the argc value, as if given to int main(int, char **) */
        char * const * Argv;  /*!< the argv value, as if given to int main(int, char **) */
        void * OptionMemory;  /*!< pointer to the option memory where the plugin can remember its options accross multiple calls */
        char **OptArg;        /*!< pointer to the char *optarg used in getopt_long() */
        int  * OptInd;        /*!< pointer to the int optind used in getopt_long() */
        int  * OptErr;        /*!< pointer to the int opterr used in getopt_long() */
        int  * OptOpt;        /*!< pointer to the int optopt used in getopt_long() */

        cbm_plugin_install_getopt_long_callback_t * GetoptLongCallback; /*!< pointer to a getopt_long() function */
        cbm_install_parameter_t                   * InstallParameter;   /*!< some generic install parameters */

    } CbmPluginInstallProcessCommandlineData_t;

    /*! \brief @@@@@ \todo document

     \param ProcessCommandlineData

     \return
    */
    typedef unsigned int CBMAPIDECL cbm_plugin_install_process_commandline_t(CbmPluginInstallProcessCommandlineData_t * ProcessCommandlineData); 

    
    /*! \brief @@@@@ \todo document

     \param OptionMemory

     \return
    */
    typedef BOOL CBMAPIDECL cbm_plugin_install_do_install_t(void * OptionMemory);

    /*! \brief @@@@@ \todo document

     \param OptionMemory

     \return
    */
    typedef BOOL CBMAPIDECL cbm_plugin_install_do_uninstall_t(void * OptionMemory);

    /*! \brief Where to install the files

      Helper enum for struct opencbm_plugin_install_neededfiles_s.
      It describes to which location each file is to be copied on installation.
    */
    typedef
    enum opencbm_plugin_install_location_e
    {
        LOCAL_DIR,        /*!< the file remains at the local dir (".") */
        LOCAL_PLUGIN_DIR, /*!< the file must be copied into the plugin dir ("./PLUGINNAME") */
        SYSTEM_DIR,       /*!< the file must be copied into the system dir ("c:\windows\system32") */
        DRIVER_DIR,       /*!< the file must be copied into the drivers dir ("c:\windows\system32\drivers") */
        LIST_END          /*!< MUST BE THE LAST ENTRY! */
    } opencbm_plugin_install_location_t;

    /*! \brief Describes the files to be copied on install

      This struct describes one file to be copied on install.
    */
    typedef
    struct opencbm_plugin_install_neededfiles_s
    {
        opencbm_plugin_install_location_t FileLocation;       /*!< the location of the file */
        const char                        Filename[100];      /*!< the file name */
        const char                      * FileLocationString; /*!< the location where the file is copied. 
                                                                   This is set to NULL by the plugin.
                                                                   instcbm uses it to keep track where the file resides. */
    } opencbm_plugin_install_neededfiles_t;

    
    /*! \brief @@@@@ \todo document

     \param Data

     \param Destination

     \return
    */
    typedef unsigned int CBMAPIDECL cbm_plugin_install_get_needed_files_t(CbmPluginInstallProcessCommandlineData_t * Data, opencbm_plugin_install_neededfiles_t * Destination);


    /*! \brief @@@@@ \todo document

     \param DefaultPluginname

     \return
    */
    typedef BOOL CBMAPIDECL cbm_plugin_install_generic_t(const char * DefaultPluginname);

    /*! \brief @@@@@ \todo document

     \param Pluginname

     \param Filepath

     \param CommandlineData

     \return
    */
    typedef BOOL CBMAPIDECL cbm_plugin_install_plugin_data_t(const char * Pluginname, const char * Filepath, const CbmPluginInstallProcessCommandlineData_t * CommandlineData);


    /*! \brief @@@@@ \todo document

     \param InstallParameter

     \param Pluginname

     \return
    */
    typedef BOOL CBMAPIDECL cbm_plugin_get_all_plugin_names_callback_t(cbm_install_parameter_t * InstallParameter, const char * Pluginname);

    /*! \brief context for cbm_plugin_get_all_plugin_names_t()

      This structure describe the callback to be used with cbm_plugin_get_all_plugin_names_t().
    */
    typedef struct cbm_plugin_get_all_plugin_names_context_s {
        cbm_plugin_get_all_plugin_names_callback_t * Callback;         /*!< pointer to the callback function */
        cbm_install_parameter_t                    * InstallParameter; /*!< additional data to be passed to the callback function */
    } cbm_plugin_get_all_plugin_names_context_t;

    /*! \brief @@@@@ \todo document

     \param Callback

     \return
    */
    typedef BOOL CBMAPIDECL cbm_plugin_get_all_plugin_names_t(cbm_plugin_get_all_plugin_names_context_t * Callback);

    /*! \brief @@@@@ \todo document

     \return
    */
    typedef int CBMAPIDECL cbm_plugin_self_init_plugin_t(void);

#endif


/*! \brief holds all callbacks of the plugin

  This structure contains all callbacks available in the plugin.

  \note
    Not all functions are mandatory. Thus, there might be NULL pointers available!
*/
typedef
struct opencbm_plugin_s {
    cbm_plugin_get_driver_name_t            * cbm_plugin_get_driver_name;            /*!< pointer to a cbm_plugin_get_driver_name_t() function */
    cbm_plugin_driver_open_t                * cbm_plugin_driver_open;                /*!< pointer to a cbm_plugin_driver_open_t() function */
    cbm_plugin_driver_close_t               * cbm_plugin_driver_close;               /*!< pointer to a cbm_plugin_driver_close_t() function */
    cbm_plugin_lock_t                       * cbm_plugin_lock;                       /*!< pointer to a cbm_plugin_lock_t() function */
    cbm_plugin_unlock_t                     * cbm_plugin_unlock;                     /*!< pointer to a cbm_plugin_unlock_t() function */
    cbm_plugin_raw_write_t                  * cbm_plugin_raw_write;                  /*!< pointer to a cbm_plugin_raw_write_t() function */
    cbm_plugin_raw_read_t                   * cbm_plugin_raw_read;                   /*!< pointer to a cbm_plugin_raw_read_t() function */
    cbm_plugin_open_t                       * cbm_plugin_open;                       /*!< pointer to a cbm_plugin_open_t() function */
    cbm_plugin_close_t                      * cbm_plugin_close;                      /*!< pointer to a cbm_plugin_close_t() function */
    cbm_plugin_listen_t                     * cbm_plugin_listen;                     /*!< pointer to a cbm_plugin_listen_t() function */
    cbm_plugin_talk_t                       * cbm_plugin_talk;                       /*!< pointer to a cbm_plugin_talk_t() function */
    cbm_plugin_unlisten_t                   * cbm_plugin_unlisten;                   /*!< pointer to a cbm_plugin_unlisten_t() function */
    cbm_plugin_untalk_t                     * cbm_plugin_untalk;                     /*!< pointer to a cbm_plugin_untalk_t() function */
    cbm_plugin_get_eoi_t                    * cbm_plugin_get_eoi;                    /*!< pointer to a cbm_plugin_get_eoi_t() function */
    cbm_plugin_clear_eoi_t                  * cbm_plugin_clear_eoi;                  /*!< pointer to a cbm_plugin_clear_eoi_t() function */
    cbm_plugin_reset_t                      * cbm_plugin_reset;                      /*!< pointer to a cbm_plugin_reset_t() function */
    cbm_plugin_pp_read_t                    * cbm_plugin_pp_read;                    /*!< pointer to a cbm_plugin_pp_read_t() function */
    cbm_plugin_pp_write_t                   * cbm_plugin_pp_write;                   /*!< pointer to a cbm_plugin_write_t() function */
    cbm_plugin_iec_poll_t                   * cbm_plugin_iec_poll;                   /*!< pointer to a cbm_plugin_iec_poll_t() function */
    cbm_plugin_iec_set_t                    * cbm_plugin_iec_set;                    /*!< pointer to a cbm_plugin_iec_set_t() function */
    cbm_plugin_iec_release_t                * cbm_plugin_iec_release;                /*!< pointer to a cbm_plugin_iec_release_t() function */
    cbm_plugin_iec_setrelease_t             * cbm_plugin_iec_setrelease;             /*!< pointer to a cbm_plugin_iec_setrelease_t() function */
    cbm_plugin_iec_wait_t                   * cbm_plugin_iec_wait;                   /*!< pointer to a cbm_plugin_iec_wait_t() function */

    cbm_plugin_parallel_burst_read_t        * cbm_plugin_parallel_burst_read;        /*!< pointer to a cbm_plugin_parallel_burst_read_t() function */
    cbm_plugin_parallel_burst_write_t       * cbm_plugin_parallel_burst_write;       /*!< pointer to a cbm_plugin_parallel_burst_write_t() function */
    cbm_plugin_parallel_burst_read_track_t  * cbm_plugin_parallel_burst_read_track;  /*!< pointer to a cbm_plugin_parallel_burst_read_track_t() function */
    cbm_plugin_parallel_burst_read_track_var_t  * cbm_plugin_parallel_burst_read_track_var;  /*!< pointer to a cbm_plugin_parallel_burst_read_track_var_t() function */
    cbm_plugin_parallel_burst_write_track_t * cbm_plugin_parallel_burst_write_track; /*!< pointer to a cbm_plugin_parallel_burst_write_track_t() function */

    cbm_plugin_iec_dbg_read_t               * cbm_plugin_iec_dbg_read;               /*!< pointer to a cbm_plugin_iec_dbg_read_t() function */
    cbm_plugin_iec_dbg_write_t              * cbm_plugin_iec_dbg_write;              /*!< pointer to a cbm_plugin_iec_dbg_write_t() function */
} opencbm_plugin_t;

#endif // #ifndef OPENCBM_PLUGIN_H
