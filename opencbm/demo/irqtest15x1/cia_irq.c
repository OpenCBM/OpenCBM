// vim: set expandtab tabstop=4 shiftwidth=4 autoindent smartindent:

#include "opencbm.h"

#include "arch.h"

#include <stdio.h>
#include <stdlib.h>

static const unsigned char irqdelay_1571[] = {
#include "irqdelay-1571.inc"
};

static const unsigned char irqdelay_1571_oneshot[] = {
#include "irqdelay-1571-oneshot.inc"
};

static const unsigned char irqdelay_1581[] = {
#include "irqdelay-1581.inc"
};

static const unsigned char irqdelay_1581_oneshot[] = {
#include "irqdelay-1581-oneshot.inc"
};

static const unsigned char * irqdelay_func             = 0;
static       unsigned int    irqdelay_func_len         = 0;

static const unsigned char * irqdelay_oneshot_func     = 0;
static       unsigned int    irqdelay_oneshot_func_len = 0;

void set_1571()
{
    irqdelay_func             = irqdelay_1571;
    irqdelay_func_len         = sizeof(irqdelay_1571);

    irqdelay_oneshot_func     = irqdelay_1571_oneshot;
    irqdelay_oneshot_func_len = sizeof(irqdelay_1571_oneshot);
}

void set_1581()
{
    irqdelay_func             = irqdelay_1581;
    irqdelay_func_len         = sizeof(irqdelay_1581);

    irqdelay_oneshot_func     = irqdelay_1581_oneshot;
    irqdelay_oneshot_func_len = sizeof(irqdelay_1581_oneshot);
}

void test_irqdelay(CBM_FILE fd, unsigned char drv, const unsigned char * func, unsigned int func_len, char * str)
{
    char result[2];
    unsigned int count;

    printf("\n\n%s:\n", str);

    cbm_upload(fd, drv, 0x0500, func, func_len);

    for (count = 0; count < 10; count++) {
        char * cia_str = "new CIA";

        cbm_exec_command(fd, drv, "U3:", 0);
        cbm_download(fd, drv, 0x0300, result, sizeof(result));

        if (result[0]) cia_str = "old CIA";

        printf("%4d: Result is: %02X (%s), %02X\n", count, (unsigned int) result[0], cia_str, (unsigned int) result[0]);
    };
}

void testdrive(CBM_FILE fd, unsigned char drv)
{
    test_irqdelay(fd, drv, irqdelay_func,         irqdelay_func_len,         "irqdelay");
    test_irqdelay(fd, drv, irqdelay_oneshot_func, irqdelay_oneshot_func_len, "irqdelay_oneshot");
}


int ARCH_MAINDECL main(int argc, char *argv[])
{
    unsigned char drv = argc > 1 ? arch_atoc(argv[1]) : 8;
    CBM_FILE fd;

    if(cbm_driver_open_ex(&fd, NULL) == 0)
    {
        enum cbm_device_type_e devicetype = cbm_dt_unknown;
        const char *drivetype_str;

        if (cbm_identify(fd, drv, &devicetype, &drivetype_str)) {
            printf("Could not identify drive! Assuming 1571...\n");
        }
        else {
            printf("Identified drive %d: as %s\n", drv, drivetype_str);

            switch (devicetype) {
                case cbm_dt_cbm1570:
                case cbm_dt_cbm1571:
                    set_1571();
                    break;

                case cbm_dt_cbm1581:
                    set_1581();
                    break;

                default:
                    printf("Device does not contain a CIA 6526, aborting...\n");
                    exit(1);
            };
        }

        testdrive(fd, drv);

        cbm_driver_close(fd);
        return 0;
    }
    return 1;
}
