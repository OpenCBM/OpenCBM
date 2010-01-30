/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005-2009 Spiro Trikaliotis
 */

#ifndef ARCHLIB_H
#define ARCHLIB_H

#include "opencbm.h"

#undef EXTERN

#include "opencbm-plugin.h"

#if defined WIN32

# if defined OPENCBM_PLUGIN
#  define EXTERN __declspec(dllexport) /*!< we are exporting the functions */
# else
#  define EXTERN __declspec(dllimport) /*!< we are importing the functions */
# endif

#else
# define EXTERN extern
#endif

EXTERN opencbm_plugin_get_driver_name_t            opencbm_plugin_get_driver_name;
EXTERN opencbm_plugin_driver_open_t                opencbm_plugin_driver_open;
EXTERN opencbm_plugin_driver_close_t               opencbm_plugin_driver_close;
EXTERN opencbm_plugin_lock_t                       opencbm_plugin_lock;
EXTERN opencbm_plugin_unlock_t                     opencbm_plugin_unlock;
EXTERN opencbm_plugin_raw_write_t                  opencbm_plugin_raw_write;
EXTERN opencbm_plugin_raw_read_t                   opencbm_plugin_raw_read;
EXTERN opencbm_plugin_listen_t                     opencbm_plugin_listen;
EXTERN opencbm_plugin_talk_t                       opencbm_plugin_talk;
EXTERN opencbm_plugin_open_t                       opencbm_plugin_open;
EXTERN opencbm_plugin_unlisten_t                   opencbm_plugin_unlisten;
EXTERN opencbm_plugin_untalk_t                     opencbm_plugin_untalk;
EXTERN opencbm_plugin_close_t                      opencbm_plugin_close;
EXTERN opencbm_plugin_unlisten_t                   opencbm_plugin_unlisten;
EXTERN opencbm_plugin_untalk_t                     opencbm_plugin_untalk;
EXTERN opencbm_plugin_get_eoi_t                    opencbm_plugin_get_eoi;
EXTERN opencbm_plugin_clear_eoi_t                  opencbm_plugin_clear_eoi;
EXTERN opencbm_plugin_reset_t                      opencbm_plugin_reset;
EXTERN opencbm_plugin_pp_read_t                    opencbm_plugin_pp_read;
EXTERN opencbm_plugin_pp_write_t                   opencbm_plugin_pp_write;
EXTERN opencbm_plugin_iec_poll_t                   opencbm_plugin_iec_poll;
EXTERN opencbm_plugin_iec_set_t                    opencbm_plugin_iec_set;
EXTERN opencbm_plugin_iec_release_t                opencbm_plugin_iec_release;
EXTERN opencbm_plugin_iec_setrelease_t             opencbm_plugin_iec_setrelease;
EXTERN opencbm_plugin_iec_wait_t                   opencbm_plugin_iec_wait;

EXTERN opencbm_plugin_parallel_burst_read_t        opencbm_plugin_parallel_burst_read;
EXTERN opencbm_plugin_parallel_burst_write_t       opencbm_plugin_parallel_burst_write;
EXTERN opencbm_plugin_parallel_burst_read_n_t      opencbm_plugin_parallel_burst_read_n;
EXTERN opencbm_plugin_parallel_burst_write_n_t     opencbm_plugin_parallel_burst_write_n;
EXTERN opencbm_plugin_parallel_burst_read_track_t  opencbm_plugin_parallel_burst_read_track;
EXTERN opencbm_plugin_parallel_burst_read_track_var_t opencbm_plugin_parallel_burst_read_track_var;
EXTERN opencbm_plugin_parallel_burst_write_track_t opencbm_plugin_parallel_burst_write_track;


EXTERN opencbm_plugin_s1_read_n_t                  opencbm_plugin_s1_read_n;
EXTERN opencbm_plugin_s1_write_n_t                 opencbm_plugin_s1_write_n;
EXTERN opencbm_plugin_s2_read_n_t                  opencbm_plugin_s2_read_n;
EXTERN opencbm_plugin_s2_write_n_t                 opencbm_plugin_s2_write_n;
EXTERN opencbm_plugin_pp_dc_read_n_t               opencbm_plugin_pp_dc_read_n;
EXTERN opencbm_plugin_pp_dc_write_n_t              opencbm_plugin_pp_dc_write_n;
EXTERN opencbm_plugin_pp_cc_read_n_t               opencbm_plugin_pp_cc_read_n;
EXTERN opencbm_plugin_pp_cc_write_n_t              opencbm_plugin_pp_cc_write_n;

EXTERN opencbm_plugin_iec_dbg_read_t               opencbm_plugin_iec_dbg_read;
EXTERN opencbm_plugin_iec_dbg_write_t              opencbm_plugin_iec_dbg_write;

EXTERN opencbm_plugin_init_t                       opencbm_plugin_init;
EXTERN opencbm_plugin_uninit_t                     opencbm_plugin_uninit;

#ifdef WIN32

EXTERN opencbm_plugin_install_process_commandline_t opencbm_plugin_install_process_commandline;

EXTERN opencbm_plugin_install_do_install_t          opencbm_plugin_install_do_install;
EXTERN opencbm_plugin_install_do_uninstall_t        opencbm_plugin_install_do_uninstall;
EXTERN opencbm_plugin_install_get_needed_files_t    opencbm_plugin_install_get_needed_files;

EXTERN opencbm_plugin_install_generic_t             opencbm_plugin_install_generic;
EXTERN opencbm_plugin_install_plugin_data_t         opencbm_plugin_install_plugin_data;
EXTERN opencbm_plugin_get_all_plugin_names_t        opencbm_plugin_get_all_plugin_names;
EXTERN opencbm_plugin_self_init_plugin_t            opencbm_plugin_self_init_plugin;
EXTERN opencbm_plugin_self_uninit_plugin_t          opencbm_plugin_self_uninit_plugin;

/* functions of the opencbm.dll that can be used by plugins */

#include "configuration.h"

extern int plugin_is_active(opencbm_configuration_handle Handle, const char Section[]);
extern int plugin_set_active(const char Section[]);
extern int plugin_set_inactive(const char Section[]);

#endif

#endif // #ifndef ARCHLIB_H
