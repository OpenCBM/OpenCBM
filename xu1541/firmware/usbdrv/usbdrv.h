/* Name: usbdrv.h
 * Project: AVR USB driver
 * Author: Christian Starkjohann
 * Creation Date: 2004-12-29
 * Tabsize: 4
 * Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: Proprietary, free under certain conditions. See Documentation.
 * This Revision: $Id: usbdrv.h,v 1.1 2007-02-04 12:36:37 harbaum Exp $
 */

#ifndef __usbdrv_h_included__
#define __usbdrv_h_included__
#include "usbconfig.h"
#include "iarcompat.h"

/*
Hardware Prerequisites:
=======================
USB lines D+ and D- MUST be wired to the same I/O port. Line D- MUST be wired
to bit number 0. D+ must also be connected to INT0. D- requires a pullup of
1.5k to +3.5V (and the device must be powered at 3.5V) to identify as
low-speed USB device. A pullup of 1M SHOULD be connected from D+ to +3.5V to
prevent interference when no USB master is connected. We use D+ as interrupt
source and not D- because it does not trigger on keep-alive and RESET states.

As a compile time option, the 1.5k pullup resistor on D- can be made
switchable to allow the device to disconnect at will. See the definition of
usbDeviceConnect() and usbDeviceDisconnect() further down in this file.

Please adapt the values in usbconfig.h according to your hardware!

The device MUST be clocked at 12 MHz. This is more than the 10 MHz allowed by
an AT90S2313 powered at 4.5V. However, if the supply voltage to maximum clock
relation is interpolated linearly, an ATtiny2313 meets the requirement by
specification. In practice, the AT90S2313 can be overclocked and works well.


Limitations:
============
Compiling:
You should link the usbdrv.o module first because it has special alignment
requirements for the receive buffer (the buffer must not cross a 256 byte
page boundary, it must not even touch it at the end). If you can't link it
first, you must use other measures to ensure alignment.
Note: gcc does not always assign variable addresses in the order as the modules
are linked or the variables are declared. You can choose a memory section for
the receive buffer with the configuration option "USB_BUFFER_SECTION". This
option defaults to ".bss". If you use your own section, you can place it at
an arbitrary location with a linker option similar to
"-Wl,--section-start=.mybuffer=0x800060". Use "avr-nm -ng" on the binary and
search for "usbRxBuf" to find tbe base address of the 22 bytes rx buffer.

Robustness with respect to communication errors:
The driver assumes error-free communication. It DOES check for errors in
the PID, but does NOT check bit stuffing errors, SE0 in middle of a byte,
token CRC (5 bit) and data CRC (16 bit). CRC checks can not be performed due
to timing constraints: We must start sending a reply within 7 bit times.
Bit stuffing and misplaced SE0 would have to be checked in real-time, but CPU
performance does not permit that. The driver does not check Data0/Data1
toggling, but application software can implement the check.

Sampling jitter:
The driver guarantees a sampling window of 1/2 bit. The USB spec requires
that the receiver has at most 1/4 bit sampling window. The 1/2 bit window
should still work reliably enough because we work at low speed. If you want
to meet the spec, define the macro "USB_CFG_SAMPLE_EXACT" to 1 in usbconfig.h.
This will unroll a loop which results in bigger code size.

Input characteristics:
Since no differential receiver circuit is used, electrical interference
robustness may suffer. The driver samples only one of the data lines with
an ordinary I/O pin's input characteristics. However, since this is only a
low speed USB implementation and the specification allows for 8 times the
bit rate over the same hardware, we should be on the safe side. Even the spec
requires detection of asymmetric states at high bit rate for SE0 detection.

Number of endpoints:
The driver supports up to two endpoints: One control endpoint (endpoint 0) and
one interrupt-in endpoint (endpoint 1) where the device can send interrupt
data to the host. Endpoint 1 is only compiled in if
USB_CFG_HAVE_INTRIN_ENDPOINT is defined to 1 in usbconfig.h.

Maximum data payload:
Data payload of control in and out transfers may be up to 254 bytes. In order
to accept payload data of out transfers, you need to implement
'usbFunctionWrite()'.

USB Suspend Mode supply current:
The USB standard limits power consumption to 500uA when the bus is in suspend
mode. This is not a problem for self-powered devices since they don't need
bus power anyway. Bus-powered devices can achieve this only by putting the
CPU in sleep mode. The driver does not implement suspend handling by itself.
However, the application may implement activity monitoring and wakeup from
sleep. The host sends regular SE0 states on the bus to keep it active. These
SE0 states can be detected by wiring the INT1 pin to D+. It is not necessary
to enable the interrupt, checking the interrupt pending flag should suffice.
Before entering sleep mode, the application should enable INT1 for a wakeup
on the next bus activity.

Operation without an USB master:
The driver behaves neutral without connection to an USB master if D- reads
as 1. To avoid spurious interrupts, we recommend a high impedance (e.g. 1M)
pullup resistor on D+. If D- becomes statically 0, the driver may block in
the interrupt routine.

Interrupt latency:
The application must ensure that the USB interrupt is not disabled for more
than 20 cycles. This implies that all interrupt routines must either be
declared as "INTERRUPT" instead of "SIGNAL" (see "avr/signal.h") or that they
are written in assembler with "sei" as the first instruction.

Maximum interrupt duration / CPU cycle consumption:
The driver handles all USB communication during the interrupt service
routine. The routine will not return before an entire USB message is received
and the reply is sent. This may be up to ca. 1200 cycles = 100us if the host
conforms to the standard. The driver will consume CPU cycles for all USB
messages, even if they address another (low-speed) device on the same bus.

*/

/* ------------------------------------------------------------------------- */
/* --------------------------- Module Interface ---------------------------- */
/* ------------------------------------------------------------------------- */

#define USBDRV_VERSION  20060314
/* This define uniquely identifies a driver version. It is a decimal number
 * constructed from the driver's release date in the form YYYYMMDD. If the
 * driver's behavior or interface changes, you can use this constant to
 * distinguish versions. If it is not defined, the driver's release date is
 * older than 2006-01-25.
 */

#ifndef __ASSEMBLER__

#ifndef uchar
#define uchar   unsigned char
#endif
#ifndef schar
#define schar   signed char
#endif
/* shortcuts for well defined 8 bit integer types */

extern void     usbInit(void);
/* This function must be called before interrupts are enabled and the main
 * loop is entered.
 */
extern void     usbPoll(void);
/* This function must be called at regular intervals from the main loop.
 * Maximum delay between calls is somewhat less than 50ms (USB timeout for
 * accepting a Setup message). Otherwise the device will not be recognized.
 * Please note that debug outputs through the UART take ~ 0.5ms per byte
 * at 19200 bps.
 */
extern uchar    *usbMsgPtr;
/* This variable may be used to pass transmit data to the driver from the
 * implementation of usbFunctionWrite(). It is also used internally by the
 * driver for standard control requests.
 */
extern uchar    usbFunctionSetup(uchar data[8]);
/* This function is called when the driver receives a SETUP transaction from
 * the host which is not answered by the driver itself (in practice: class and
 * vendor requests). All control transfers start with a SETUP transaction where
 * the host communicates the parameters of the following (optional) data
 * transfer. The SETUP data is available in the 'data' parameter which can
 * (and should) be casted to 'usbRequest_t *' for a more user-friendly access
 * to parameters.
 *
 * If the SETUP indicates a control-in transfer, you should provide the
 * requested data to the driver. There are two ways to transfer this data:
 * (1) Set the global pointer 'usbMsgPtr' to the base of the static RAM data
 * block and return the length of the data in 'usbFunctionSetup()'. The driver
 * will handle the rest. Or (2) return 0xff in 'usbFunctionSetup()'. The driver
 * will then call 'usbFunctionRead()' when data is needed. See the
 * documentation for usbFunctionRead() for details.
 *
 * If the SETUP indicates a control-out transfer, the only way to receive the
 * data from the host is through the 'usbFunctionWrite()' call. If you
 * implement this function, you must return 0xff in 'usbFunctionSetup()' to
 * indicate that 'usbFunctionWrite()' should be used. See the documentation of
 * this function for more information. If you just want to ignore the data sent
 * by the host, return 0 in 'usbFunctionSetup()'.
 *
 * Note that calls to the functions usbFunctionRead() and usbFunctionWrite()
 * are only done if enabled by the configuration in usbconfig.h.
 */
#if USB_CFG_HAVE_INTRIN_ENDPOINT
void    usbSetInterrupt(uchar *data, uchar len);
/* This function sets the message which will be sent during the next interrupt
 * IN transfer. The message is copied to an internal buffer and must not exceed
 * a length of 8 bytes. The message may be 0 bytes long just to indicate the
 * interrupt status to the host.
 * If you need to transfer more bytes, use a control read after the interrupt.
 */
extern volatile schar usbTxLen1;
#define usbInterruptIsReady()   (usbTxLen1 == -1)
/* This macro indicates whether the last interrupt message has already been
 * sent. If you set a new interrupt message before the old was sent, the
 * message already buffered will be lost.
 */
#endif /* USB_CFG_HAVE_INTRIN_ENDPOINT */
#if USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
extern PROGMEM const char usbHidReportDescriptor[];
/* If you implement an HID device, you need to provide a report descriptor.
 * The HID report descriptor syntax is a bit complex. If you understand how
 * report descriptors are constructed, we recommend that you use the HID
 * Descriptor Tool from usb.org, see http://www.usb.org/developers/hidpage/.
 * Otherwise you should probably start with a working example.
 */
#endif  /* USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH */
#if USB_CFG_IMPLEMENT_FN_WRITE
extern uchar    usbFunctionWrite(uchar *data, uchar len);
/* This function is called by the driver to provide a control transfer's
 * payload data (control-out). It is called in chunks of up to 8 bytes. The
 * total count provided in the current control transfer can be obtained from
 * the 'length' property in the setup data. If an error occurred during
 * processing, return 0xff (== -1). The driver will answer the entire transfer
 * with a STALL token in this case. If you have received the entire payload
 * successfully, return 1. If you expect more data, return 0. If you don't
 * know whether the host will send more data (you should know, the total is
 * provided in the usbFunctionSetup() call!), return 1.
 * NOTE: If you return 0xff for STALL, 'usbFunctionWrite()' may still be called
 * for the remaining data. You must continue to return 0xff for STALL in these
 * calls.
 * In order to get usbFunctionWrite() called, define USB_CFG_IMPLEMENT_FN_WRITE
 * to 1 in usbconfig.h and return 0xff in usbFunctionSetup()..
 */
#endif /* USB_CFG_IMPLEMENT_FN_WRITE */
#if USB_CFG_IMPLEMENT_FN_READ
extern uchar usbFunctionRead(uchar *data, uchar len);
/* This function is called by the driver to ask the application for a control
 * transfer's payload data (control-in). It is called in chunks of up to 8
 * bytes each. You should copy the data to the location given by 'data' and
 * return the actual number of bytes copied. If you return less than requested,
 * the control-in transfer is terminated. If you return 0xff, the driver aborts
 * the transfer with a STALL token.
 * In order to get usbFunctionRead() called, define USB_CFG_IMPLEMENT_FN_READ
 * to 1 in usbconfig.h and return 0xff in usbFunctionSetup()..
 */
#endif /* USB_CFG_IMPLEMENT_FN_READ */
#ifdef USB_CFG_PULLUP_IOPORT
#define usbDeviceConnect()      ((USB_PULLUP_DDR |= (1<<USB_CFG_PULLUP_BIT)), \
                                  (USB_PULLUP_OUT |= (1<<USB_CFG_PULLUP_BIT)))
/* This macro (intended to look like a function) connects the device to the
 * USB bus. It is only available if you have defined the constants
 * USB_CFG_PULLUP_IOPORT and USB_CFG_PULLUP_BIT in usbconfig.h.
 */
#define usbDeviceDisconnect()   (USB_PULLUP_OUT &= ~(1<<USB_CFG_PULLUP_BIT))
/* This macro (intended to look like a function) disconnects the device from
 * the USB bus. It is only available if you have defined the constants
 * USB_CFG_PULLUP_IOPORT and USB_CFG_PULLUP_BIT in usbconfig.h.
 */
#endif /* USB_CFG_PULLUP_IOPORT */
extern unsigned usbCrc16(uchar *data, uchar len);
/* This function calculates the binary complement of the data CRC used in
 * USB data packets. The value is used to build raw transmit packets.
 * You may want to use this function for data checksums or to verify received
 * data.
 */
extern unsigned usbCrc16Append(unsigned char *data, unsigned char len);
/* This function is equivalent to usbCrc16() above, except that it appends
 * the 2 bytes CRC (lowbyte first) in the 'data' buffer after reading 'len'
 * bytes.
 */
extern uchar    usbConfiguration;
/* This value contains the current configuration set by the host. The driver
 * allows setting and querying of this variable with the USB SET_CONFIGURATION
 * and GET_CONFIGURATION requests, but does not use it otherwise.
 * You may want to reflect the "configured" status with a LED on the device or
 * switch on high power parts of the circuit only if the device is configured.
 */
#define USB_STRING_DESCRIPTOR_HEADER(stringLength) ((2*(stringLength)+2) | (3<<8))
/* This macro builds a descriptor header for a string descriptor given the
 * string's length. See usbdrv.c for an example how to use it.
 */
#if USB_CFG_SERIAL_NUMBER_LENGTH
extern PROGMEM int  usbCfgSerialNumberStringDescriptor[];
/* This array of unicode characters (prefixed by a string descriptor header as
 * explained above) represents the serial number of the device.
 */
#endif

#endif  /* __ASSEMBLER__ */

/* ------------------------------------------------------------------------- */
/* ------------------------- Constant definitions -------------------------- */
/* ------------------------------------------------------------------------- */

#if USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH && (!defined USB_CFG_VENDOR_ID || !defined USB_CFG_DEVICE_ID)
#error "You MUST NOT use obdev's shared VID/PID with HID class devices!"
/* The shared VID/PID must be used in conjunction with libusb (see license for
 * the IDs). This contradicts HID usage (at least on Windows).
 */
#endif

/* make sure we have a VID and PID defined, byte order is lowbyte, highbyte */
#ifndef USB_CFG_VENDOR_ID
#   define  USB_CFG_VENDOR_ID   0xc0, 0x16  /* 5824 in dec, stands for VOTI */
#endif

#ifndef USB_CFG_DEVICE_ID
#   define  USB_CFG_DEVICE_ID   0xdc, 0x05  /* 1500 in dec, obdev's free PID */
#endif

#ifndef USB_BUFFER_SECTION
#   define  USB_BUFFER_SECTION  ".bss"      /* if user has not selected a named section */
#endif

/* I/O definitions for assembler module */
#define USBOUT          USB_CFG_IOPORT          /* output port for USB bits */
#define USB_PULLUP_OUT  USB_CFG_PULLUP_IOPORT
#ifdef __ASSEMBLER__
/* the following two lines must start in column 0 for IAR assembler */
USBIN = (USB_CFG_IOPORT - 2)                /* input port for USB bits */
USBDDR = (USB_CFG_IOPORT - 1)               /* data direction for USB bits */
#else
#define USBIN           (*(&USB_CFG_IOPORT - 2))    /* input port for USB bits */
#define USBDDR          (*(&USB_CFG_IOPORT - 1))    /* data direction for USB bits */
#define USB_PULLUP_DDR  (*(&USB_CFG_PULLUP_IOPORT - 1))
#endif
#if USB_CFG_DMINUS_BIT != 0
#   error "USB_CFG_DMINUS_BIT MUST be 0!"
#endif
#define USBMINUS    0       /* D- MUST be on bit 0 */
#define USBIDLE     0x01    /* value representing J state */
#define USBMASK     ((1<<USB_CFG_DPLUS_BIT) | 1)    /* mask for USB I/O bits */

#define USB_BUFSIZE     11  /* PID, 8 bytes data, 2 bytes CRC */

/* Try to find registers and bits responsible for ext interrupt 0 */

#if defined EICRA
#   define  USB_INTR_CFG    EICRA
#else
#   define  USB_INTR_CFG    MCUCR
#endif
#define USB_INTR_CFG_SET    ((1 << ISC00) | (1 << ISC01))   /* cfg for rising edge */
#define USB_INTR_CFG_CLR    0   /* no bits to clear */

#if defined GIMSK
#   define  USB_INTR_ENABLE     GIMSK
#elif defined EIMSK
#   define  USB_INTR_ENABLE     EIMSK
#else
#   define  USB_INTR_ENABLE     GICR
#endif
#define USB_INTR_ENABLE_BIT     INT0

#if defined EIFR
#   define  USB_INTR_PENDING    EIFR
#else
#   define  USB_INTR_PENDING    GIFR
#endif
#define USB_INTR_PENDING_BIT    INTF0

/*
The defines above don't work for the following chips
at90c8534: no ISC0?, no PORTB, can't find a data sheet
at86rf401: no PORTB, no MCUCR etc, low clock rate
atmega103: no ISC0? (maybe omission in header, can't find data sheet)
atmega603: not defined in avr-libc
at43usb320, at43usb355, at76c711: have USB anyway
at94k: is different...

at90s1200, attiny11, attiny12, attiny15, attiny28: these have no RAM
*/

/* ------------------------------------------------------------------------- */
/* ----------------- USB Specification Constants and Types ----------------- */
/* ------------------------------------------------------------------------- */

/* USB Token values */
#define USBPID_SETUP    0x2d
#define USBPID_OUT      0xe1
#define USBPID_IN       0x69
#define USBPID_DATA0    0xc3
#define USBPID_DATA1    0x4b

#define USBPID_ACK      0xd2
#define USBPID_NAK      0x5a
#define USBPID_STALL    0x1e

#ifndef __ASSEMBLER__
typedef union usbWord{
    unsigned    word;
    uchar       bytes[2];
}usbWord_t;

typedef struct usbRequest{
    uchar       bmRequestType;
    uchar       bRequest;
    usbWord_t   wValue;
    usbWord_t   wIndex;
    usbWord_t   wLength;
}usbRequest_t;
/* This structure matches the 8 byte setup request */
#endif

/* bmRequestType field in USB setup:
 * d t t r r r r r, where
 * d ..... direction: 0=host->device, 1=device->host
 * t ..... type: 0=standard, 1=class, 2=vendor, 3=reserved
 * r ..... recipient: 0=device, 1=interface, 2=endpoint, 3=other
 */

/* USB setup recipient values */
#define USBRQ_RCPT_MASK         0x1f
#define USBRQ_RCPT_DEVICE       0
#define USBRQ_RCPT_INTERFACE    1
#define USBRQ_RCPT_ENDPOINT     2

/* USB request type values */
#define USBRQ_TYPE_MASK         0x60
#define USBRQ_TYPE_STANDARD     (0<<5)
#define USBRQ_TYPE_CLASS        (1<<5)
#define USBRQ_TYPE_VENDOR       (2<<5)

/* USB direction values: */
#define USBRQ_DIR_MASK              0x80
#define USBRQ_DIR_HOST_TO_DEVICE    (0<<7)
#define USBRQ_DIR_DEVICE_TO_HOST    (1<<7)

/* USB Standard Requests */
#define USBRQ_GET_STATUS        0
#define USBRQ_CLEAR_FEATURE     1
#define USBRQ_SET_FEATURE       3
#define USBRQ_SET_ADDRESS       5
#define USBRQ_GET_DESCRIPTOR    6
#define USBRQ_SET_DESCRIPTOR    7
#define USBRQ_GET_CONFIGURATION 8
#define USBRQ_SET_CONFIGURATION 9
#define USBRQ_GET_INTERFACE     10
#define USBRQ_SET_INTERFACE     11
#define USBRQ_SYNCH_FRAME       12

/* USB descriptor constants */
#define USBDESCR_DEVICE         1
#define USBDESCR_CONFIG         2
#define USBDESCR_STRING         3
#define USBDESCR_INTERFACE      4
#define USBDESCR_ENDPOINT       5
#define USBDESCR_HID            0x21
#define USBDESCR_HID_REPORT     0x22
#define USBDESCR_HID_PHYS       0x23

#define USBATTR_BUSPOWER        0x80
#define USBATTR_SELFPOWER       0x40
#define USBATTR_REMOTEWAKE      0x20

/* USB HID Requests */
#define USBRQ_HID_GET_REPORT    0x01
#define USBRQ_HID_GET_IDLE      0x02
#define USBRQ_HID_GET_PROTOCOL  0x03
#define USBRQ_HID_SET_REPORT    0x09
#define USBRQ_HID_SET_IDLE      0x0a
#define USBRQ_HID_SET_PROTOCOL  0x0b

/* ------------------------------------------------------------------------- */

#endif /* __usbdrv_h_included__ */
