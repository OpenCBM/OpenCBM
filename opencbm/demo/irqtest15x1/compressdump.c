// vim: set expandtab tabstop=4 shiftwidth=4 autoindent smartindent:

#include <stdio.h>

#include "dump.bin"

#define ARRAYSIZE(_x) (sizeof (_x) / sizeof (_x)[0])

struct buffer_s {
    char          *name;
    unsigned char *raw_values;
    unsigned int   compressed_length;
    unsigned char  compressed_values[2000];
};

struct buffer_s buffers[] = {
    { "cia_sdr_icr_0_7f_result1",         cia_sdr_icr_0_7f_result1 },
    { "cia_sdr_icr_0_7f_result2",         cia_sdr_icr_0_7f_result2 },
    { "cia_sdr_icr_0_result1",            cia_sdr_icr_0_result1 },
    { "cia_sdr_icr_0_result2",            cia_sdr_icr_0_result2 },
    { "cia_sdr_icr_1_7f_result1",         cia_sdr_icr_1_7f_result1 },
    { "cia_sdr_icr_1_7f_result2",         cia_sdr_icr_1_7f_result2 },
    { "cia_sdr_icr_19_result1",           cia_sdr_icr_19_result1 },
    { "cia_sdr_icr_19_result2",           cia_sdr_icr_19_result2 },
    { "cia_sdr_icr_39_result1",           cia_sdr_icr_39_result1 },
    { "cia_sdr_icr_39_result2",           cia_sdr_icr_39_result2 },
    { "cia_sdr_icr_3_result1",            cia_sdr_icr_3_result1 },
    { "cia_sdr_icr_3_result2",            cia_sdr_icr_3_result2 },
    { "cia_sdr_icr_4485_0_7f_result1",    cia_sdr_icr_4485_0_7f_result1 },
    { "cia_sdr_icr_4485_0_7f_result2",    cia_sdr_icr_4485_0_7f_result2 },
    { "cia_sdr_icr_4485_0_result1",       cia_sdr_icr_4485_0_result1 },
    { "cia_sdr_icr_4485_0_result2",       cia_sdr_icr_4485_0_result2 },
    { "cia_sdr_icr_4485_1_7f_result1",    cia_sdr_icr_4485_1_7f_result1 },
    { "cia_sdr_icr_4485_1_7f_result2",    cia_sdr_icr_4485_1_7f_result2 },
    { "cia_sdr_icr_4485_19_result1",      cia_sdr_icr_4485_19_result1 },
    { "cia_sdr_icr_4485_19_result2",      cia_sdr_icr_4485_19_result2 },
    { "cia_sdr_icr_4485_39_result1",      cia_sdr_icr_4485_39_result1 },
    { "cia_sdr_icr_4485_39_result2",      cia_sdr_icr_4485_39_result2 },
    { "cia_sdr_icr_4485_3_result1",       cia_sdr_icr_4485_3_result1 },
    { "cia_sdr_icr_4485_3_result2",       cia_sdr_icr_4485_3_result2 },
    { "cia_sdr_icr_4485_4_7f_result1",    cia_sdr_icr_4485_4_7f_result1 },
    { "cia_sdr_icr_4485_4_7f_result2",    cia_sdr_icr_4485_4_7f_result2 },
    { "cia_sdr_icr_4_7f_result1",         cia_sdr_icr_4_7f_result1 },
    { "cia_sdr_icr_4_7f_result2",         cia_sdr_icr_4_7f_result2 },
    { "cia_sdr_icr_generic_0_7f_result1", cia_sdr_icr_generic_0_7f_result1 },
    { "cia_sdr_icr_generic_0_7f_result2", cia_sdr_icr_generic_0_7f_result2 },
    { "cia_sdr_icr_generic_0_result1",    cia_sdr_icr_generic_0_result1 },
    { "cia_sdr_icr_generic_0_result2",    cia_sdr_icr_generic_0_result2 },
    { "cia_sdr_icr_generic_1_7f_result1", cia_sdr_icr_generic_1_7f_result1 },
    { "cia_sdr_icr_generic_1_7f_result2", cia_sdr_icr_generic_1_7f_result2 },
    { "cia_sdr_icr_generic_19_result1",   cia_sdr_icr_generic_19_result1 },
    { "cia_sdr_icr_generic_19_result2",   cia_sdr_icr_generic_19_result2 },
    { "cia_sdr_icr_generic_39_result1",   cia_sdr_icr_generic_39_result1 },
    { "cia_sdr_icr_generic_39_result2",   cia_sdr_icr_generic_39_result2 },
    { "cia_sdr_icr_generic_3_result1",    cia_sdr_icr_generic_3_result1 },
    { "cia_sdr_icr_generic_3_result2",    cia_sdr_icr_generic_3_result2 },
    { "cia_sdr_icr_generic_4_7f_result1", cia_sdr_icr_generic_4_7f_result1 },
    { "cia_sdr_icr_generic_4_7f_result2", cia_sdr_icr_generic_4_7f_result2 }
};

void compress(struct buffer_s * data)
{
    unsigned int i;
    unsigned int writeptr = 0;
    unsigned int length = 1000;

    printf("Processing %s.\n", data->name);

    for (i = 0; i < length; i++) {
        unsigned char current_value = data->raw_values[i];
        unsigned int count = 1;
        unsigned int nextvalues_index;

        if (i + 1 < length) {
            for (nextvalues_index = i + 1; nextvalues_index < length; nextvalues_index++) {
                if (data->raw_values[nextvalues_index] == current_value) {
                    ++count;
                }
                else {
                    break;
                }
            };
        }

        if (count > 1) {
            while (count > 0) {
                unsigned int localcount = count;
                if (localcount > 0x7e) localcount = 0x7e;
                data->compressed_values[writeptr++] = 0x80 | (localcount - 1);
                i = nextvalues_index - 1;
                count -= localcount;
                data->compressed_values[writeptr++] = data->raw_values[i];
            }
        }
        else {
            data->compressed_values[writeptr++] = data->raw_values[i];
        }
    }

    data->compressed_length = writeptr;
}

void output(struct buffer_s * data, FILE * foutput)
{
    unsigned int i;

    fprintf(foutput, "\nunsigned char %s_compressed[] = { /* %d */ ", data->name, data->compressed_length);

    for (i = 0; i < data->compressed_length; i++) {
        if (i % 16 == 0) fprintf(foutput, "\n");
//        if (i % 16 == 0) fprintf(foutput, "\n/* %04x */", i);
//        if (i % 4 == 0) fprintf(foutput, " ");
        fprintf(foutput, "0x%02x, ", data->compressed_values[i]);
    }

    fprintf(foutput, "0xFF\n};\n");
}

int main(void)
{
    FILE * foutput = NULL;
    unsigned int dataindex;

    foutput = fopen("dump_compressed.bin", "w");
    if (!foutput) {
        return 1;
    }

    for (dataindex = 0; dataindex < ARRAYSIZE(buffers); dataindex++) {
        compress(&buffers[dataindex]);
        output(&buffers[dataindex], foutput);
    }

    if (foutput) fclose(foutput);

    return 0;
}
