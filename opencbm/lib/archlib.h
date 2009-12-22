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

EXTERN const char * CBMAPIDECL cbmarch_get_driver_name(int PortNumber);
EXTERN cbm_plugin_driver_open_t                cbmarch_driver_open;
EXTERN cbm_plugin_driver_close_t               cbmarch_driver_close;
EXTERN cbm_plugin_lock_t                       cbmarch_lock;
EXTERN cbm_plugin_unlock_t                     cbmarch_unlock;
EXTERN cbm_plugin_raw_write_t                  cbmarch_raw_write;
EXTERN cbm_plugin_raw_read_t                   cbmarch_raw_read;
EXTERN cbm_plugin_listen_t                     cbmarch_listen;
EXTERN cbm_plugin_talk_t                       cbmarch_talk;
EXTERN cbm_plugin_open_t                       cbmarch_open;
EXTERN cbm_plugin_unlisten_t                   cbmarch_unlisten;
EXTERN cbm_plugin_untalk_t                     cbmarch_untalk;
EXTERN cbm_plugin_close_t                      cbmarch_close;
EXTERN cbm_plugin_unlisten_t                   cbmarch_unlisten;
EXTERN cbm_plugin_untalk_t                     cbmarch_untalk;
EXTERN cbm_plugin_get_eoi_t                    cbmarch_get_eoi;
EXTERN cbm_plugin_clear_eoi_t                  cbmarch_clear_eoi;
EXTERN cbm_plugin_reset_t                      cbmarch_reset;
EXTERN cbm_plugin_pp_read_t                    cbmarch_pp_read;
EXTERN cbm_plugin_pp_write_t                   cbmarch_pp_write;
EXTERN cbm_plugin_iec_poll_t                   cbmarch_iec_poll;
EXTERN cbm_plugin_iec_set_t                    cbmarch_iec_set;
EXTERN cbm_plugin_iec_release_t                cbmarch_iec_release;
EXTERN cbm_plugin_iec_setrelease_t             cbmarch_iec_setrelease;
EXTERN cbm_plugin_iec_wait_t                   cbmarch_iec_wait;

EXTERN cbm_plugin_parallel_burst_read_t        cbmarch_parallel_burst_read;
EXTERN cbm_plugin_parallel_burst_write_t       cbmarch_parallel_burst_write;
EXTERN cbm_plugin_parallel_burst_read_n_t      cbmarch_parallel_burst_read_n;
EXTERN cbm_plugin_parallel_burst_write_n_t     cbmarch_parallel_burst_write_n;
EXTERN cbm_plugin_parallel_burst_read_track_t  cbmarch_parallel_burst_read_track;
EXTERN cbm_plugin_parallel_burst_read_track_var_t cbmarch_parallel_burst_read_track_var;
EXTERN cbm_plugin_parallel_burst_write_track_t cbmarch_parallel_burst_write_track;


EXTERN cbm_plugin_s1_read_n_t                  cbmarch_s1_read_n;
EXTERN cbm_plugin_s1_write_n_t                 cbmarch_s1_write_n;
EXTERN cbm_plugin_s2_read_n_t                  cbmarch_s2_read_n;
EXTERN cbm_plugin_s2_write_n_t                 cbmarch_s2_write_n;
EXTERN cbm_plugin_pp_dc_read_n_t               cbmarch_pp_dc_read_n;
EXTERN cbm_plugin_pp_dc_write_n_t              cbmarch_pp_dc_write_n;
EXTERN cbm_plugin_pp_cc_read_n_t               cbmarch_pp_cc_read_n;
EXTERN cbm_plugin_pp_cc_write_n_t              cbmarch_pp_cc_write_n;

EXTERN cbm_plugin_iec_dbg_read_t               cbmarch_iec_dbg_read;
EXTERN cbm_plugin_iec_dbg_write_t              cbmarch_iec_dbg_write;


#ifdef WIN32

EXTERN cbm_plugin_install_process_commandline_t cbm_plugin_install_process_commandline;

EXTERN cbm_plugin_install_do_install_t          cbm_plugin_install_do_install;
EXTERN cbm_plugin_install_do_uninstall_t        cbm_plugin_install_do_uninstall;
EXTERN cbm_plugin_install_get_needed_files_t    cbm_plugin_install_get_needed_files;

EXTERN cbm_plugin_install_generic_t             cbm_plugin_install_generic;
EXTERN cbm_plugin_install_plugin_data_t         cbm_plugin_install_plugin_data;
EXTERN cbm_plugin_get_all_plugin_names_t        cbm_plugin_get_all_plugin_names;
EXTERN cbm_plugin_self_init_plugin_t            cbm_plugin_self_init_plugin;

#endif

#endif // #ifndef ARCHLIB_H
