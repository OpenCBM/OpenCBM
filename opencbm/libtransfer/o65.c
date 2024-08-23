/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2006, 2009, 2024 Spiro Trikaliotis
*/

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "LIBTRANSFER.DLL"

#include "debug.h"

#include "o65.h"
#include "helper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#undef DBG
#if DBG
 #define DBG_O65_SHOW(_x_)              DBG_PRINT(_x_)
 #define DBG_O65_MEMDUMP(_x_, _y_, _z_) DBG_MEMDUMP(_x_, _y_, _z_)
#else
 #define DBG_O65_SHOW(_x_)
 #define DBG_O65_MEMDUMP(_x_, _y_, _z_)
#endif

// #define DBG_DETAILS_RELOCTABLE 1

#if DBG_DETAILS_RELOCTABLE
 #define DBG_O65_RELOCTABLE(_x) _x
#else
 #define DBG_O65_RELOCTABLE(_x)
#endif


typedef
struct undefined_references_s {
    char    *name;
} undefined_references_t;

typedef
struct exported_symbols_s {
    char    *name;
    uint8_t  segmentid;
    uint8_t  value;
} exported_symbols_t;

typedef enum {
    SEGMENT_TEXT,
    SEGMENT_DATA
} segment_e;

typedef
struct relocation_entry_s {
    segment_e segment;
    uint32_t  offset;
    uint8_t   type;
    uint8_t   segmentid;
    uint16_t  lower_bytes; /* on type HIGH the lower byte, and on type SEG the two lower bytes */
    uint32_t  index;
} relocation_entry_t;


typedef
struct o65_file_s
{
    uint8_t                    *raw_buffer;
    size_t                      raw_buffer_length;
    size_t                      readpointer;

    uint16_t                    o65mode;
    int                         mode32;
    int                         pagewiserelocation;

    uint32_t                    text_segment_base;
    uint32_t                    text_segment_length;
    size_t                      text_segment_pointer;

    uint32_t                    data_segment_base;
    uint32_t                    data_segment_length;
    size_t                      data_segment_pointer;

    uint32_t                    bss_segment_base;
    uint32_t                    bss_segment_length;

    uint32_t                    zero_segment_base;
    uint32_t                    zero_segment_length;

    uint32_t                    min_stack_size;

    size_t                      option_filename_pointer;
    size_t                      option_os_pointer;
    size_t                      option_assembler_pointer;
    size_t                      option_author_pointer;
    size_t                      option_creation_pointer;

    size_t                      undefined_references_count;
    size_t                      undefined_references_pointer;

    size_t                      relocation_text_pointer;
    size_t                      relocation_data_pointer;

    size_t                      exported_globals_count;
    size_t                      exported_globals_pointer;

    undefined_references_t     *undefined_references_table;
    exported_symbols_t         *exported_symbols_table;
} o65_file_t;

#define O65_FILE_HEADER_MARKER            0x0001
#define O65_FILE_HEADER_MAGIC           0x35366F
#define O65_FILE_HEADER_VERSION_MAX            0

#define O65_FILE_HEADER_MODE_65816        0x8000 /* if set, uses 65816 code; if not set, uses 6502 */
#define O65_FILE_HEADER_MODE_PAGERELOC    0x4000 /* pagewise relocation allowed only */
#define O65_FILE_HEADER_MODE_SIZE32       0x2000 /* size is 32 bit; else, it is 16 bit */
#define O65_FILE_HEADER_MODE_OBJFILE      0x1000 /* if defined, this is an OBJ file; else, it is an executable */
#define O65_FILE_HEADER_MODE_SIMPLE       0x0800 /* if defined, this file uses the "simple" approach (new for v1.3) */
#define O65_FILE_HEADER_MODE_CHAIN        0x0400 /* if set, another file follows this one */
#define O65_FILE_HEADER_MODE_BSSZERO      0x0200 /* if set, the bss segment must be zeroed out */

#define O65_FILE_HEADER_MODE_UNUSED       0x010C /* these bits should all be 0 */

#define O65_FILE_HEADER_MODE_CPU_MASK     0x00F0 /* mask for getting the CPU */
#define O65_FILE_HEADER_MODE_CPU_6502     0x0000 /* CPU is a 6502 core, no illegal opcodes */
#define O65_FILE_HEADER_MODE_CPU_65C02    0x0010 /* CPU is a 65C02 core w/ some bugfix, no illegal opcodes */
#define O65_FILE_HEADER_MODE_CPU_65SC02   0x0020 /* CPU is a 65SC02 core (enhanced 65C02), some new opcodes */
#define O65_FILE_HEADER_MODE_CPU_65CE02   0x0030 /* CPU is a 65CE02 core some 16 bit ops/branches, Z register modifiable */
#define O65_FILE_HEADER_MODE_CPU_NMOS6502 0x0040 /* CPU is an NMOS 6502 core (including undocumented opcodes) */
#define O65_FILE_HEADER_MODE_CPU_65816    0x0050 /* CPU is a 65816 in 6502 emulation mode */

#define O65_FILE_HEADER_MODE_ALIGN_MASK   0x0003 /* mask for getting the alignment */
#define O65_FILE_HEADER_MODE_ALIGN_BYTE   0x0000 /* alignment on arbitry bytes */
#define O65_FILE_HEADER_MODE_ALIGN_WORD   0x0001 /* alignment on double-byte boundaries */
#define O65_FILE_HEADER_MODE_ALIGN_QUAD   0x0002 /* alignment on quad-byte boundaries */
#define O65_FILE_HEADER_MODE_ALIGN_PAGE   0x0003 /* alignmont on pages (256 byte) only */


#define O65_FILE_HEADER_OHEADER_TYPE_FILENAME   0
#define O65_FILE_HEADER_OHEADER_TYPE_OS_SYSTEM  1
#define O65_FILE_HEADER_OHEADER_TYPE_ASSEMBLER  2
#define O65_FILE_HEADER_OHEADER_TYPE_AUTHOR     3
#define O65_FILE_HEADER_OHEADER_TYPE_CREATION   4

#define O65_FILE_HEADER_OHEADER_TYPE_OS_OSA65   1
#define O65_FILE_HEADER_OHEADER_TYPE_OS_LUNIX   2
#define O65_FILE_HEADER_OHEADER_TYPE_OS_CC65    3
#define O65_FILE_HEADER_OHEADER_TYPE_OS_OPENCBM 4


#define O65_FILE_RELOC_SEGMTYPEBYTE_UNDEF_MASK  0x18
#define O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_MASK   0x07
#define O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_MASK   0xE0

#define O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_UNDEF  0x00
#define O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_ABS    0x01
#define O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_TEXT   0x02
#define O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_DATA   0x03
#define O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_BSS    0x04
#define O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_ZERO   0x05

#define O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_WORD   0x80
#define O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_HIGH   0x40
#define O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_LOW    0x20
#define O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_SEGADR 0xc0
#define O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_SEG    0xa0

static o65_file_t *
o65_file_alloc(uint8_t *Buffer, size_t BufferLength)
{
    o65_file_t * file = NULL;

    FUNC_ENTER();

    file = malloc(sizeof *file + BufferLength);
    if (file) {
        file->raw_buffer        = (uint8_t*) (file + 1);
        if (Buffer) {
            memcpy(file->raw_buffer, Buffer, BufferLength);
        }
        file->raw_buffer_length = BufferLength;
        file->readpointer       = 0;
        file->mode32            = -1;
    }

    FUNC_LEAVE_PTR(file, o65_file_t *);
}

static void
o65_file_free(o65_file_t * File)
{
    FUNC_ENTER();

    if (File) {
        free(File->undefined_references_table);
        free(File->exported_symbols_table);
        free(File);
    }

    FUNC_LEAVE();
}

static O65ERR
o65_file_parse_skip(o65_file_t * File, size_t SkipCount)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    FUNC_ENTER();

    DBG_ASSERT(File != NULL);
    DBG_ASSERT(File->raw_buffer != NULL);
    DBG_ASSERT(File->raw_buffer_length > 0);
    DBG_ASSERT(File->readpointer <= File->raw_buffer_length - SkipCount);

    if (File->readpointer <= File->raw_buffer_length - SkipCount) {
        File->readpointer += SkipCount;
        error = O65ERR_SUCCESS;
    }

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_uint8(o65_file_t * File, uint8_t * Destination)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    FUNC_ENTER();

    DBG_ASSERT(File != NULL);
    DBG_ASSERT(Destination != NULL);
    DBG_ASSERT(File->raw_buffer != NULL);
    DBG_ASSERT(File->raw_buffer_length > 0);
    DBG_ASSERT(File->readpointer <= File->raw_buffer_length - sizeof(uint8_t));

    if (File->readpointer <= File->raw_buffer_length - sizeof(uint8_t)) {
        *Destination = File->raw_buffer[File->readpointer++];
        error = O65ERR_SUCCESS;
    }

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_uint16(o65_file_t * File, uint16_t * Destination)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    uint8_t value_low;
    uint8_t value_high;

    FUNC_ENTER();

    DBG_ASSERT(File != NULL);
    DBG_ASSERT(Destination != NULL);
    DBG_ASSERT(File->raw_buffer != NULL);
    DBG_ASSERT(File->raw_buffer_length > 0);
    DBG_ASSERT(File->readpointer <= File->raw_buffer_length - sizeof(uint16_t));

    do {
        error = o65_file_parse_uint8(File, &value_low);
        if (error) {
            break;
        }

        error = o65_file_parse_uint8(File, &value_high);
        if (error) {
            break;
        }

        *Destination = value_low | (value_high << 8);

    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_uint24(o65_file_t * File, uint32_t * Destination)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    uint16_t value_low;
    uint8_t value_high;

    FUNC_ENTER();

    DBG_ASSERT(File != NULL);
    DBG_ASSERT(Destination != NULL);
    DBG_ASSERT(File->raw_buffer != NULL);
    DBG_ASSERT(File->raw_buffer_length > 0);
    DBG_ASSERT(File->readpointer <= File->raw_buffer_length - sizeof(uint16_t) - sizeof(uint8_t));

    do {
        error = o65_file_parse_uint16(File, &value_low);
        if (error) {
            break;
        }

        error = o65_file_parse_uint8(File, &value_high);
        if (error) {
            break;
        }

        *Destination = value_low | (value_high << 16);

    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_uint32(o65_file_t * File, uint32_t * Destination)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    uint16_t value_low;
    uint16_t value_high;

    FUNC_ENTER();

    DBG_ASSERT(File != NULL);
    DBG_ASSERT(Destination != NULL);
    DBG_ASSERT(File->raw_buffer != NULL);
    DBG_ASSERT(File->raw_buffer_length > 0);
    DBG_ASSERT(File->readpointer <= File->raw_buffer_length - sizeof(uint32_t));

    do {
        error = o65_file_parse_uint16(File, &value_low);
        if (error) {
            break;
        }

        error = o65_file_parse_uint16(File, &value_high);
        if (error) {
            break;
        }

        *Destination = value_low | (value_high << 16);

    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_size(o65_file_t * File, uint32_t * Destination)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    uint16_t value16;
    uint32_t value32;

    FUNC_ENTER();

    DBG_ASSERT(File != NULL);
    DBG_ASSERT(Destination != NULL);
    DBG_ASSERT(File->raw_buffer != NULL);
    DBG_ASSERT(File->raw_buffer_length > 0);
    DBG_ASSERT(File->mode32 >= 0);

    do {
        if (File->mode32) {
            error = o65_file_parse_uint32(File, &value32);
            if (error) {
                break;
            }
        }
        else {
            error = o65_file_parse_uint16(File, &value16);
            if (error) {
                break;
            }
            value32 = value16;
        }

        *Destination = value32;

    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_string(o65_file_t * File, char **Destination)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    size_t pointer;

    FUNC_ENTER();

    DBG_ASSERT(File != NULL);
    DBG_ASSERT(Destination != NULL);
    DBG_ASSERT(File->raw_buffer != NULL);
    DBG_ASSERT(File->raw_buffer_length > 0);
    DBG_ASSERT(File->mode32 >= 0);

    do {
        pointer = File->readpointer;

        error = O65ERR_SUCCESS;
        while (File->raw_buffer[File->readpointer++]) {
            if (File->readpointer >= File->raw_buffer_length) {
                error = O65ERR_STRING_ERROR;
                break;
            }
        }
        if (error) {
            break;
        }

        *Destination = (char*) &File->raw_buffer[pointer];
    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_one_option(o65_file_t * File)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    size_t       *option_pointer   = NULL;
    unsigned int  option_is_string = 0;

    uint8_t       option_length;
    uint8_t       option_type;

    FUNC_ENTER();

    DBG_ASSERT(File != NULL);
    DBG_ASSERT(File->raw_buffer != NULL);
    DBG_ASSERT(File->raw_buffer_length > 0);
    DBG_ASSERT(File->mode32 >= 0);

    do {
        error = o65_file_parse_uint8(File, &option_length);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "OPTION:"));
        DBG_O65_SHOW((DBG_PREFIX "* LENGTH: $%02X", option_length));

        if (option_length == 0) {
            error = O65ERR_NO_MORE_DATA;
            break;
        }
        if (option_length == 1) {
            /* this is a weird option, but allow it anyway */
            error = O65ERR_SUCCESS;
        }

        error = o65_file_parse_uint8(File, &option_type);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "* TYPE:   $%02X", option_type));

        switch (option_type) {
            case O65_FILE_HEADER_OHEADER_TYPE_FILENAME:
                /* filename */
                option_pointer = &File->option_filename_pointer;
                option_is_string = 1;
                break;

            case O65_FILE_HEADER_OHEADER_TYPE_OS_SYSTEM:
                /* OS info */
                option_pointer = &File->option_os_pointer;
                break;

            case O65_FILE_HEADER_OHEADER_TYPE_ASSEMBLER:
                /* assembler */
                option_pointer = &File->option_assembler_pointer;
                option_is_string = 1;
                break;

            case O65_FILE_HEADER_OHEADER_TYPE_AUTHOR:
                /* author */
                option_pointer = &File->option_author_pointer;
                option_is_string = 1;
                break;

            case O65_FILE_HEADER_OHEADER_TYPE_CREATION:
                /* creation date */
                option_pointer = &File->option_creation_pointer;
                option_is_string = 1;
                break;

            default:
                /* unknown option */
                option_pointer = NULL;
                break;
        }

        if (*option_pointer != 0) {
            DBG_O65_SHOW((DBG_PREFIX "* Option %d given twice!", option_type));
        }


#if 0
        {
            DBG_O65_SHOW((DBG_PREFIX "* OPTION: "));
            unsigned int bytecount;
            uint8_t      option_byte;

            for (bytecount = 2; bytecount < option_length; ++bytecount) {
                error = o65_file_parse_uint8(File, &option_byte);
                if (error) {
                    break;
                }
                DBG_O65_SHOW((DBG_PREFIX " $%02X,", option_byte));
            }
            DBG_O65_SHOW((DBG_PREFIX ""));
        }
#else
        if (option_is_string) {
            size_t current_readpointer = File->readpointer;
            uint8_t terminator;

            if (option_length < 3) {
                error = O65ERR_OPTIONAL_HEADER_TOO_SHORT;
                break;
            }
            error = o65_file_parse_skip(File, option_length - 2 - 1);
            if (error) {
                break;
            }
            error = o65_file_parse_uint8(File, &terminator);
            if (error) {
                break;
            }
            if (terminator != 0) {
                error = O65ERR_OPTIONAL_HEADER_NOT_TERMINATED;
                break;
            }
            *option_pointer = current_readpointer;
        }
        else {
            *option_pointer = File->readpointer - 2;
            error = o65_file_parse_skip(File, option_length - 2);
        }
#endif
        if (error) {
            break;
        }
    } while (0);

    FUNC_LEAVE_INT(error);
}



#if DBG
void DEBUG_O65_SHOW_O65MODE(uint16_t O65mode)
{
    char *cpu = NULL;
    char *reloc = NULL;
    char *size = NULL;
    char *obj = NULL;
    char *simple = NULL;
    char *chain = NULL;
    char *bsszero = NULL;
    char *cpu2 = NULL;
    char *align = NULL;
    char unused[6];

    FUNC_ENTER();

    cpu     = O65mode & O65_FILE_HEADER_MODE_65816     ? "65816" : "6502";
    reloc   = O65mode & O65_FILE_HEADER_MODE_PAGERELOC ? "page" : "byte";
    size    = O65mode & O65_FILE_HEADER_MODE_SIZE32    ? "32 bit" : "16 bit";
    obj     = O65mode & O65_FILE_HEADER_MODE_OBJFILE   ? "objectfile" : "executable";
    simple  = O65mode & O65_FILE_HEADER_MODE_SIMPLE    ? "simple file addresses" : "-";
    chain   = O65mode & O65_FILE_HEADER_MODE_CHAIN     ? "another file follows" : "-";
    bsszero = O65mode & O65_FILE_HEADER_MODE_BSSZERO   ? "BSS must be zeroed" : "-";

    switch (O65mode & O65_FILE_HEADER_MODE_CPU_MASK) {
        case O65_FILE_HEADER_MODE_CPU_6502:     cpu2 = "6502 (PURE)"; break;
        case O65_FILE_HEADER_MODE_CPU_65C02:    cpu2 = "65C02";       break;
        case O65_FILE_HEADER_MODE_CPU_65SC02:   cpu2 = "65SC02";      break;
        case O65_FILE_HEADER_MODE_CPU_65CE02:   cpu2 = "65CE02";      break;
        case O65_FILE_HEADER_MODE_CPU_NMOS6502: cpu2 = "6502 (NMOS)"; break;
        case O65_FILE_HEADER_MODE_CPU_65816:    cpu2 = "65816";       break;
        default:
            static char cpu2buffer[20];
            sprintf(cpu2buffer, "unknown CPU2: %04X", O65mode & O65_FILE_HEADER_MODE_CPU_MASK);
    }

    switch (O65mode & O65_FILE_HEADER_MODE_ALIGN_MASK) {
        case O65_FILE_HEADER_MODE_ALIGN_BYTE:   align = "byte"; break;
        case O65_FILE_HEADER_MODE_ALIGN_WORD:   align = "word"; break;
        case O65_FILE_HEADER_MODE_ALIGN_QUAD:   align = "quad"; break;
        case O65_FILE_HEADER_MODE_ALIGN_PAGE:   align = "page"; break;
    }

    sprintf(unused, "%04X", O65mode & O65_FILE_HEADER_MODE_UNUSED);

    DBG_O65_SHOW((DBG_PREFIX "- CPU:             %s", cpu));
    DBG_O65_SHOW((DBG_PREFIX "- RELOC:           %s", reloc));
    DBG_O65_SHOW((DBG_PREFIX "- SIZE:            %s", size));
    DBG_O65_SHOW((DBG_PREFIX "- OBJ:             %s", obj));
    DBG_O65_SHOW((DBG_PREFIX "- SIMPLE:          %s", simple));
    DBG_O65_SHOW((DBG_PREFIX "- CHAIN:           %s", chain));
    DBG_O65_SHOW((DBG_PREFIX "- BSSZERO:         %s", bsszero));
    DBG_O65_SHOW((DBG_PREFIX "- CPU2:            %s", cpu2));
    DBG_O65_SHOW((DBG_PREFIX "- ALIGN:           %s", align));
    DBG_O65_SHOW((DBG_PREFIX "- UNUSED:          $%s", unused));

    FUNC_LEAVE();
}
#else
 #define DEBUG_O65_SHOW_O65MODE(_x)
#endif

static O65ERR
o65_file_parse_header(o65_file_t *O65file)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    uint16_t marker;
    uint32_t magic;
    uint8_t  version;

    FUNC_ENTER();

    do {
        /* get marker: must be $01, $00 = $0001 */
        error = o65_file_parse_uint16(O65file, &marker);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "o65 marker is:     $%04X", marker));
        if (marker != O65_FILE_HEADER_MARKER) {
            DBG_ERROR((DBG_PREFIX "- marker is not correct, no O65 file"));
            error = O65ERR_NO_O65_FILE;
            break;
        }

        /* get magic and version: must be $6f, $36, $35, $00 = "O65" + 0 */
        error = o65_file_parse_uint24(O65file, &magic);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "o65 magic is:      $%06X", magic));
        if (magic != O65_FILE_HEADER_MAGIC) {
            DBG_ERROR((DBG_PREFIX "- magic is not correct, no O65 file"));
            error = O65ERR_NO_O65_FILE;
            break;
        }

        /* get version: currently, only "0" is supported */
        error = o65_file_parse_uint8(O65file, &version);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "o65 version is:    $%02X", version));
        if (version > O65_FILE_HEADER_VERSION_MAX) {
            DBG_ERROR((DBG_PREFIX "- no supported O65 file version"));
            error = O65ERR_NO_O65_FILE;
            break;
        }

        /* get mode */
        error = o65_file_parse_uint16(O65file, &O65file->o65mode);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "o65 mode is:       $%04X", O65file->o65mode));
        DEBUG_O65_SHOW_O65MODE(O65file->o65mode);

        O65file->mode32 = O65file->o65mode & O65_FILE_HEADER_MODE_SIZE32;
        DBG_O65_SHOW((DBG_PREFIX "* mode32 is:       %d", O65file->mode32));

        O65file->pagewiserelocation = O65file->o65mode & O65_FILE_HEADER_MODE_PAGERELOC;
        DBG_O65_SHOW((DBG_PREFIX "* pagereloc is:    %d", O65file->pagewiserelocation));

        /* get the code segment base */
        error = o65_file_parse_size(O65file, &O65file->text_segment_base);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "text base is:      $%08X", O65file->text_segment_base));

        /* get the code segment length */
        error = o65_file_parse_size(O65file, &O65file->text_segment_length);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "text size is:      $%08X", O65file->text_segment_length));

        /* get the data segment base */
        error = o65_file_parse_size(O65file, &O65file->data_segment_base);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "data base is:      $%08X", O65file->data_segment_base));

        /* get the data segment length */
        error = o65_file_parse_size(O65file, &O65file->data_segment_length);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "data size is:      $%08X", O65file->data_segment_length));

        /* get the bss segment base */
        error = o65_file_parse_size(O65file, &O65file->bss_segment_base);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "bss base is:       $%08X", O65file->bss_segment_base));

        /* get the bss segment length */
        error = o65_file_parse_size(O65file, &O65file->bss_segment_length);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "bss size is:       $%08X", O65file->bss_segment_length));

        /* get the zero segment base */
        error = o65_file_parse_size(O65file, &O65file->zero_segment_base);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "zero base is:      $%08X", O65file->zero_segment_base));

        /* get the zero segment length */
        error = o65_file_parse_size(O65file, &O65file->zero_segment_length);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "zero size is:      $%08X", O65file->zero_segment_length));

        /* get the minimum stack size */
        error = o65_file_parse_size(O65file, &O65file->min_stack_size);
        if (error) {
            break;
        }
        DBG_O65_SHOW((DBG_PREFIX "min stack size is: $%08X", O65file->min_stack_size));
    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_options(o65_file_t *O65file)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    FUNC_ENTER();

    do {
        while ((error = o65_file_parse_one_option(O65file)) == O65ERR_SUCCESS)
            ;

        if (error != O65ERR_NO_MORE_DATA) {
            break;
        }

        if (O65file->option_filename_pointer) {
            DBG_O65_SHOW((DBG_PREFIX "filename:       %s", (char*) &O65file->raw_buffer[O65file->option_filename_pointer]));
        }

        if (O65file->option_assembler_pointer) {
            DBG_O65_SHOW((DBG_PREFIX "assembler:      %s", (char*) &O65file->raw_buffer[O65file->option_assembler_pointer]));
        }

        if (O65file->option_author_pointer) {
            DBG_O65_SHOW((DBG_PREFIX "author:         %s", (char*) &O65file->raw_buffer[O65file->option_author_pointer]));
        }

        if (O65file->option_creation_pointer) {
            DBG_O65_SHOW((DBG_PREFIX "creation date:  %s", (char*) &O65file->raw_buffer[O65file->option_creation_pointer]));
        }

        if (O65file->option_os_pointer) {
            uint8_t *option_os = &O65file->raw_buffer[O65file->option_os_pointer];

            if (option_os[0] < 3) {
                DBG_O65_SHOW((DBG_PREFIX "option os given, but it is too short!"));
                error = O65ERR_OPTIONAL_HEADER_TOO_SHORT;
                break;
            }

            error = O65ERR_NOT_AN_OPENCBM_MODULE;
            switch (option_os[2]) {
                case 1:
                    DBG_O65_SHOW((DBG_PREFIX "OS:             OSA/65"));
                    break;

                case 2:
                    DBG_O65_SHOW((DBG_PREFIX "OS:             Lunix"));
                    break;

                case 3:
                    DBG_O65_SHOW((DBG_PREFIX "OS:             CC65 generic"));
                    break;

                case 4:
                    DBG_O65_SHOW((DBG_PREFIX "OS:             OpenCBM floppy module"));
                    error = O65ERR_SUCCESS;
                    break;
            }
            if (error) {
                break;
            }
        }

    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_segment(const char * const WhereString, o65_file_t *O65file, size_t Length, size_t *Segment_pointer)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    size_t pointer;

    FUNC_ENTER();

    do {
        pointer = O65file->readpointer;
        error = o65_file_parse_skip(O65file, Length);
        if (error) {
            break;
        }
        *Segment_pointer = pointer;
        DBG_O65_MEMDUMP(WhereString, &O65file->raw_buffer[pointer], Length);
    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_undefined_references(o65_file_t *O65file)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    uint32_t count;
    size_t   pointer;

    FUNC_ENTER();

    do {
        uint32_t i;

        pointer = O65file->readpointer;

        DBG_O65_SHOW((DBG_PREFIX "undefined references:"));

        error = o65_file_parse_size(O65file, &count);
        if (error) {
            break;
        }

        O65file->undefined_references_table = calloc(count, sizeof *O65file->undefined_references_table);
        if (!O65file->undefined_references_table) {
            error = O65ERR_OUT_OF_MEMORY;
            break;
        }
        for (i = 0; i < count; ++i) {
            char * string;
            error = o65_file_parse_string(O65file, &string);
            if (error) {
                break;
            }
            DBG_O65_SHOW((DBG_PREFIX "- %d: %s", i, string));
            O65file->undefined_references_table[i].name = string;

        }
        if (error) {
            break;
        }

        O65file->undefined_references_count   = count;
        O65file->undefined_references_pointer = pointer;

    } while (0);

    FUNC_LEAVE_INT(error);
}

#if DBG
static char *
DEBUG_O65_SEGMENT_NAME(uint8_t Segmentid)
{
    static char *segmentidtext[] = {
        "undefined (imported)",
        "absolute",
        "text",
        "data",
        "bss",
        "zero",
        "***UNKNOWN***"
    };

    if (Segmentid >= ARRAYSIZE(segmentidtext)) {
        Segmentid = ARRAYSIZE(segmentidtext) - 1;
    }

    return segmentidtext[Segmentid];
}
#endif

#if DBG
static void
DBG_O65_RELOCENTRY(const relocation_entry_t *entry, undefined_references_t *undefined_references_table)
{
    char * mysegment = "unknown";
    char * a = "";

    switch (entry->segment) {
        case SEGMENT_TEXT: mysegment = "TEXT"; break;
        case SEGMENT_DATA: mysegment = "DATA"; break;
    }

    a = (undefined_references_table && entry->segmentid == O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_UNDEF) ? undefined_references_table[entry->index].name : "";

    DBG_O65_SHOW((DBG_PREFIX "Relocation entry in segment %s: "
                             "offset = $%08X, type = $%02X, segment = %s, lower_bytes = $%04X, index = %d (%s)",
                mysegment,
                entry->offset,
                entry->type,
                DEBUG_O65_SEGMENT_NAME(entry->segmentid),
                entry->lower_bytes,
                entry->index,
                a // "---" // (undefined_references_table ? undefined_references_table[entry->index].name : "")
                ));
}
#else
 #define DBG_O65_RELOCENTRY(_e, _urt)
#endif


static O65ERR
o65_file_parse_one_relocation_table_entry(o65_file_t *O65file, uint32_t * address_offset, relocation_entry_t * RelocationEntry)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    uint32_t current_address_offset = *address_offset;

    FUNC_ENTER();

    do {
        uint8_t  offsetbyte;
        uint8_t  typebyte;
        uint8_t  segmentid;
        uint16_t lower_bytes = 0xFFFFu;
        uint32_t index = 0xFFFFFFFFu;

        error = o65_file_parse_uint8(O65file, &offsetbyte);
        if (error) {
            break;
        }
        DBG_O65_RELOCTABLE(DBG_O65_SHOW((DBG_PREFIX "offsetbyte 1 = $%02X", offsetbyte)));
        if (offsetbyte == 0) {
            error = O65ERR_NO_MORE_DATA;
            break;
        }

        DBG_O65_RELOCTABLE(
        if (offsetbyte == 0xFFu) {
            DBG_O65_SHOW((DBG_PREFIX "- additional offsetbyte: "));
        }
        )

        while (offsetbyte == 0xFFu) {
            current_address_offset += 0xFEu;
            error = o65_file_parse_uint8(O65file, &offsetbyte);
            if (error) {
                break;
            }
        DBG_O65_RELOCTABLE(DBG_O65_SHOW((DBG_PREFIX "$%02X, ", offsetbyte)));
        }
        if (error) {
            break;
        }
        current_address_offset += offsetbyte;
        DBG_O65_RELOCTABLE(DBG_O65_SHOW((DBG_PREFIX "offset is: $%04X", current_address_offset)));

        error = o65_file_parse_uint8(O65file, &typebyte);
        if (error) {
            break;
        }
        DBG_O65_RELOCTABLE(DBG_O65_SHOW((DBG_PREFIX "typebyte | segment is: $%04X", typebyte)));

        DBG_O65_RELOCTABLE(
        if (typebyte != 0x82) {
            DBG_O65_SHOW((DBG_PREFIX "interesting typebyte..."));
        }
        if (typebyte & O65_FILE_RELOC_SEGMTYPEBYTE_UNDEF_MASK) {
            DBG_O65_SHOW((DBG_PREFIX "Relocation typebyte has bits set that should be zero: $%02X, undefined mask = $%02X",
                typebyte & O65_FILE_RELOC_SEGMTYPEBYTE_UNDEF_MASK,
                typebyte));
        }
        )

        segmentid = typebyte & O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_MASK;
        typebyte  = typebyte & O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_MASK;
        DBG_O65_RELOCTABLE(DBG_O65_SHOW((DBG_PREFIX "type is     $%02X", typebyte)));
        DBG_O65_RELOCTABLE(DBG_O65_SHOW((DBG_PREFIX "segment is: %d = %s", segmentid, DEBUG_O65_SEGMENT_NAME(segmentid))));

        if (segmentid == O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_UNDEF) {
            error = o65_file_parse_size(O65file, &index);
        }
        if (error) {
            break;
        }

        switch (typebyte) {
            case O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_WORD:
                /* WORD */
                break;
            case O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_HIGH:
                /* HIGH */

                /**!TODO: The condition segmentid != ...SEGM_UNDEF is not according to
                 * the spec! But it seems ca65 generates it wrong! */
                if (!O65file->pagewiserelocation && segmentid != O65_FILE_RELOC_SEGMTYPEBYTE_SEGM_UNDEF) {
                    uint8_t lowbyte;
                    error = o65_file_parse_uint8(O65file, &lowbyte);
                    if (error) {
                        break;
                    }
                    lower_bytes = lowbyte;
                }
                else {
                    lower_bytes = 0;
                }
                break;
            case O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_LOW:
                /* LOW */
                break;
            case O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_SEGADR:
                /* SEGADR */
                break;
            case O65_FILE_RELOC_SEGMTYPEBYTE_TYPE_SEG:
                /* SEG */
                {
                    uint16_t lowerbytes;
                    error = o65_file_parse_uint16(O65file, &lowerbytes);
                    if (error) {
                        break;
                    }
                }
                break;
        }
        if (error) {
            break;
        }

        RelocationEntry->offset      = current_address_offset;
        RelocationEntry->type        = typebyte;
        RelocationEntry->segmentid   = segmentid;
        RelocationEntry->lower_bytes = lower_bytes;
        RelocationEntry->index       = index;

        *address_offset = current_address_offset;

    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_relocation_table(o65_file_t *O65file, uint32_t base_address, size_t * relocation_table, segment_e Segment)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    size_t   pointer;
    uint32_t current_address_offset = -1u;

    FUNC_ENTER();

    do {
        pointer = O65file->readpointer;

        do {
            relocation_entry_t relocation_entry = { Segment };
            error = o65_file_parse_one_relocation_table_entry(O65file, &current_address_offset, &relocation_entry);
            if (error == O65ERR_NO_MORE_DATA) {
                error = O65ERR_SUCCESS;
                break;
            }
            if (error) {
                break;
            }
            DBG_O65_RELOCENTRY(&relocation_entry, O65file->undefined_references_table);
        } while (1);
        if (error) {
            break;
        }

        error = O65ERR_SUCCESS;

        *relocation_table = pointer;

    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse_exported_globals(o65_file_t *O65file)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    uint32_t count;
    size_t   pointer;

    FUNC_ENTER();

    do {
        uint32_t i;

        pointer = O65file->readpointer;

        DBG_O65_SHOW((DBG_PREFIX "exported globals:"));

        error = o65_file_parse_size(O65file, &count);
        if (error) {
            break;
        }

        O65file->exported_symbols_table = calloc(count, sizeof *O65file->exported_symbols_table);
        if (!O65file->exported_symbols_table) {
            error = O65ERR_OUT_OF_MEMORY;
            break;
        }

        for (i = 0; i < count; ++i) {
            char     *string;
            uint8_t   segmentid;
            uint32_t  value;

            error = o65_file_parse_string(O65file, &string);
            if (error) {
                break;
            }
            error = o65_file_parse_uint8(O65file, &segmentid);
            if (error) {
                break;
            }
            error = o65_file_parse_size(O65file, &value);
            if (error) {
                break;
            }
            DBG_O65_SHOW((DBG_PREFIX "- %s: %s:$%08X", string, DEBUG_O65_SEGMENT_NAME(segmentid), value));

            O65file->exported_symbols_table[i].name      = string;
            O65file->exported_symbols_table[i].segmentid = segmentid;
            O65file->exported_symbols_table[i].value     = value;
        }
        if (error) {
            break;
        }

        O65file->exported_globals_count   = count;
        O65file->exported_globals_pointer = pointer;
    } while (0);

    FUNC_LEAVE_INT(error);
}

static O65ERR
o65_file_parse(o65_file_t *O65file)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    FUNC_ENTER();

    do {
        /* parse the (fixed) header */
        error = o65_file_parse_header(O65file);
        if (error) {
            break;
        }

        /* parse the options */
        error = o65_file_parse_options(O65file);
        if (error) {
            break;
        }

        /* parse the text segment */
        error = o65_file_parse_segment("text segment", O65file, O65file->text_segment_length, &O65file->text_segment_pointer);
        if (error) {
            break;
        }

        /* parse the data segment */
        error = o65_file_parse_segment("data segment", O65file, O65file->data_segment_length, &O65file->data_segment_pointer);
        if (error) {
            break;
        }

        /* parse the undefined references table */
        error = o65_file_parse_undefined_references(O65file);
        if (error) {
            break;
        }

        /* parse the relocation table for text segment */
        error = o65_file_parse_relocation_table(O65file, O65file->text_segment_base, &O65file->relocation_text_pointer, SEGMENT_TEXT);
        if (error) {
            break;
        }

        /* parse the relocation table for data segment */
        error = o65_file_parse_relocation_table(O65file, O65file->data_segment_base, &O65file->relocation_data_pointer, SEGMENT_DATA);
        if (error) {
            break;
        }

        /* parse the expoerted globals table */
        error = o65_file_parse_exported_globals(O65file);
        if (error) {
            break;
        }

    } while (0);

    FUNC_LEAVE_INT(error);
}

int
o65_init(void)
{
    O65ERR error = O65ERR_SUCCESS;

    FUNC_ENTER();

    FUNC_LEAVE_INT(error);
}

int
o65_shutdown(void)
{
    O65ERR error = O65ERR_SUCCESS;

    FUNC_ENTER();

    FUNC_LEAVE_INT(error);
}

int
o65_file_process(void *Buffer, size_t Length, void **voidPO65file)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    o65_file_t **PO65file = (o65_file_t **) voidPO65file;
    o65_file_t *O65file = NULL;

    FUNC_ENTER();

    DBG_ASSERT(Buffer != NULL);
    DBG_ASSERT(Length > 0);
    DBG_ASSERT(PO65file != NULL);
    DBG_ASSERT(*PO65file == NULL);

    do {
        O65file = o65_file_alloc(Buffer, Length);
        if (O65file == NULL) {
            error = O65ERR_OUT_OF_MEMORY;
            break;
        }

        error = o65_file_parse(O65file);
        if (error) {
            break;
        }

    } while (0);

    if (error != O65ERR_SUCCESS) {
        o65_file_free(O65file);
    }
    else {
        *PO65file = O65file;
    }

    FUNC_LEAVE_INT(error);
}

int
o65_file_load(const char * const Filename, void **voidPO65file)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    o65_file_t **PO65file = (o65_file_t **) voidPO65file;
    o65_file_t  *O65file  = NULL;

    FILE        *f        = NULL;

    FUNC_ENTER();

    DBG_ASSERT(Filename != NULL);
    DBG_ASSERT(PO65file != NULL);
    DBG_ASSERT(*PO65file == NULL);

    do {
        size_t file_size_seek = 0;
        size_t file_size      = 0;

        DBG_O65_SHOW((DBG_PREFIX "Reading O65 file '%s'", Filename));
        f = fopen(Filename, "rb");
        if (!f) {
            DBG_ERROR((DBG_PREFIX "could not open file '%s'", Filename));
            error = O65ERR_NO_FILE;
            break;
        }

        if (fseek(f, 0, SEEK_END) != 0) {
            error = O65ERR_FILE_HANDLING_ERROR;
            break;
        }

        file_size_seek = ftell(f);
        if (file_size_seek == 0) {
            error = O65ERR_FILE_EMPTY;
            break;
        }

        if (fseek(f, 0, SEEK_SET) != 0) {
            error = O65ERR_FILE_HANDLING_ERROR;
            break;
        }


        O65file = o65_file_alloc(NULL, file_size_seek);
        if (O65file == NULL) {
            error = O65ERR_OUT_OF_MEMORY;
            break;
        }

        file_size = fread(O65file->raw_buffer, 1, file_size_seek + 1, f);
        if (file_size == file_size_seek && !feof(f)) {
            error = O65ERR_FILE_HANDLING_ERROR;
            break;
        }

        error = o65_file_parse(O65file);
        if (error) {
            break;
        }

    } while (0);

    if (f) {
        fclose(f);
    }

    if (error != O65ERR_SUCCESS) {
        o65_file_free(O65file);
    }
    else {
        *PO65file = O65file;
    }

    FUNC_LEAVE_INT(error);
}


int
o65_file_reloc(void *O65file, unsigned int Address)
{
    O65ERR error = O65ERR_UNSPECIFIED;

    FUNC_ENTER();

    FUNC_LEAVE_INT(error);
}

void
o65_file_delete(void *O65file)
{
    FUNC_ENTER();

    FUNC_LEAVE();
}
