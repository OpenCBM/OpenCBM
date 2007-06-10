#ifndef XU1541_BIOS_H
#define XU1541_BIOS_H

#define BIOSTABLE __attribute__ ((section (".textbiostable")))

typedef
struct xu1541_bios_data_s {
        unsigned char version_major;
        unsigned char version_minor;
        void (*start_flash_bootloader)(void);
} xu1541_bios_data_t;

extern xu1541_bios_data_t bios_data BIOSTABLE;

#endif /* #ifndef XU1541_BIOS_H */

