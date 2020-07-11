#ifndef OPENCBM_H_
#define OPENCBM_H_

#include "arch.h"

#define ARCH_MAINDECL

typedef int CBM_FILE;

enum cbm_device_type_e
{
    cbm_dt_unknown = -1, /*!< The device could not be identified */
    cbm_dt_cbm1541,      /*!< The device is a VIC 1541 */
    cbm_dt_cbm1570,      /*!< The device is a VIC 1570 */
    cbm_dt_cbm1571,      /*!< The device is a VIC 1571 */
    cbm_dt_cbm1581,      /*!< The device is a VIC 1581 */
    cbm_dt_cbm2040,      /*!< The device is a CBM-2040 DOS1 or 2   */
    cbm_dt_cbm2031,      /*!< The device is a CBM-2031 DOS2.6      */
    cbm_dt_cbm3040,      /*!< The device is a CBM-3040 DOS1 or 2   */
    cbm_dt_cbm4040,      /*!< The device is a CBM-4040 DOS2        */
    cbm_dt_cbm4031,      /*!< The device is a CBM-4031 DOS2.6      */
    cbm_dt_cbm8050,      /*!< The device is a CBM-8050             */
    cbm_dt_cbm8250,      /*!< The device is a CBM-8250 or SFD-1001 */
    cbm_dt_sfd1001       /*!< The device is a SFD-1001             */
};

int  cbm_driver_open_ex(CBM_FILE *, char * adapter);
void cbm_driver_close(CBM_FILE);
int  cbm_exec_command(CBM_FILE, unsigned char dev, unsigned char *cmd, size_t len);
int  cbm_identify(CBM_FILE, unsigned char drv, enum cbm_device_type_e *t, const char **type_str);
int  cbm_upload(CBM_FILE, unsigned char dev, int adr, const void *prog, size_t size);
int  cbm_download(CBM_FILE, unsigned char dev, int adr, void *dbuf, size_t size);

int  cbm_listen(CBM_FILE, unsigned char dev, unsigned char secondary);
void cbm_unlisten(CBM_FILE);
int  cbm_talk(CBM_FILE, unsigned char dev, unsigned char secondary);
void cbm_untalk(CBM_FILE);
int  cbm_raw_write(CBM_FILE, unsigned char *buffer, size_t len);
int  cbm_raw_read(CBM_FILE, unsigned char *buffer, size_t len);

#endif /* #ifndef OPENCBM_H_ */
