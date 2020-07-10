#ifndef OPENCBM_H_
#define OPENCBM_H_

#include "arch.h"

#define ARCH_MAINDECL

typedef int CBM_FILE;

enum cbm_device_type_e {
    cbm_dt_unknown, cbm_dt_cbm1570, cbm_dt_cbm1571, cbm_dt_cbm1581
};

int  cbm_driver_open_ex(CBM_FILE *f, char * adapter);
void cbm_driver_close(CBM_FILE f);
int  cbm_exec_command(CBM_FILE f, unsigned char dev, unsigned char *cmd, size_t len);
int  cbm_identify(CBM_FILE f, unsigned char drv, enum cbm_device_type_e *t, const char **type_str);
int  cbm_upload(CBM_FILE f, unsigned char dev, int adr, const void *prog, size_t size);
int  cbm_download(CBM_FILE f, unsigned char dev, int adr, void *dbuf, size_t size);


#endif /* #ifndef OPENCBM_H_ */
