/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, version 
 *  2 of the License.
 *
 *  Copyright 2007 Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file flasher.c \n
** \author Spiro Trikaliotis \n
** \version $Id: flasher.c,v 1.1 2007-07-15 14:05:46 strik Exp $ \n
** \n
** \brief Flash the bootloader from the application space
**
****************************************************************/


#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <avr/boot.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include <util/delay.h>

#include "../bootloader/xu1541bios.h"

#define STATIC static

STATIC
void
delay_ms(unsigned int ms) {
        unsigned int i;
        for (i = 0; i < ms; i++)
                _delay_ms(1);
}

xu1541_bios_data_t bios_data;

STATIC
void
start_bootloader(void) {
        ((start_flash_bootloader_t) pgm_read_word_near(&bios_data.start_flash_bootloader))();
}

STATIC
void
spm(uint8_t what, uint16_t address, uint16_t data) {
        ((spm_t) pgm_read_word_near(&bios_data.spm))(what, address, data);
}

#undef boot_page_erase
STATIC
void
boot_page_erase(uint16_t address) 
{
        spm(__BOOT_PAGE_ERASE, address, 0);
}

#undef boot_page_fill
STATIC
void
boot_page_fill(uint16_t address, uint16_t data) 
{
        spm(__BOOT_PAGE_FILL, address, data);
}

#undef boot_page_write
STATIC
void
boot_page_write(uint16_t address) 
{
        spm(__BOOT_PAGE_WRITE, address, 0);
}

STATIC
void
blink(unsigned long count)
{
/**/
        DDRD  |=  _BV(1);
        PORTD &= ~_BV(1);

        count *= 2;

        while (--count) {
                delay_ms(300);
                PORTD ^= _BV(1);
        }

        delay_ms(1000);
/**/
}

STATIC
void
boot_program_page(uint16_t page, uint8_t *buf)
{
        uint16_t i;

        eeprom_busy_wait();
        boot_spm_busy_wait();      // Wait until the memory is erased.

blink(1);
        boot_page_erase(page);
        boot_spm_busy_wait();      // Wait until the memory is erased.
blink(2);

        for (i = 0; i < SPM_PAGESIZE; i += 2)
        {
                // Set up little-endian word.

                uint16_t w = *buf++;
                w += (*buf++) << 8;

                boot_page_fill(page + i, w);
        }

        boot_page_write(page);     // Store buffer in flash page.
}

int
main(void)
{
        static uint8_t data[128] = { 'H', 'a', 'l', 'l', 'o', ' ', 'S', 'a', 'n', 'd', 'r', 'a', '.', ' ',
                'I', 'c', 'h', ' ', 'l', 'i', 'e', 'b', 'e', ' ', 'd', 'i', 'c', 'h', '!', 0};

        cli();

        blink(1);

        boot_program_page(0x1000, data);
        boot_program_page(0x1040, data);

        blink(3);

        /* make sure we are not called anymore */
        boot_page_erase(0);

        start_bootloader();

        return 0;
}
