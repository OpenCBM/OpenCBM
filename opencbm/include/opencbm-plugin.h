#ifndef OPENCBM_PLUGIN_H
#define OPENCBM_PLUGIN_H

#include "opencbm.h"

typedef const char * (CBMAPIDECL *cbm_plugin_get_driver_name_t)(int PortNumber);
typedef int          (CBMAPIDECL *cbm_plugin_driver_open_t)(CBM_FILE *HandleDevice, int PortNumber);
typedef void         (CBMAPIDECL *cbm_plugin_driver_close_t)(CBM_FILE HandleDevice);
typedef void         (CBMAPIDECL *cbm_plugin_lock_t)(CBM_FILE HandleDevice);
typedef void         (CBMAPIDECL *cbm_plugin_unlock_t)(CBM_FILE HandleDevice);
typedef int          (CBMAPIDECL *cbm_plugin_raw_write_t)(CBM_FILE HandleDevice, const void *Buffer, size_t Count);
typedef int          (CBMAPIDECL *cbm_plugin_raw_read_t)(CBM_FILE HandleDevice, void *Buffer, size_t Count);
typedef int          (CBMAPIDECL *cbm_plugin_open_t)(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
typedef int          (CBMAPIDECL *cbm_plugin_close_t)(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
typedef int          (CBMAPIDECL *cbm_plugin_listen_t)(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
typedef int          (CBMAPIDECL *cbm_plugin_talk_t)(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
typedef int          (CBMAPIDECL *cbm_plugin_unlisten_t)(CBM_FILE HandleDevice);
typedef int          (CBMAPIDECL *cbm_plugin_untalk_t)(CBM_FILE HandleDevice);
typedef int          (CBMAPIDECL *cbm_plugin_get_eoi_t)(CBM_FILE HandleDevice);
typedef int          (CBMAPIDECL *cbm_plugin_clear_eoi_t)(CBM_FILE HandleDevice);
typedef int          (CBMAPIDECL *cbm_plugin_reset_t)(CBM_FILE HandleDevice);
typedef __u_char     (CBMAPIDECL *cbm_plugin_pp_read_t)(CBM_FILE HandleDevice);
typedef void         (CBMAPIDECL *cbm_plugin_pp_write_t)(CBM_FILE HandleDevice, __u_char Byte);
typedef int          (CBMAPIDECL *cbm_plugin_iec_poll_t)(CBM_FILE HandleDevice);
typedef void         (CBMAPIDECL *cbm_plugin_iec_set_t)(CBM_FILE HandleDevice, int Line);
typedef void         (CBMAPIDECL *cbm_plugin_iec_release_t)(CBM_FILE HandleDevice, int Line);
typedef void         (CBMAPIDECL *cbm_plugin_iec_setrelease_t)(CBM_FILE HandleDevice, int Set, int Release);
typedef int          (CBMAPIDECL *cbm_plugin_iec_wait_t)(CBM_FILE HandleDevice, int Line, int State);
typedef __u_char     (CBMAPIDECL *cbm_plugin_parallel_burst_read_t)(CBM_FILE HandleDevice);
typedef void         (CBMAPIDECL *cbm_plugin_parallel_burst_write_t)(CBM_FILE HandleDevice, __u_char Value);
typedef int          (CBMAPIDECL *cbm_plugin_parallel_burst_read_track_t)(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);
typedef int          (CBMAPIDECL *cbm_plugin_parallel_burst_write_track_t)(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);


typedef void         (CBMAPIDECL *cbm_plugin_s1_read_n_t) (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
typedef void         (CBMAPIDECL *cbm_plugin_s1_write_n_t)(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

typedef void         (CBMAPIDECL *cbm_plugin_s2_read_n_t) (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
typedef void         (CBMAPIDECL *cbm_plugin_s2_write_n_t)(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

typedef void         (CBMAPIDECL *cbm_plugin_pp_dc_read_n_t) (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
typedef void         (CBMAPIDECL *cbm_plugin_pp_dc_write_n_t)(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

typedef void         (CBMAPIDECL *cbm_plugin_pp_cc_read_n_t) (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
typedef void         (CBMAPIDECL *cbm_plugin_pp_cc_write_n_t)(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

struct opencbm_plugin_s {
    cbm_plugin_get_driver_name_t            cbm_plugin_get_driver_name;
    cbm_plugin_driver_open_t                cbm_plugin_driver_open;
    cbm_plugin_driver_close_t               cbm_plugin_driver_close;
    cbm_plugin_lock_t                       cbm_plugin_lock;
    cbm_plugin_unlock_t                     cbm_plugin_unlock;
    cbm_plugin_raw_write_t                  cbm_plugin_raw_write;
    cbm_plugin_raw_read_t                   cbm_plugin_raw_read;
    cbm_plugin_open_t                       cbm_plugin_open;
    cbm_plugin_close_t                      cbm_plugin_close;
    cbm_plugin_listen_t                     cbm_plugin_listen;
    cbm_plugin_talk_t                       cbm_plugin_talk;
    cbm_plugin_unlisten_t                   cbm_plugin_unlisten;
    cbm_plugin_untalk_t                     cbm_plugin_untalk;
    cbm_plugin_get_eoi_t                    cbm_plugin_get_eoi;
    cbm_plugin_clear_eoi_t                  cbm_plugin_clear_eoi;
    cbm_plugin_reset_t                      cbm_plugin_reset;
    cbm_plugin_pp_read_t                    cbm_plugin_pp_read;
    cbm_plugin_pp_write_t                   cbm_plugin_pp_write;
    cbm_plugin_iec_poll_t                   cbm_plugin_iec_poll;
    cbm_plugin_iec_set_t                    cbm_plugin_iec_set;
    cbm_plugin_iec_release_t                cbm_plugin_iec_release;
    cbm_plugin_iec_setrelease_t             cbm_plugin_iec_setrelease;
    cbm_plugin_iec_wait_t                   cbm_plugin_iec_wait;

    cbm_plugin_parallel_burst_read_t        cbm_plugin_parallel_burst_read;
    cbm_plugin_parallel_burst_write_t       cbm_plugin_parallel_burst_write;
    cbm_plugin_parallel_burst_read_track_t  cbm_plugin_parallel_burst_read_track;
    cbm_plugin_parallel_burst_write_track_t cbm_plugin_parallel_burst_write_track;
};

typedef struct opencbm_plugin_s opencbm_plugin_t;

#endif // #ifndef OPENCBM_PLUGIN_H
