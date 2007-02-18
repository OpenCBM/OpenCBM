/* Name: usbdrv.c
 * Project: AVR USB driver
 * Author: Christian Starkjohann
 * Creation Date: 2004-12-29
 * Tabsize: 4
 * Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: Proprietary, free under certain conditions. See Documentation.
 * This Revision: $Id: usbdrv.c,v 1.1 2007-02-18 19:47:32 harbaum Exp $
 */

#include "iarcompat.h"
#ifndef __IAR_SYSTEMS_ICC__
#   include <avr/io.h>
#   include <avr/pgmspace.h>
#endif
#include "usbdrv.h"
#include "oddebug.h"

/*
General Description:
This module implements the C-part of the USB driver. See usbdrv.h for a
documentation of the entire driver.
*/

#ifndef IAR_SECTION
#define IAR_SECTION(arg)
#define __no_init
#endif
/* The macro IAR_SECTION is a hack to allow IAR-cc compatibility. On gcc, it
 * is defined to nothing. __no_init is required on IAR.
 */

/* ------------------------------------------------------------------------- */

/* raw USB registers / interface to assembler code: */
/* usbRxBuf MUST be in 1 byte addressable range (because usbInputBuf is only 1 byte) */
__no_init uchar usbRxBuf[2][USB_BUFSIZE] __attribute__ ((section (USB_BUFFER_SECTION))) IAR_SECTION(USB_BUFFER_SECTION);/* raw RX buffer: PID, 8 bytes data, 2 bytes CRC */
uchar       usbDeviceAddr;      /* assigned during enumeration, defaults to 0 */
uchar       usbNewDeviceAddr;   /* device ID which should be set after status phase */
uchar       usbConfiguration;   /* currently selected configuration. Administered by driver, but not used */
uchar       usbInputBuf;        /* ptr to raw buffer used for receiving */
uchar       usbAppBuf;          /* ptr to raw buffer passed to app for processing */
volatile schar usbRxLen;        /* = 0; number of bytes in usbAppBuf; 0 means free */
uchar       usbCurrentTok;      /* last token received */
uchar       usbRxToken;         /* token for data we received */
uchar       usbMsgLen = 0xff;   /* remaining number of bytes, no msg to send if -1 (see usbMsgPtr) */
volatile schar usbTxLen = -1;   /* number of bytes to transmit with next IN token */
uchar       usbTxBuf[USB_BUFSIZE];/* data to transmit with next IN, free if usbTxLen == -1 */
#if USB_CFG_HAVE_INTRIN_ENDPOINT
/* uchar       usbRxEndp;          endpoint which was addressed (1 bit in MSB) [not impl] */
volatile schar usbTxLen1 = -1;  /* TX count for endpoint 1 */
uchar       usbTxBuf1[USB_BUFSIZE];/* TX data for endpoint 1 */
#endif
uchar       usbAckBuf[1] = {USBPID_ACK};    /* transmit buffer for ack tokens */
uchar       usbNakBuf[1] = {USBPID_NAK};    /* transmit buffer for nak tokens */

/* USB status registers / not shared with asm code */
uchar           *usbMsgPtr;     /* data to transmit next -- ROM or RAM address */
static uchar    usbMsgFlags;    /* flag values see below */
static uchar    usbIsReset;     /* = 0; USB bus is in reset phase */

#define USB_FLG_TX_PACKET       (1<<0)
/* Leave free 6 bits after TX_PACKET. This way we can increment usbMsgFlags to toggle TX_PACKET */
#define USB_FLG_MSGPTR_IS_ROM   (1<<6)
#define USB_FLG_USE_DEFAULT_RW  (1<<7)

/*
optimizing hints:
- do not post/pre inc/dec integer values in operations
- assign value of PRG_RDB() to register variables and don't use side effects in arg
- use narrow scope for variables which should be in X/Y/Z register
- assign char sized expressions to variables to force 8 bit arithmetics
*/

/* ------------------------------------------------------------------------- */

static PROGMEM char usbDescrDevice[] = {    /* USB device descriptor */
    18,         /* sizeof(usbDescrDevice): length of descriptor in bytes */
    USBDESCR_DEVICE,    /* descriptor type */
    0x01, 0x01, /* USB version supported */
    USB_CFG_DEVICE_CLASS,
    USB_CFG_DEVICE_SUBCLASS,
    0,          /* protocol */
    8,          /* max packet size */
    USB_CFG_VENDOR_ID,  /* 2 bytes */
    USB_CFG_DEVICE_ID,  /* 2 bytes */
    USB_CFG_DEVICE_VERSION, /* 2 bytes */
#if USB_CFG_VENDOR_NAME_LEN
    1,          /* manufacturer string index */
#else
    0,          /* manufacturer string index */
#endif
#if USB_CFG_DEVICE_NAME_LEN
    2,          /* product string index */
#else
    0,          /* product string index */
#endif
#if USB_CFG_SERIAL_NUMBER_LENGTH
    3,          /* serial number string index */
#else
    0,          /* serial number string index */
#endif
    1,          /* number of configurations */
};

static PROGMEM char usbDescrConfig[] = {    /* USB configuration descriptor */
    9,          /* sizeof(usbDescrConfig): length of descriptor in bytes */
    USBDESCR_CONFIG,    /* descriptor type */
    (18 + 7 * USB_CFG_HAVE_INTRIN_ENDPOINT
#if USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
     + 9
#endif
        ), 0,   /* total length of data returned (including inlined descriptors) */
    1,          /* number of interfaces in this configuration */
    1,          /* index of this configuration */
    0,          /* configuration name string index */
#if USB_CFG_IS_SELF_POWERED
    USBATTR_SELFPOWER,  /* attributes */
#else
    USBATTR_BUSPOWER,   /* attributes */
#endif
    USB_CFG_MAX_BUS_POWER/2,            /* max USB current in 2mA units */
/* interface descriptor follows inline: */
    9,          /* sizeof(usbDescrInterface): length of descriptor in bytes */
    USBDESCR_INTERFACE, /* descriptor type */
    0,          /* index of this interface */
    0,          /* alternate setting for this interface */
    USB_CFG_HAVE_INTRIN_ENDPOINT,   /* endpoints excl 0: number of endpoint descriptors to follow */
    USB_CFG_INTERFACE_CLASS,
    USB_CFG_INTERFACE_SUBCLASS,
    USB_CFG_INTERFACE_PROTOCOL,
    0,          /* string index for interface */
#if USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH    /* HID descriptor */
    9,          /* sizeof(usbDescrHID): length of descriptor in bytes */
    USBDESCR_HID,   /* descriptor type: HID */
    0x01, 0x01, /* BCD representation of HID version */
    0x00,       /* target country code */
    0x01,       /* number of HID Report (or other HID class) Descriptor infos to follow */
    0x22,       /* descriptor type: report */
    USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH, 0,  /* total length of report descriptor */
#endif
#if USB_CFG_HAVE_INTRIN_ENDPOINT    /* endpoint descriptor for endpoint 1 */
    7,          /* sizeof(usbDescrEndpoint) */
    USBDESCR_ENDPOINT,  /* descriptor type = endpoint */
    0x81,       /* IN endpoint number 1 */
    0x03,       /* attrib: Interrupt endpoint */
    8, 0,       /* maximum packet size */
    USB_CFG_INTR_POLL_INTERVAL, /* in ms */
#endif
};

static PROGMEM char usbDescrString0[] = {   /* language descriptor */
    4,          /* sizeof(usbDescrString0): length of descriptor in bytes */
    3,          /* descriptor type */
    0x09, 0x04, /* language index (0x0409 = US-English) */
};

#if USB_CFG_VENDOR_NAME_LEN
static PROGMEM int  usbDescrString1[] = {
    USB_STRING_DESCRIPTOR_HEADER(USB_CFG_VENDOR_NAME_LEN),
    USB_CFG_VENDOR_NAME
};
#endif
#if USB_CFG_DEVICE_NAME_LEN
static PROGMEM int  usbDescrString2[] = {
    USB_STRING_DESCRIPTOR_HEADER(USB_CFG_DEVICE_NAME_LEN),
    USB_CFG_DEVICE_NAME
};
#endif

/* We don't use prog_int or prog_int16_t for compatibility with various libc
 * versions. Here's an other compatibility hack:
 */
#ifndef PRG_RDB
#define PRG_RDB(addr)   pgm_read_byte(addr)
#endif

typedef union{
    unsigned    word;
    uchar       *ptr;
    uchar       bytes[2];
}converter_t;
/* We use this union to do type conversions. This is better optimized than
 * type casts in gcc 3.4.3 and much better than using bit shifts to build
 * ints from chars. Byte ordering is not a problem on an 8 bit platform.
 */

/* ------------------------------------------------------------------------- */

#if USB_CFG_HAVE_INTRIN_ENDPOINT
static uchar    usbTxPacketCnt1;
#if USB_CFG_IMPLEMENT_HALT
static uchar    usbHalted1;         /* not 0 if endpoint 1 is halted */
#endif

void    usbSetInterrupt(uchar *data, uchar len)
{
uchar       *p, i;

#if USB_CFG_IMPLEMENT_HALT
    if(usbHalted1)
        return;
#endif
    if(len > 8) /* interrupt transfers are limited to 8 bytes */
        len = 8;
    i = USBPID_DATA1;
    if(usbTxPacketCnt1 & 1)
        i = USBPID_DATA0;
    if(usbTxLen1 < 0){      /* packet buffer was empty */
        usbTxPacketCnt1++;
    }else{
        usbTxLen1 = -1;     /* avoid sending incomplete interrupt data */
    }
    p = usbTxBuf1;
    *p++ = i;
    for(i=len;i--;)
        *p++ = *data++;
    usbCrc16Append(&usbTxBuf1[1], len);
    usbTxLen1 = len + 4;    /* len must be given including sync byte */
#if DEBUG_LEVEL > 1
    DBG2(0x21, usbTxBuf1, usbTxLen1-1);
#else
    DBG1(0x21, usbTxBuf1 + 1, 2);
#endif
}
#endif


static uchar    usbRead(uchar *data, uchar len)
{
#if USB_CFG_IMPLEMENT_FN_READ
    if(usbMsgFlags & USB_FLG_USE_DEFAULT_RW){
#endif
        uchar i = len, *r = usbMsgPtr;
        if(usbMsgFlags & USB_FLG_MSGPTR_IS_ROM){    /* ROM data */
            while(i--){
                uchar c = PRG_RDB(r);    /* assign to char size variable to enforce byte ops */
                *data++ = c;
                r++;
            }
        }else{                  /* RAM data */
            while(i--)
                *data++ = *r++;
        }
        usbMsgPtr = r;
        return len;
#if USB_CFG_IMPLEMENT_FN_READ
    }else{
        if(len != 0)    /* don't bother app with 0 sized reads */
            return usbFunctionRead(data, len);
        return 0;
    }
#endif
}

/* Don't make this function static to avoid inlining.
 * The entire function would become too large and exceed the range of
 * relative jumps.
 * 2006-02-25: Either gcc 3.4.3 is better than the gcc used when the comment
 * above was written, or other parts of the code have changed. We now get
 * better results with an inlined function. Test condition: PowerSwitch code.
 */
static void usbProcessRx(uchar *data, uchar len)
{
usbRequest_t    *rq = (void *)data;
uchar           replyLen = 0, flags = USB_FLG_USE_DEFAULT_RW;
/* We use if() cascades because the compare is done byte-wise while switch()
 * is int-based. The if() cascades are therefore more efficient.
 */
#if DEBUG_LEVEL > 1
    DBG2(0x10 + (usbRxToken == (uchar)USBPID_SETUP), data, len);
#else
    DBG1(0x10 + (usbRxToken == (uchar)USBPID_SETUP), data, 2);
#endif
    if(usbRxToken == (uchar)USBPID_SETUP){
        if(len == 8){   /* Setup size must be always 8 bytes. Ignore otherwise. */
            uchar type = rq->bmRequestType & USBRQ_TYPE_MASK;
            if(type == USBRQ_TYPE_STANDARD){
                uchar *replyData = usbTxBuf + 9; /* there is 3 bytes free space at the end of the buffer */
                replyData[0] = 0;   /* common to USBRQ_GET_STATUS and USBRQ_GET_INTERFACE */
                if(rq->bRequest == USBRQ_GET_STATUS){           /* 0 */
                    uchar __attribute__((__unused__)) recipient = rq->bmRequestType & USBRQ_RCPT_MASK;  /* assign arith ops to variables to enforce byte size */
#if USB_CFG_IS_SELF_POWERED
                    if(recipient == USBRQ_RCPT_DEVICE)
                        replyData[0] =  USB_CFG_IS_SELF_POWERED;
#endif
#if USB_CFG_HAVE_INTRIN_ENDPOINT && USB_CFG_IMPLEMENT_HALT
                    if(usbHalted1 && recipient == USBRQ_RCPT_ENDPOINT && rq->wIndex.bytes[0] == 0x81)   /* request status for endpoint 1 */
                        replyData[0] = 1;
#endif
                    replyData[1] = 0;
                    replyLen = 2;
                }else if(rq->bRequest == USBRQ_SET_ADDRESS){    /* 5 */
                    usbNewDeviceAddr = rq->wValue.bytes[0];
                }else if(rq->bRequest == USBRQ_GET_DESCRIPTOR){ /* 6 */
                    flags = USB_FLG_MSGPTR_IS_ROM | USB_FLG_USE_DEFAULT_RW;
                    if(rq->wValue.bytes[1] == 1){   /* descriptor type requested */
                        replyLen = sizeof(usbDescrDevice);
                        replyData = (uchar *)usbDescrDevice;
                    }else if(rq->wValue.bytes[1] == 2){
                        replyLen = sizeof(usbDescrConfig);
                        replyData = (uchar *)usbDescrConfig;
                    }else if(rq->wValue.bytes[1] == 3){ /* string descriptor */
                        if(rq->wValue.bytes[0] == 0){   /* descriptor index */
                            replyLen = sizeof(usbDescrString0);
                            replyData = (uchar *)usbDescrString0;
#if USB_CFG_VENDOR_NAME_LEN
                        }else if(rq->wValue.bytes[0] == 1){
                            replyLen = sizeof(usbDescrString1);
                            replyData = (uchar *)usbDescrString1;
#endif
#if USB_CFG_DEVICE_NAME_LEN
                        }else if(rq->wValue.bytes[0] == 2){
                            replyLen = sizeof(usbDescrString2);
                            replyData = (uchar *)usbDescrString2;
#endif
#if USB_CFG_SERIAL_NUMBER_LENGTH
                        }else if(rq->wValue.bytes[0] == 3){
                            replyLen = 2 * USB_CFG_SERIAL_NUMBER_LENGTH + 2;
                            replyData = (uchar *)usbCfgSerialNumberStringDescriptor;
#endif
                        }
                    }
#if USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH
                    else if(rq->wValue.bytes[1] == USBDESCR_HID){           /* 0x21 */
                        replyLen = 9;
                        replyData = (uchar *)usbDescrConfig + 18;
                    }else if(rq->wValue.bytes[1] == USBDESCR_HID_REPORT){   /* 0x22 */
                        replyLen = USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH;
                        replyData = (uchar *)usbHidReportDescriptor;
                    }
#endif
                }else if(rq->bRequest == USBRQ_GET_CONFIGURATION){  /* 8 */
                    replyLen = 1;
                    replyData = &usbConfiguration;  /* send current configuration value */
                }else if(rq->bRequest == USBRQ_SET_CONFIGURATION){  /* 9 */
                    usbConfiguration = rq->wValue.bytes[0];
#if USB_CFG_IMPLEMENT_HALT
                    usbHalted1 = 0;
#endif
                }else if(rq->bRequest == USBRQ_GET_INTERFACE){      /* 10 */
                    replyLen = 1;
#if USB_CFG_HAVE_INTRIN_ENDPOINT
#if USB_CFG_IMPLEMENT_HALT
                }else if(rq->bRequest == USBRQ_CLEAR_FEATURE || rq->bRequest == USBRQ_SET_FEATURE){   /* 1|3 */
                    if(rq->wValue.bytes[0] == 0 && rq->wIndex.bytes[0] == 0x81){   /* feature 0 == HALT for endpoint == 1 */
                        usbHalted1 = rq->bRequest - 1;
                        if(usbHalted1){
                            usbTxBuf1[0] = USBPID_STALL;
                            usbTxLen1 = 2;      /* length including sync byte */
                        }
                        usbTxPacketCnt1 = 0;    /* reset data toggling for interrupt endpoint */
                    }
#endif
                }else if(rq->bRequest == USBRQ_SET_INTERFACE){      /* 11 */
                    usbTxPacketCnt1 = 0;        /* reset data toggling for interrupt endpoint */
#if USB_CFG_IMPLEMENT_HALT
                    usbHalted1 = 0;
#endif
#endif
                }else{
                    /* the following requests can be ignored, send default reply */
                    /* 1: CLEAR_FEATURE, 3: SET_FEATURE, 7: SET_DESCRIPTOR */
                    /* 12: SYNCH_FRAME */
                }
                usbMsgPtr = replyData;
                if(!rq->wLength.bytes[1] && replyLen > rq->wLength.bytes[0])  /* max length is in */
                    replyLen = rq->wLength.bytes[0];
            }else{  /* not a standard request -- must be vendor or class request */
                replyLen = usbFunctionSetup(data);
#if USB_CFG_IMPLEMENT_FN_READ || USB_CFG_IMPLEMENT_FN_WRITE
                if(replyLen == 0xff){   /* use user-supplied read/write function */
                    if((rq->bmRequestType & USBRQ_DIR_MASK) == USBRQ_DIR_DEVICE_TO_HOST){
                        replyLen = rq->wLength.bytes[0];    /* IN transfers only */
                    }
                    flags = 0;  /* we have no valid msg, use user supplied read/write functions */
                }
#endif
            }
        }
        /* make sure that data packets which are sent as ACK to an OUT transfer are always zero sized */
    }else{  /* DATA packet from out request */
#if USB_CFG_IMPLEMENT_FN_WRITE
        if(!(usbMsgFlags & USB_FLG_USE_DEFAULT_RW)){
            uchar rval = usbFunctionWrite(data, len);
            replyLen = 0xff;
            if(rval == 0xff){       /* an error occurred */
                /* usbMsgLen = 0xff; cancel potentially pending ACK [has been done by ASM module when OUT token arrived] */
                usbTxBuf[0] = USBPID_STALL;
                usbTxLen = 2;       /* length including sync byte */
            }else if(rval != 0){    /* This was the final package */
                replyLen = 0;       /* answer with a zero-sized data packet */
            }
            flags = 0;    /* start with a DATA1 package, stay with user supplied write() function */
        }
#else
        replyLen = 0;      /* send zero-sized block as ACK */
#endif
    }
    usbMsgFlags = flags;
    usbMsgLen = replyLen;
}

/* ------------------------------------------------------------------------- */

static void usbBuildTxBlock(void)
{
uchar       wantLen, len, txLen, token;

    wantLen = usbMsgLen;
    if(wantLen > 8)
        wantLen = 8;
    usbMsgLen -= wantLen;
    token = USBPID_DATA1;
    if(usbMsgFlags & USB_FLG_TX_PACKET)
        token = USBPID_DATA0;
    usbMsgFlags++;
    len = usbRead(usbTxBuf + 1, wantLen);
    if(len <= 8){           /* valid data packet */
        usbCrc16Append(usbTxBuf + 1, len);
        txLen = len + 4;    /* length including sync byte */
        if(len < 8)         /* a partial package identifies end of message */
            usbMsgLen = 0xff;
    }else{
        token = USBPID_STALL;
        txLen = 2;          /* length including sync byte */
        usbMsgLen = 0xff;
    }
    usbTxBuf[0] = token;
    usbTxLen = txLen;
#if DEBUG_LEVEL > 1
    DBG2(0x20, usbTxBuf, txLen-1);
#else
    DBG1(0x20, usbTxBuf + 1, 2);
#endif
}

static inline uchar isNotSE0(void)
{
uchar   rval;
/* We want to do
 *     return (USBIN & USBMASK);
 * here, but the compiler does int-expansion acrobatics.
 * We can avoid this by assigning to a char-sized variable.
 */
    rval = USBIN & USBMASK;
    return rval;
}

/* ------------------------------------------------------------------------- */

void    usbPoll(void)
{
uchar   len;

    if((len = usbRxLen) > 0){
/* We could check CRC16 here -- but ACK has already been sent anyway. If you
 * need data integrity checks with this driver, check the CRC in your app
 * code and report errors back to the host. Since the ACK was already sent,
 * retries must be handled on application level.
 * unsigned crc = usbCrc16((uchar *)(unsigned)(usbAppBuf + 1), usbRxLen - 3);
 */
        len -= 3;       /* remove PID and CRC */
        if(len < 128){  /* no overflow */
            converter_t appBuf;
            appBuf.ptr = (uchar *)usbRxBuf;
            appBuf.bytes[0] = usbAppBuf;
            appBuf.bytes[0]++;
            usbProcessRx(appBuf.ptr, len);
        }
        usbRxLen = 0;   /* mark rx buffer as available */
    }
    if(usbMsgLen != 0xff){  /* transmit data pending? */
        if(usbTxLen < 0)    /* transmit system idle */
            usbBuildTxBlock();
    }
    if(isNotSE0()){ /* SE0 state */
        usbIsReset = 0;
    }else{
        /* check whether SE0 lasts for more than 2.5us (3.75 bit times) */
        if(!usbIsReset){
            uchar i;
            for(i=100;i;i--){
                if(isNotSE0())
                    goto notUsbReset;
            }
            usbIsReset = 1;
            usbNewDeviceAddr = 0;
            usbDeviceAddr = 0;
#if USB_CFG_IMPLEMENT_HALT
            usbHalted1 = 0;
#endif
            DBG1(0xff, 0, 0);
notUsbReset:;
        }
    }
}

/* ------------------------------------------------------------------------- */

void    usbInit(void)
{
    usbInputBuf = (uchar)usbRxBuf[0];
    usbAppBuf = (uchar)usbRxBuf[1];
#if USB_INTR_CFG_SET != 0
    USB_INTR_CFG |= USB_INTR_CFG_SET;
#endif
#if USB_INTR_CFG_CLR != 0
    USB_INTR_CFG &= ~(USB_INTR_CFG_CLR);
#endif
    USB_INTR_ENABLE |= (1 << USB_INTR_ENABLE_BIT);
}

/* ------------------------------------------------------------------------- */
