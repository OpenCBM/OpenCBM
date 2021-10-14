/*
 * Name: s3.c
 * Project: xs1541
 * Author: Thomas Winkler
 * Copyright: (c) 2011 by Thomas Winkler <t.winkler@aon.at>
 * License: GPL
 * This Revision: $Id$
 *
 * Revision 1.0.0.0  2011/07/01 ToWi
 * Initial version
 */

#include "xum1541.h"

#define    SRQ_DELAY1    3
#define    SRQ_DELAY2    3
#define    SRQ_DELAY3    3

#ifdef IO_SRQ

/*
;             0__   1__   2__   3__   4__   5__   6__   7__
;    srq    __|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__
;
;             0   __1__   0   __1__   0   __1_____1__   0
;    data   _____|     |_____|     |_____|           |________
;
;
;    http://www.cs.tut.fi/~albert/Dev/burst/
;
;       DATA - input   :: low = drive not ready, high = drive ready
;       CLK  - output  :: low = AVR not ready,   high = AVR ready
;
;
;       AVR send data:
;       ---------------
;       set CLK low                     (AVR ready to send)
;       wait for DATA = lo              (drive ready)
;       release CLK hi
;       wait for DATA = hi 
;       wait for CLK = lo               (drive ready to receive)
;       send byte
;       wait for CLK = hi               (drive :: byte received)
;
;       1571 receive data:
;       ------------------
;       wait for CLK low
;       set DATA low
;       wait for CLK high
;       release DATA, set CLK low
;       receive byte
;       release CLK
;
;
;       1571 send data:
;       ---------------
;       set CLK low                     (drive ready to send)
;       wait for DATA = lo              (AVR ready)
;       release CLK hi
;       wait for CLK = lo               (drive ready to receive)
;       send byte
;       wait for CLK = hi               (drive :: byte received)
;
;       AVR receive data:
;       ------------------
;       wait for CLK = low              (drive ready to send)
;       set DATA low
;       wait for CLK = high             (drive ready to send)
;       set CLK low                     (ready to receive)
;       receive byte
;       set CLK high                    (byte received)
*/

void
s3_write_byte(uint8_t data)
{
    uint8_t i;

    iec_set(IO_CLK);                            // set CLK low 
    while ((iec_poll_pins() & IO_DATA) == 0)    // wait for DATA high 
    {
        //if (!TimerWorker())
        //    break;
    } 


    for (i=0;i<8;++i)
    {
        iec_set(IO_SRQ);

        if (data & 0x80) 
            iec_release(IO_DATA);
        else
            iec_set(IO_DATA);

        data <<= 1;
        _delay_us(SRQ_DELAY1);
        iec_release(IO_SRQ);
        _delay_us(SRQ_DELAY2);
    }
    _delay_us(SRQ_DELAY3);
    iec_release(IO_DATA);

    iec_release(IO_CLK);                        // set CLK high
    _delay_us(2);
    while ((iec_poll_pins() & IO_DATA) != 0)    // wait for DATA low
    {
        //if (!TimerWorker())
        //    break;
    } 

    //LED_ON();
    //LED_TOGGLE();
}

uint8_t
s3_read_byte(void)
{
    uint16_t cntSrq0;
    uint8_t ch, c[8], i;

    iec_set(IO_CLK);                            // set CLK low 
    while (iec_get(IO_DATA))                    // wait for DATA high 
    {
        //if (!TimerWorker())
        //    break;
    } 

    cli();                                      // no interrupts! 
    iec_release(IO_CLK);                        // set CLK high
    iec_set(IO_DATA);                           // set DATA low 
    while (iec_get(IO_CLK))                     // wait for CLK high 
    {
        //if (!TimerWorker())
        //    break;
    } 

    cntSrq0 = 1;
    iec_release(IO_DATA);                       // set DATA high
    iec_set(IO_CLK);                            // set CLK low 

    
    for (i=0;i<8;i++) 
    {
        while (!iec_get(IO_SRQ)) {              // wait for SRQ low (active)
            if (cntSrq0 == 0)
                break;    
            else
                cntSrq0++;
        }
        while ((iec_get(IO_SRQ)))               // wait for SRQ low (active)
            ;
        c[i] = iec_get(IO_DATA);                    
    }


    ch = 0;
    if (cntSrq0 == 0) {
        // timeout!!
        //LED_OFF();
        ch = 0xff;
    } else {
        for (i=0;i<8;i++) {
            ch <<= 1;
            if (!c[i])
                ch |= 1;
        }
    }

    iec_release(IO_CLK);                        // set CLK high
    _delay_us(2);
    while (!iec_get(IO_CLK))                    // wait for CLK low 
    {
        //if (!TimerWorker())
        //    break;
    } 
    //iec_set(IO_DATA);                         // set DATA low 
    while (iec_get(IO_CLK))                     // wait for CLK high 
    {
        //if (!TimerWorker())
        //    break;
    } 
    sei();

    return ch;
}
#endif
