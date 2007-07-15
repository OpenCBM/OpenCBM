#ifndef XU1541_BIOS_H
#define XU1541_BIOS_H

#define BIOSTABLE __attribute__ ((section (".textbiostable")))

typedef void (*start_flash_bootloader_t)(void);
typedef void (*spm_t)(uint8_t what, uint16_t address, uint16_t data);

typedef
struct xu1541_bios_data_s {
        unsigned char version_major;
        unsigned char version_minor;
        start_flash_bootloader_t start_flash_bootloader;
        spm_t spm;
} xu1541_bios_data_t;

extern xu1541_bios_data_t bios_data BIOSTABLE;

#endif /* #ifndef XU1541_BIOS_H */

