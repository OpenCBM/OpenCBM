/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis <cbm4win@trikaliotis.net>
 *
 */

/*! ************************************************************** 
** \file sys/libiec/i_iec.h \n
** \author Spiro Trikaliotis \n
** \version $Id: i_iec.h,v 1.1 2004-11-07 11:05:14 strik Exp $ \n
** \n
** \brief Internal functions and definitions of the libiec library
**
****************************************************************/

#ifndef I_CBMIEC_H
#define I_CBMIEC_H

#include "iec.h"

#include <parallel.h>

/* The port addresses (relative) of the parallel port */

/*! the DATA register is located here */
#define PARALLEL_DATA_OFFSET 0
/*! the STATUS register is located here */
#define PARALLEL_STATUS_OFFSET 1
/*! the CONTROL register is located here */
#define PARALLEL_CONTROL_OFFSET 2
/*! the count of port addresses the parallel port occupies */
#define PARALLEL_REGISTER_SPAN 3


/* lpt output lines */
/* this is correct for a XA1541/XM1541; a XE1541 has DATA and RESET exchanged! */

#define PP_ATN_OUT    0x01 //!< The ATN OUT bit
#define PP_CLK_OUT    0x02 //!< The CLOCK OUT bit
#define PP_DATA_OUT   0x04 //!< The DATA OUT bit
#define PP_RESET_OUT  0x08 //!< The RESET OUT bit

/* additional LP control */
#define PP_LP_IRQ     0x10 //!< Bit for allowing interrupts of the LPT
#define PP_LP_BIDIR   0x20 //!< Bit for setting set bidirectional mode of the LPT

/* lpt input lines */
#define PP_ATN_IN     0x10 //!< The ATN IN bit
#define PP_CLK_IN     0x20 //!< The CLOCK IN bit
#define PP_DATA_IN    0x40 //!< The DATA IN bit

/*! Get the address of the parallel port DATA register out of the Pdx info */
#define PAR_PORT (Pdx->PortInfo->Controller + PARALLEL_DATA_OFFSET)
/*! Get the address of the parallel port STATUS register (= the port for input)
 * out of the Pdx info */
#define IN_PORT  (Pdx->PortInfo->Controller + PARALLEL_STATUS_OFFSET)
/*! Get the address of the parallel port CONTROL register (= the port for
 * output) out of the Pdx info */
#define OUT_PORT (Pdx->PortInfo->Controller + PARALLEL_CONTROL_OFFSET)

/*! set an output line on the parallel port */
#define CBMIEC_SET(_set)              Pdx->IecOutBits|=(_set); WRITE_PORT_UCHAR(OUT_PORT,(UCHAR)(Pdx->IecOutEor ^ Pdx->IecOutBits))
/*! release an output line on the parallel port */
#define CBMIEC_RELEASE(_rel)          Pdx->IecOutBits&=~(_rel); WRITE_PORT_UCHAR(OUT_PORT,(UCHAR)(Pdx->IecOutEor ^ Pdx->IecOutBits))
/*! set and release an output line on the parallel port (simultaneously) */
#define CBMIEC_SET_RELEASE(_set,_rel) Pdx->IecOutBits|=(_set); Pdx->IecOutBits&=~(_rel); \
                                      WRITE_PORT_UCHAR(OUT_PORT,(UCHAR)(Pdx->IecOutEor ^ Pdx->IecOutBits))

/*! get the value of the parallel port */
#define CBMIEC_GET(_line)             ((READ_PORT_UCHAR(IN_PORT)&_line)==0?1:0)


/*! The various timeouts of the IEC bus protocol
 * \todo Rename the timeout values so that the name does have more meaning.
 */
typedef
struct IEC_TIMEOUTS {

    ULONG T_holdreset;  //!< = 100 us: How long is a RESET being held?
    ULONG T_afterreset; //!< = 5 s: How long to delay after a RESET
    ULONG T_1;          //!< = 20 us: \todo Document and rename timeout
    ULONG T_2_Times;    //!< x T_2 is 40: \todo Document and rename timeout
    ULONG T_2;          //!< = 10 us: \todo Document and rename timeout
    ULONG T_3;          //!< = 70 us: \todo Document and rename timeout
    ULONG T_4_Times;    //!< x T_4, is 100: \todo Document and rename timeout
    ULONG T_4;          //!< = 20 us: \todo Document and rename timeout
    ULONG T_5_Times;    //!< x T_5, is 200: \todo Document and rename timeout
    ULONG T_5;          //!< = 10 us: \todo Document and rename timeout
    ULONG T_6_Times;    //!< x T_6, is 100: \todo Document and rename timeout
    ULONG T_6;          //!< = 20 us: \todo Document and rename timeout
    ULONG T_7;          //!< = 70 us: \todo Document and rename timeout

    ULONG T_8;          //!< = 20 us: \todo Document and rename timeout
    ULONG T_9;          //!< = 10 us: \todo Document and rename timeout
    ULONG T_9a_Times;   //!< x T_9a, is 100: \todo Document and rename timeout
    ULONG T_9a;         //!< = 10 us: \todo Document and rename timeout
    ULONG T_10;         //!< = 20 us: \todo Document and rename timeout
    ULONG T_11;         //!< = 50 us: \todo Document and rename timeout
    ULONG T_12;         //!< = 100 us: \todo Document and rename timeout
    ULONG T_13;         //!< = 20 us: \todo Document and rename timeout
    ULONG T_14;         //!< = 100 us: \todo Document and rename timeout

    // sendbyte related:
    ULONG T_15;         //!< = 70 us: \todo Document and rename timeout
    ULONG T_16;         //!< = 20 us: \todo Document and rename timeout
    ULONG T_17_Times;   //!< x T_17, is 20: \todo Document and rename timeout
    ULONG T_17;         //!< = 100 us: \todo Document and rename timeout

    ULONG T_WaitForListener_Granu; //!< = 10 us: Graunularity of wait_for_listener() polls

} IEC_TIMEOUTS;

/*! timeout values */
extern IEC_TIMEOUTS libiec_global_timeouts;

#if DBG

/* The functions DbgWp() and DbgRp() are used to intercept
 * NDIS_WRITE_PORT() and NDIS_READ_PORT() when debugging.
 */
extern VOID
DbgWp(IN PUCHAR Port, IN UCHAR Value);

extern UCHAR
DbgRp(IN PUCHAR Port);

/* cbmiec_show_port() is used to show the value of the
 * parallel port lines if needed for debugging
 */
extern VOID
cbmiec_show_port(UCHAR *s);

/*
 * If DBG is defined, we want to be able to monitor every
 * READ_PORT_UCHAR and WRITE_PORT_UCHAR, so we override them
 * here with our own functions:
 */

#define READ_PORT_UCHAR(_x_)       DbgRp(_x_)
#define WRITE_PORT_UCHAR(_x_, _y_) DbgWp(_x_, _y_)

/*
 *  dump input lines
 */
extern VOID
cbmiec_show_state(IN PDEVICE_EXTENSION Pdx, IN UCHAR *Str);

#else /* #if DBG */

    //! A cbmiec_show_state() implementation for release builds
    #define cbmiec_show_state(_x_, _y_)

#endif /* #if DBG */

extern VOID
cbmiec_schedule_timeout(IN ULONG howlong); // howlong in us!

extern VOID
cbmiec_udelay(IN ULONG howlong); // howlong in ms!

extern NTSTATUS
cbmiec_i_raw_read(IN PDEVICE_EXTENSION Pdx, OUT UCHAR *buf, USHORT cnt, OUT USHORT *pReceived);

extern NTSTATUS
cbmiec_i_raw_write(PDEVICE_EXTENSION Pdx, const UCHAR *buf, USHORT cnt, USHORT *pSent, BOOLEAN atn, BOOLEAN talk);

extern VOID
cbmiec_block_irq(VOID);

extern VOID
cbmiec_release_irq(VOID);

#ifdef USE_DPC

    extern VOID
    cbmiec_dpc(IN PKDPC Dpc, IN PDEVICE_OBJECT Fdo, IN PIRP Irp, IN PVOID Context);

#endif // #ifdef USE_DPC

#endif /* #ifndef I_CBMIEC_H */
