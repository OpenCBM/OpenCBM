#ifndef XU1541_BIOS_H
#define XU1541_BIOS_H

typedef
struct xu1541_bios_data_s {
        int version_major;
        int version_minor;
        void (*start_flash_bootloader)(void);
} xu1541_bios_data_t;

typedef void (*xu1541_bios_fill_data_t)(unsigned int structsize, xu1541_bios_data_t *data);

#endif /* #ifndef XU1541_BIOS_H */

