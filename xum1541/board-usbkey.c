/*
 * Board interface routines for the USBKEY development kit
 * Copyright (c) 2009 Nate Lawson <nate@root.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <LUFA/Drivers/Board/LEDs.h>
#include "xum1541.h"

// IEC and parallel port accessors
#define CBM_PORT  PORTA
#define CBM_DDR   DDRA
#define CBM_PIN   PINA

#define PAR_PORT_PORT   PORTC
#define PAR_PORT_DDR    DDRC
#define PAR_PORT_PIN    PINC

#ifdef DEBUG
// Send a byte to the UART for debugging printf()
static int
uart_putchar(char c, FILE *stream)
{
    if (c == '\n')
        uart_putchar('\r', stream);
    loop_until_bit_is_set(UCSR1A, UDRE1);
    UDR1 = c;
    return 0;
}
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
#endif // DEBUG

// Initialize the board (IO ports, indicators, UART)
void
board_init(void)
{
    /*
     * Configure the IEC port for 4 inputs, 4 outputs.
     *
     * Add pull-ups on all the inputs since we're running at 3.3V while
     * the 1541 is at 5V.  This causes current to flow into our Vcc
     * through the 1541's 1K ohm pull-ups but with our pull-ups also,
     * the current is only ~50 uA (versus 1.7 mA with just the 1K pull-ups).
     */
    CBM_DDR = IO_OUTPUT_MASK;
    CBM_PORT = ~IO_OUTPUT_MASK;

#ifdef DEBUG
    /*
     * Initialize the UART baud rate at 115200 8N1 and select it for
     * printf() output.
     */
    UCSR1A |= _BV(U2X1);
    UCSR1B |= _BV(TXEN1);
    UBRR1 = 8;
    stdout = &mystdout;
#endif

    LEDs_Init();
}

// Routines for getting/setting individual IEC lines
void
iec_set(uint8_t line)
{
    CBM_PORT |= line;
}

void
iec_release(uint8_t line)
{
    CBM_PORT &= ~line;
}

void
iec_set_release(uint8_t s, uint8_t r)
{
    CBM_PORT = (CBM_PORT & ~r) | s;
}

uint8_t
iec_get(uint8_t line)
{
    return ((CBM_PIN >> 1) & line) == 0 ? 1 : 0;
}

uint8_t
iec_poll(void)
{
    return CBM_PIN >> 1;
}

// Make 8-bit port all inputs and read value
uint8_t
xu1541_pp_read(void)
{
    PAR_PORT_DDR = 0;
    PAR_PORT_PORT = 0;
    return PAR_PORT_PIN;
}

// Make 8-bits of port output and write out the data
void
xu1541_pp_write(uint8_t val)
{
    PAR_PORT_DDR = 0xff;
    PAR_PORT_PORT = val;
}

#define LED_UPPER_RED   LEDS_LED3
#define LED_UPPER_GREEN LEDS_LED4
#define LED_LOWER_RED   LEDS_LED1
#define LED_LOWER_GREEN LEDS_LED2

static uint8_t statusValue;
static uint8_t statusMask;
static uint16_t statusCount;

uint8_t
board_get_status()
{
    return statusValue;
}

// Status indicators (LEDs for this board)
void
board_set_status(uint8_t status)
{
    statusValue = status;

    switch (status) {
    case STATUS_INIT:
        LEDs_SetAllLEDs(LED_UPPER_RED);
        break;
    case STATUS_CONNECTING:
        LEDs_SetAllLEDs(LED_UPPER_GREEN);
        break;
    case STATUS_READY:
        // Disable timer
        TCCR0B = 0;
        LEDs_SetAllLEDs(LED_LOWER_GREEN);
        statusMask = 0;
        statusCount = 0;
        break;
    case STATUS_ACTIVE:
        // Toggle both green LEDs while busy
        statusMask = LED_UPPER_GREEN | LED_LOWER_GREEN;
        LEDs_SetAllLEDs(statusMask);
        // Normal counter with prescaler FCPU/1024
        TCCR0B |= (1 << CS02) | (1 << CS00);
        TCNT0 = 0;
        break;
    case STATUS_ERROR:
        // Set both red on error
        statusMask = LED_UPPER_RED | LED_LOWER_RED;
        LEDs_SetAllLEDs(statusMask);
        break;
    default:
        DEBUGF("badstsval %d\n", status);
    }
}

// Callback that is pinged every command to update LEDs
void
board_update_display()
{
    uint8_t leds;

    if (statusMask != 0) {
        // Every ~30 ms, flash the LEDs
        if ((TIFR0 & (1 << TOV0)) != 0) {
            // Clear overflow bit
            TIFR0 |= (1 << TOV0);
            statusCount++;
            leds = LEDs_GetLEDs();
            LEDs_SetAllLEDs(leds ^ statusMask);

            // Go back to idle after ~300 ms.
            if (statusValue == STATUS_ACTIVE && statusCount == 10) {
                board_set_status(STATUS_READY);
            }
        }

    }
}
