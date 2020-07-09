// vim: set expandtab tabstop=4 shiftwidth=4 autoindent smartindent:

#include "opencbm.h"

#include "arch.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(_x) ( sizeof (_x) / sizeof ((_x)[0]) )
#endif

#define IGNORE_MASK 0xEFu

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


#include "dump.bin"

enum {
    INDEX_RESULT1,
    INDEX_RESULT1_4485,
    INDEX_RESULT2,
    INDEX_RESULT2_4485,
    INDEX_LAST
};

static unsigned int test_baudrates[] = { /*0*/ 1, 1, 3, 4, 19, 39 };

static struct dump_s {
    unsigned int const   count;
    unsigned int         baudrate[ARRAYSIZE(test_baudrates)];
    unsigned char       *dump[INDEX_LAST][ARRAYSIZE(test_baudrates)];
} dumps = {
    ARRAYSIZE(test_baudrates),
};

static void create_all_compare_dumps()
{
    unsigned int baud_index;
    unsigned int index;

    for (baud_index = 0; baud_index < ARRAYSIZE(dumps.baudrate); baud_index++) {
        dumps.baudrate[baud_index] = test_baudrates[baud_index];
        for (index = 0; index < INDEX_LAST; index++) {
            dumps.dump[index][baud_index] = NULL;
        }
    }

    index = 0;
    dumps.dump[INDEX_RESULT1]     [index] = cia_sdr_icr_0_result1;
    dumps.dump[INDEX_RESULT2]     [index] = cia_sdr_icr_0_result2;
    dumps.dump[INDEX_RESULT1_4485][index] = cia_sdr_icr_4485_0_result1;
    dumps.dump[INDEX_RESULT2_4485][index] = cia_sdr_icr_4485_0_result2;

    index = 1;
    dumps.dump[INDEX_RESULT1]     [index] = cia_sdr_icr_1_7f_result1;
    dumps.dump[INDEX_RESULT2]     [index] = cia_sdr_icr_1_7f_result2;
    dumps.dump[INDEX_RESULT1_4485][index] = cia_sdr_icr_4485_1_7f_result1;
    dumps.dump[INDEX_RESULT2_4485][index] = cia_sdr_icr_4485_1_7f_result2;

    index = 2;
    dumps.dump[INDEX_RESULT1]     [index] = cia_sdr_icr_3_result1;
    dumps.dump[INDEX_RESULT2]     [index] = cia_sdr_icr_3_result2;
    dumps.dump[INDEX_RESULT1_4485][index] = cia_sdr_icr_4485_3_result1;
    dumps.dump[INDEX_RESULT2_4485][index] = cia_sdr_icr_4485_3_result2;

    index = 3;
    dumps.dump[INDEX_RESULT1]     [index] = cia_sdr_icr_4_7f_result1;
    dumps.dump[INDEX_RESULT2]     [index] = cia_sdr_icr_4_7f_result2;
    dumps.dump[INDEX_RESULT1_4485][index] = cia_sdr_icr_4485_4_7f_result1;
    dumps.dump[INDEX_RESULT2_4485][index] = cia_sdr_icr_4485_4_7f_result2;

    index = 4;
    dumps.dump[INDEX_RESULT1]     [index] = cia_sdr_icr_19_result1;
    dumps.dump[INDEX_RESULT2]     [index] = cia_sdr_icr_19_result2;
    dumps.dump[INDEX_RESULT1_4485][index] = cia_sdr_icr_4485_19_result1;
    dumps.dump[INDEX_RESULT2_4485][index] = cia_sdr_icr_4485_19_result2;

    index = 5;
    dumps.dump[INDEX_RESULT1]     [index] = cia_sdr_icr_39_result1;
    dumps.dump[INDEX_RESULT2]     [index] = cia_sdr_icr_39_result2;
    dumps.dump[INDEX_RESULT1_4485][index] = cia_sdr_icr_4485_39_result1;
    dumps.dump[INDEX_RESULT2_4485][index] = cia_sdr_icr_4485_39_result2;

    printf("DONE\n");
}

struct drive_functions_s {
    const unsigned char * irqdelay;
    const unsigned int    irqdelay_len;

    const unsigned char * irqdelay_oneshot;
    const unsigned int    irqdelay_oneshot_len;

    const unsigned char * cia_sdr_icr;
    const unsigned int    cia_sdr_icr_len;

    const unsigned int    cia_sdr_icr_startaddress;
    const unsigned int    cia_sdr_icr_startaddress_len;

    const unsigned int    cia_sdr_icr_jsr_swap_offset;

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
        0x0401,             // start of first block
        sizeof cia_sdr_icr_1571,   // length of first block
        0x1,                // where are the JSR to change between results and results2 (relative to start of first block)
        0x0500,             // results are found here
        0x0500,             // results2 are found here
        767                 // tottests
    },
    {
        irqdelay_1581,
        sizeof irqdelay_1581,
        irqdelay_1581_oneshot,
        sizeof irqdelay_1581_oneshot,
        cia_sdr_icr_1581,
        sizeof cia_sdr_icr_1581,
        0x0401,             // start of first block
        sizeof cia_sdr_icr_1581, // 0x04C7 - 0x0401,    // length of first block
        0,                  // where are the JSR to change between results and results2
        0x0500,             // results are found here
        0x0900,             // results2 are found here
        1000                // tottests
    },
};

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

void mod_and(unsigned char * buffer, unsigned int bufferlen, unsigned char mask)
{
    unsigned int i;
    for (i = 0; i < bufferlen; i++) {
        buffer[i] &= mask;
    }
}

unsigned int check_result(const unsigned char * buffer_in, unsigned int len, unsigned int baudrate, unsigned int index)
{
    unsigned int match = 0;

    unsigned int i;

    unsigned int baudrate_index = 0;

    for (baudrate_index = 0; baudrate_index < dumps.count; baudrate_index++) {
        if (dumps.baudrate[baudrate_index] == baudrate) {
            break;
        }
    }

    if (baudrate_index == dumps.count) return match;

    if (dumps.dump[index][baudrate_index] == NULL) return match;

//    memdump("result", buffer_in, len, 0x500);
//    memdump("compare", dumps.dump[index][baudrate_index], len, 0x500);

    match = 1;
    for (i = 0; i < len; i++) {
        if ( (dumps.dump[index][baudrate_index][i] & IGNORE_MASK) != ( buffer_in[i] & IGNORE_MASK) ) {
//            printf("DIFF: %04X: %02x != %02x\n", i, (dumps.dump[index][baudrate_index][i] & IGNORE_MASK), ( buffer_in[i] & IGNORE_MASK));
            match = 0;
        }
    }

//    match = 1;

    return match;
}

void test_cia_sdr(CBM_FILE fd, unsigned char drv, struct drive_functions_s *drive_functions)
{
    char execute_command[] = "M-E  ";
    const unsigned int drvaddress_baudrate = drive_functions->cia_sdr_icr_startaddress;
    const unsigned int drvaddress_start    = drvaddress_baudrate + 3;

    static unsigned char result[1000] = { 0 };
    static unsigned char result2[1000] = { 0 };

    static unsigned char jsrfunc_orig[6];
    static unsigned char jsrfunc_swapped[6];

    unsigned int baudindex;

    unsigned int jsr_swap_address = 0;

    printf("\n\nTest CIA SDR:\n");

    create_all_compare_dumps();

    execute_command[3] = drvaddress_start & 0xFFu;
    execute_command[4] = (drvaddress_start >> 8) & 0xFFu;

#if DBG_PROGRAM
    printf("Start address: %04X\n", drvaddress_start);
#endif

    if (drive_functions->cia_sdr_icr_jsr_swap_offset) {
        jsr_swap_address = drive_functions->cia_sdr_icr[drive_functions->cia_sdr_icr_jsr_swap_offset] + (drive_functions->cia_sdr_icr[drive_functions->cia_sdr_icr_jsr_swap_offset + 1] << 8);

        unsigned int offset = jsr_swap_address - drive_functions->cia_sdr_icr_startaddress;
        memcpy(jsrfunc_orig, &drive_functions->cia_sdr_icr[offset], 6);
        jsrfunc_swapped[0] = jsrfunc_orig[0];
        jsrfunc_swapped[1] = jsrfunc_orig[4];
        jsrfunc_swapped[2] = jsrfunc_orig[5];
        jsrfunc_swapped[3] = jsrfunc_orig[3];
        jsrfunc_swapped[4] = jsrfunc_orig[1];
        jsrfunc_swapped[5] = jsrfunc_orig[2];

#if DBG_PROGRAM
        memdump("orig jsrfunc", jsrfunc_orig, 6, jsr_swap_address);
        memdump("mod. jsrfunc", jsrfunc_swapped, 6, jsr_swap_address);
        printf("\n");
#endif
    }

    // initial upload of the program
    cbm_upload(
            fd,
            drv,
            drive_functions->cia_sdr_icr_startaddress,
            drive_functions->cia_sdr_icr,
            drive_functions->cia_sdr_icr_startaddress_len
            );
#if DBG_PROGRAM
    memdump("up1",
            drive_functions->cia_sdr_icr,
            drive_functions->cia_sdr_icr_startaddress_len,
            drive_functions->cia_sdr_icr_startaddress);
#endif

    for (baudindex = 0; baudindex < ARRAYSIZE(test_baudrates); baudindex++) {
        unsigned char baudrate[1];

        printf("\n- Test baudrate %u:\n", test_baudrates[baudindex]);

        baudrate[0] = test_baudrates[baudindex] & 0xFFu;

        cbm_upload(fd, drv, drvaddress_baudrate, baudrate, sizeof baudrate);

        if (jsr_swap_address) {
            cbm_upload(fd, drv, jsr_swap_address, jsrfunc_swapped, sizeof jsrfunc_swapped);
        }
        cbm_exec_command(fd, drv, execute_command, 5);

        // now, get the results from the drive
        cbm_download(fd, drv, drive_functions->cia_sdr_result_address,  result,  drive_functions->cia_sdr_tottest);


//printf("\nRESULT1:\n");
        if (check_result(result, drive_functions->cia_sdr_tottest, test_baudrates[baudindex], INDEX_RESULT1)) {
            printf("result for normal CIA\n");
        }
        else if (check_result(result, drive_functions->cia_sdr_tottest, test_baudrates[baudindex], INDEX_RESULT1_4485)) {
            printf("result for 4485 CIA\n");
        }
        else {
//          mod_and(result, drive_functions->cia_sdr_tottest, IGNORE_MASK);
            memdump("result",  result,  drive_functions->cia_sdr_tottest, drive_functions->cia_sdr_result_address);
        }

        if (jsr_swap_address) {
            cbm_upload(fd, drv, jsr_swap_address, jsrfunc_orig, sizeof jsrfunc_orig);
            cbm_exec_command(fd, drv, execute_command, 5);
        }

        if (drive_functions->cia_sdr_result2_address) {
            cbm_download(fd, drv, drive_functions->cia_sdr_result2_address, result2, drive_functions->cia_sdr_tottest);


//printf("\nRESULT2:\n");
            if (check_result(result2, drive_functions->cia_sdr_tottest, test_baudrates[baudindex], INDEX_RESULT2)) {
                printf("result2 for normal CIA\n");
            }
            else {
                if (check_result(result, drive_functions->cia_sdr_tottest, test_baudrates[baudindex], INDEX_RESULT2_4485)) {
                    printf("result2 for 4485 CIA\n");
                }
                else {
//                  mod_and(result2, drive_functions->cia_sdr_tottest, IGNORE_MASK);
                    memdump("result2", result2, drive_functions->cia_sdr_tottest, drive_functions->cia_sdr_result2_address);
                }
            }
        }
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
