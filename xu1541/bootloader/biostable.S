#include "version.h"

	.data
;; nothing

	.text
        .section .textbiostable

	.global	bios_data
	.type	bios_data, @function

bios_data:
	.byte   XU1541_BIOS_VERSION_MAJOR
        .byte   XU1541_BIOS_VERSION_MINOR
        rjmp    start_flash_bootloader
        rjmp    spm

