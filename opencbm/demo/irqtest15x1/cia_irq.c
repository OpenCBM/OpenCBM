// vim: set expandtab tabstop=4 shiftwidth=4 autoindent smartindent:

#include "opencbm.h"

#include "arch.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(_x) ( sizeof (_x) / sizeof ((_x)[0]) )
#endif

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

static const unsigned char cia_sdr_icr_1571[] = {
#include "cia-sdr-icr-1571.inc"
};

static const unsigned char cia_sdr_icr_1581[] = {
#include "cia-sdr-icr-1581.inc"
};

struct drive_functions_s {
    const unsigned char * irqdelay;
    const unsigned int    irqdelay_len;

    const unsigned char * irqdelay_oneshot;
    const unsigned int    irqdelay_oneshot_len;

    const unsigned char * cia_sdr_icr;
    const unsigned int    cia_sdr_icr_len;

    const unsigned int    cia_sdr_icr_startaddress;
    const unsigned int    cia_sdr_icr_startaddress_len;

    const unsigned int    cia_sdr_icr_contaddress;
    const unsigned int    cia_sdr_icr_contaddress_len;

    const unsigned int    cia_sdr_result_address;
    const unsigned int    cia_sdr_result2_address;
    const unsigned int    cia_sdr_tottest;
};

static struct drive_functions_s drive_functions[] = {
    {
        irqdelay_1571,
        sizeof irqdelay_1571,
        irqdelay_1571_oneshot,
        sizeof irqdelay_1571_oneshot,
        cia_sdr_icr_1571,
        sizeof cia_sdr_icr_1571,
        0x0146,             // start of first block
        0x01A0 - 0x0146,    // length of first block
        0x0401,             // start of second block
        0x046d - 0x0401,    // length of second block
        0x046d,             // results are found here
        0x0636,             // results2 are found here
        457                 // tottests
    },
    {
        irqdelay_1581,
        sizeof irqdelay_1581,
        irqdelay_1581_oneshot,
        sizeof irqdelay_1581_oneshot,
        cia_sdr_icr_1581,
        sizeof cia_sdr_icr_1581,
        0x0401,             // start of first block
        0x04C7 - 0x0401,    // length of first block
        0x0,                // start of second block (none)
        0,                  // length of second block (none)
        0x04c7,             // results are found here
        0x08ae,             // results2 are found here
        1000 - 1            // tottests
    },
};

static unsigned int test_baudrates[] = { 4, 5, 6, 7, 8, 50, 51, 64 };

static unsigned int current_drive = 0;

void set_1571()
{
    current_drive = 0;
}

void set_1581()
{
    current_drive = 1;
}

#define DUMP_BYTE_PER_ROW 16

void memdump(const char * text, const unsigned char * buffer, unsigned int bufferlen, unsigned int offset)
{
    unsigned int row, col;
    printf("DUMP of %s:", text);

    for (row = 0; row < bufferlen; row += DUMP_BYTE_PER_ROW) {
        printf("\n%04X: ", offset + row);
        for (col = row; (col < bufferlen) && (col < row + DUMP_BYTE_PER_ROW); col++) {
            printf("%02X ", buffer[col]);
        }
    }
    printf("\n");
}


void test_cia_sdr(CBM_FILE fd, unsigned char drv, struct drive_functions_s *drive_functions)
{
    char execute_command[] = "M-E  ";
    const unsigned int drvaddress_baudrate = drive_functions->cia_sdr_icr_startaddress;
    const unsigned int drvaddress_start    = drvaddress_baudrate + 2;

    static unsigned char result[1000] = { 0 };
    static unsigned char result2[1000] = { 0 };

    unsigned int baudindex;

    printf("\n\nTest CIA SDR:\n");

    execute_command[3] = drvaddress_start & 0xFFu;
    execute_command[4] = (drvaddress_start >> 8) & 0xFFu;

    // initial upload of the program
    cbm_upload(
            fd,
            drv,
            drive_functions->cia_sdr_icr_startaddress,
            drive_functions->cia_sdr_icr,
            drive_functions->cia_sdr_icr_startaddress_len
            );
    memdump("up1",
            drive_functions->cia_sdr_icr,
            drive_functions->cia_sdr_icr_startaddress_len,
            drive_functions->cia_sdr_icr_startaddress);

    if (drive_functions->cia_sdr_icr_contaddress_len) {
        cbm_upload(
                fd,
                drv,
                drive_functions->cia_sdr_icr_contaddress,
                drive_functions->cia_sdr_icr + drive_functions->cia_sdr_icr_startaddress_len,
                drive_functions->cia_sdr_icr_contaddress_len
                );
        memdump("up2",
                drive_functions->cia_sdr_icr + drive_functions->cia_sdr_icr_startaddress_len,
                drive_functions->cia_sdr_icr_contaddress_len,
                drive_functions->cia_sdr_icr_contaddress);
    }

    for (baudindex = 0; baudindex < ARRAYSIZE(test_baudrates); baudindex++) {
        unsigned char baudrate[2];

        printf("\n- Test baudrate %u:\n", test_baudrates[baudindex]);

        baudrate[0] = test_baudrates[baudindex] & 0xFFu;
        baudrate[1] = (test_baudrates[baudindex] >> 8) & 0xFFu;

        cbm_upload(fd, drv, drvaddress_baudrate, baudrate, sizeof baudrate);

        cbm_exec_command(fd, drv, execute_command, 5);

        // now, get the results from the drive
        cbm_download(fd, drv, drive_functions->cia_sdr_result_address,  result,  drive_functions->cia_sdr_tottest);
        cbm_download(fd, drv, drive_functions->cia_sdr_result2_address, result2, drive_functions->cia_sdr_tottest);

        memdump("result",  result,  drive_functions->cia_sdr_tottest, 0);
        memdump("result2", result2, drive_functions->cia_sdr_tottest, 0);
    }
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
    test_irqdelay(fd, drv, drive_functions[current_drive].irqdelay,         drive_functions[current_drive].irqdelay_len,         "irqdelay");
    test_irqdelay(fd, drv, drive_functions[current_drive].irqdelay_oneshot, drive_functions[current_drive].irqdelay_oneshot_len, "irqdelay_oneshot");

    test_cia_sdr(fd, drv, &drive_functions[current_drive]);

    // reset the drive
    cbm_exec_command(fd, drv, "UJ:", 0);
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
