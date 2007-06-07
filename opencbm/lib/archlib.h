#ifndef ARCHLIB_H
#define ARCHLIB_H

#include "opencbm.h"

#undef EXTERN

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
EXTERN int          CBMAPIDECL cbmarch_driver_open(CBM_FILE *HandleDevice, int PortNumber);
EXTERN void         CBMAPIDECL cbmarch_driver_close(CBM_FILE HandleDevice);
EXTERN void         CBMAPIDECL cbmarch_lock(CBM_FILE HandleDevice);
EXTERN void         CBMAPIDECL cbmarch_unlock(CBM_FILE HandleDevice);
EXTERN int          CBMAPIDECL cbmarch_raw_write(CBM_FILE HandleDevice, const void *Buffer, size_t Count);
EXTERN int          CBMAPIDECL cbmarch_raw_read(CBM_FILE HandleDevice, void *Buffer, size_t Count);
EXTERN int          CBMAPIDECL cbmarch_listen(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
EXTERN int          CBMAPIDECL cbmarch_talk(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
EXTERN int          CBMAPIDECL cbmarch_open(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
EXTERN int          CBMAPIDECL cbmarch_unlisten(CBM_FILE HandleDevice);
EXTERN int          CBMAPIDECL cbmarch_untalk(CBM_FILE HandleDevice);
EXTERN int          CBMAPIDECL cbmarch_close(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char SecondaryAddress);
EXTERN int          CBMAPIDECL cbmarch_unlisten(CBM_FILE HandleDevice);
EXTERN int          CBMAPIDECL cbmarch_untalk(CBM_FILE HandleDevice);
EXTERN int          CBMAPIDECL cbmarch_get_eoi(CBM_FILE HandleDevice);
EXTERN int          CBMAPIDECL cbmarch_clear_eoi(CBM_FILE HandleDevice);
EXTERN int          CBMAPIDECL cbmarch_reset(CBM_FILE HandleDevice);
EXTERN __u_char     CBMAPIDECL cbmarch_pp_read(CBM_FILE HandleDevice);
EXTERN void         CBMAPIDECL cbmarch_pp_write(CBM_FILE HandleDevice, __u_char Byte);
EXTERN int          CBMAPIDECL cbmarch_iec_poll(CBM_FILE HandleDevice);
EXTERN void         CBMAPIDECL cbmarch_iec_set(CBM_FILE HandleDevice, int Line);
EXTERN void         CBMAPIDECL cbmarch_iec_release(CBM_FILE HandleDevice, int Line);
EXTERN void         CBMAPIDECL cbmarch_iec_setrelease(CBM_FILE HandleDevice, int Set, int Release);
EXTERN int          CBMAPIDECL cbmarch_iec_wait(CBM_FILE HandleDevice, int Line, int State);

EXTERN __u_char     CBMAPIDECL cbmarch_parallel_burst_read(CBM_FILE HandleDevice);
EXTERN void         CBMAPIDECL cbmarch_parallel_burst_write(CBM_FILE HandleDevice, __u_char Value);
EXTERN int          CBMAPIDECL cbmarch_parallel_burst_read_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);
EXTERN int          CBMAPIDECL cbmarch_parallel_burst_write_track(CBM_FILE HandleDevice, __u_char *Buffer, unsigned int Length);


EXTERN void         CBMAPIDECL cbmarch_s1_read_n    (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
EXTERN void         CBMAPIDECL cbmarch_s1_write_n   (CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);
EXTERN void         CBMAPIDECL cbmarch_s2_read_n    (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
EXTERN void         CBMAPIDECL cbmarch_s2_write_n   (CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);
EXTERN void         CBMAPIDECL cbmarch_pp_dc_read_n (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
EXTERN void         CBMAPIDECL cbmarch_pp_dc_write_n(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);
EXTERN void         CBMAPIDECL cbmarch_pp_cc_read_n (CBM_FILE HandleDevice,       unsigned char *data, unsigned int size);
EXTERN void         CBMAPIDECL cbmarch_pp_cc_write_n(CBM_FILE HandleDevice, const unsigned char *data, unsigned int size);

#if defined WIN32
EXTERN BOOL CBMAPIDECL cbm_install_complete(OUT PULONG Buffer, IN ULONG BufferLen);
/*! Function pointer for the cbm_i_driver_install() function */
typedef BOOL (CBMAPIDECL *P_CBM_INSTALL_COMPLETE)(OUT PULONG Buffer, IN ULONG BufferLen);
#endif

#endif // #ifndef ARCHLIB_H
