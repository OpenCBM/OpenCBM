/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
*/

#define CBMCTRL_TALK	    0
#define CBMCTRL_LISTEN	    1
#define CBMCTRL_UNTALK      2
#define CBMCTRL_UNLISTEN    3
#define CBMCTRL_OPEN        4
#define CBMCTRL_CLOSE       5
#define CBMCTRL_RESET       6

#define CBMCTRL_PP_READ     10
#define CBMCTRL_PP_WRITE    11
#define CBMCTRL_IEC_POLL    12
#define CBMCTRL_IEC_SET     13
#define CBMCTRL_IEC_RELEASE 14
#define CBMCTRL_PAR_READ    15
#define CBMCTRL_PAR_WRITE   16

/* lpt lines for XE1541 cable */
#define XE_ATN_OUT    0x01
#define XE_CLK_OUT    0x02
#define XE_DATA_OUT   0x08
#define XE_RESET_OUT  0x04
#define XE_OUTMASK    0x04

#define XE_ATN_IN     0x10
#define XE_CLK_IN     0x20
#define XE_DATA_IN    0x80
#define XE_RESET_IN   0x40
#define XE_INMASK     0x80

/* lpt lines for XA1541 cable */
#define XA_ATN_OUT    0x01
#define XA_CLK_OUT    0x02
#define XA_DATA_OUT   0x04
#define XA_RESET_OUT  0x08
#define XA_OUTMASK    0x0b

#define XA_ATN_IN     0x10
#define XA_CLK_IN     0x20
#define XA_DATA_IN    0x40
#define XA_RESET_IN   0x80
#define XA_INMASK     0x80


/*
 * lpt lines for XM1541 cable, added by Womo 20041211,
 * same as XA1541, but output inverted again
 */
#define XM_ATN_OUT    0x01
#define XM_CLK_OUT    0x02
#define XM_DATA_OUT   0x04
#define XM_RESET_OUT  0x08
#define XM_OUTMASK    0x04

#define XM_ATN_IN     0x10
#define XM_CLK_IN     0x20
#define XM_DATA_IN    0x40
#define XM_RESET_IN   0x80
#define XM_INMASK     0x80

#define SET(line)       (outportb(serport+2,(*serportval|=(line))^OUTMASK))
#define RELEASE(line)   (outportb(serport+2,(*serportval&=~(line))^OUTMASK))
#define GET(line)       (((inportb(serport+1)^INMASK)&(line))==0?1:0)

/* global variables */
extern int ATN_OUT, CLK_OUT, DATA_OUT, RESET_OUT, OUTMASK;
extern int ATN_IN, CLK_IN, DATA_IN, RESET_IN, INMASK;
extern unsigned int serport;		/* 'serial' LPT port address */
extern unsigned int parport;		/* 'parallel' LPT port address */
extern unsigned char *serportval;	/* current value in output register */
extern unsigned char *parportval;	/* current value in output register */
extern unsigned char portval[4];

/* prototypes */
void set_xe(void);
void set_xa(void);
void set_xm(void);
void calibrate(void);
void msleep(unsigned long usec);
void show(char * s);
void do_reset(void);
int send_byte(int b);
void wait_for_listener(void);
void release_all(void);
int cbm_raw_read(int f, char * buf, int count);
int cbm_raw_write(int f, char * buf, int cnt);
int cbm_ioctl(int f, unsigned int cmd, unsigned long arg);
int cbm_release(int f);
int scan_xe1541(unsigned int port);
int set_par_port(int port);
int detect_ports(int reset);

