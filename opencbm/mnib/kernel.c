/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 *
 *	Modified for DOS use by Markus Brenner <markus@brenner.de>
 *	XM1541 support by Wolfgang Moser <w.moser@gm.fh-koeln.de>
 *
 *  MNIB Write support + consolidation by Pete Rittwage <peter@rittwage.com>
*/

// #define DEBUG

#include <stdio.h>	/* printk substituted by printf */
#include <unistd.h>	/* usleep() function */
#include <errno.h>	/* EINVAL */
#include <time.h>	/* PC specific includes (outb, inb) */

/* DOS stuff */
#ifdef DJGPP
#include <dos.h>			/* delay() */
#include <sys/movedata.h>	/* _dosmemgetb() */
#include "cbm.h"
#include "kernel.h"
#endif // DJGPP

#if 0
unsigned int serport = 0x378;	/* 'serial' LPT port address */
unsigned int serport = 0x3bc;	/* 'serial' LPT port address */
unsigned int parport = 0x378;	/* 'parallel' LPT port address */
#endif

unsigned int serport;		/* 'serial' LPT port address */
unsigned int parport;		/* 'parallel' LPT port address */
int ATN_OUT, CLK_OUT, DATA_OUT, RESET_OUT, OUTMASK;
int ATN_IN, CLK_IN, DATA_IN, RESET_IN, INMASK;

/* cable types, indexed from 0 */
enum cable_types
{
	CABLE_TYPE_XE1541, CABLE_TYPE_XA1541,
	CABLE_TYPE_XM1541, CABLE_TYPE_NUM
};

char cableSpec[] = { 'E', 'A', 'M' };

int lpt_num;				/* # of available printer ports */
unsigned int lpt[4];		/* port addresses */

unsigned char *serportval;	/* current value in output register */
unsigned char *parportval;	/* current value in output register */
unsigned char portval[4];	/* current value in output register */

#define PARREAD()       (outportb(parport+2,(*parportval|=0x20)^OUTMASK))
#define PARWRITE()      (outportb(parport+2,(*parportval&=0xdf)^OUTMASK))

#ifdef DEBUG
#define DPRINTK(fmt,args...)     printf(fmt, ## args)
#define SHOW(str)                show(str)
#else
#define DPRINTK(fmt,args...)
#define SHOW(str)
#endif

int cbm_iec_raw_write(const char * buf, int cnt, int atn, int talk);

int eoi;
int irq_count;
//uclock_t t_timeout;
int t_timeout;
unsigned long tickspersec = 1000000;

void
set_xe()
{
	ATN_IN = XE_ATN_IN;
	ATN_OUT = XE_ATN_OUT;
	CLK_IN = XE_CLK_IN;
	CLK_OUT = XE_CLK_OUT;
	DATA_IN = XE_DATA_IN;
	DATA_OUT = XE_DATA_OUT;
	RESET_IN = XE_RESET_IN;
	RESET_OUT = XE_RESET_OUT;
	INMASK = XE_INMASK;
	OUTMASK = XE_OUTMASK;
}

void
set_xa()
{
	ATN_IN = XA_ATN_IN;
	ATN_OUT = XA_ATN_OUT;
	CLK_IN = XA_CLK_IN;
	CLK_OUT = XA_CLK_OUT;
	DATA_IN = XA_DATA_IN;
	DATA_OUT = XA_DATA_OUT;
	RESET_IN = XA_RESET_IN;
	RESET_OUT = XA_RESET_OUT;
	INMASK = XA_INMASK;
	OUTMASK = XA_OUTMASK;
}

/* initialisation for XM1541 cable, added by Womo 20041211*/
void
set_xm()
{
	ATN_IN = XM_ATN_IN;
	ATN_OUT = XM_ATN_OUT;
	CLK_IN = XM_CLK_IN;
	CLK_OUT = XM_CLK_OUT;
	DATA_IN = XM_DATA_IN;
	DATA_OUT = XM_DATA_OUT;
	RESET_IN = XM_RESET_IN;
	RESET_OUT = XM_RESET_OUT;
	INMASK = XM_INMASK;
	OUTMASK = XM_OUTMASK;
}

void
calibrate()
{
	clock_t stop;

	stop = clock() + CLOCKS_PER_SEC;
	for (tickspersec = 0; clock() < stop; tickspersec += 10)
	{
		inportb(0x340);
		inportb(0x340);
		inportb(0x340);
		inportb(0x340);
		inportb(0x340);

		inportb(0x340);
		inportb(0x340);
		inportb(0x340);
		inportb(0x340);
		inportb(0x340);
	}
	printf("Timing: %ld\n", tickspersec);

}

void
msleep(unsigned long usec)
{
	unsigned long ticks =
	  (int) ((float) 1.5 * usec * tickspersec / 1000000);
	while (ticks--)
		inportb(0x340);
}

/*
 *  dump input lines
 */
void
show(char *s)
{
	printf("%s:\ndatain=%d, clkin=%d, atnin=%d, resetin=%d\n", s,
	  GET(DATA_IN), GET(CLK_IN), GET(ATN_IN), GET(RESET_IN));
	printf("dataout=%d, clkout=%d, atnout=%d, resetout=%d\n\n",
	  GET(DATA_OUT), GET(CLK_OUT), GET(ATN_OUT), GET(RESET_OUT));
}

void
do_reset(void)
{
	printf("cbm_init: resetting devices\n");
	RELEASE(DATA_OUT | ATN_OUT | CLK_OUT);
	SET(RESET_OUT);
	delay(100);		/* 100ms */
	RELEASE(RESET_OUT);

	printf("cbm_init: sleeping 2 seconds...\n");
	delay(2000);	/* 2s */
}

/*
 *  send byte
 */
int
send_byte(int b)
{
	int i, ack = 0;

	DPRINTK("send_byte %02x\n", b);

	disable();
	for (i = 0; i < 8; i++)
	{
		msleep(70);
		if (!((b >> i) & 1))
		{
			SET(DATA_OUT);
		}
		RELEASE(CLK_OUT);
		msleep(20);
		SET(CLK_OUT);
		RELEASE(DATA_OUT);
	}
	for (i = 0; i < 20 && !(ack = GET(DATA_IN)); i++)
	{
		msleep(100);
	}
	enable();

	DPRINTK("ack=%d\n", ack);

	return ack;
}

/*
 *  wait until listener is ready to receive
 */
void
wait_for_listener()
{
	SHOW("waiting for device");

	disable();
	RELEASE(CLK_OUT);

	while (GET(DATA_IN));
	if (--irq_count == 0)
	{
		SET(CLK_OUT);
		DPRINTK("continue to send (no EOI)\n");
		return;
	}
	DPRINTK("signaling EOI\n");
	msleep(150);
	while (!GET(DATA_IN));
	while (GET(DATA_IN));
	SET(CLK_OUT);
	enable();
}

/*
 *  idle
 */
void
release_all(void)
{
	RELEASE(ATN_OUT | DATA_OUT);
}

int
cbm_raw_read(int f, char * buf, int count)
{
	int received, i, b, bit, ok;

	DPRINTK("cbm_raw_read: %d bytes\n", count);
	received = 0;
	ok = 0;

	if (eoi)
	{
		return 0;
	}

	do
	{
		i = 0;
		while (GET(CLK_IN))
		{
			if (i >= 50)
			{
				delay(20);	/* 20ms */
			}
			else
			{
				i++;
				msleep(20);
			}
		}

		// disable();
		RELEASE(DATA_OUT);
		for (i = 0; i < 40 && !(ok = GET(CLK_IN)); i++)
		{
			msleep(10);
		}

		if (!ok)
		{
			/* device signals eoi */
			eoi = 1;
			SET(DATA_OUT);
			msleep(70);
			RELEASE(DATA_OUT);
		}

		disable();

		for (i = 0; i < 100 && !(ok = GET(CLK_IN)); i++)
		{
			msleep(20);
		}

		for (bit = b = 0; bit < 8 && ok; bit++)
		{
			for (i = 0; i < 200 && !(ok = (GET(CLK_IN) == 0)); i++)
			{
				msleep(10);
			}
			if (ok)
			{
				b >>= 1;
				if (GET(DATA_IN) == 0)
				{
					b |= 0x80;
				}
				for (i = 0; i < 100 && !(ok = GET(CLK_IN)); i++)
				{
					msleep(20);
				}
			}
		}
		if (ok)
		{
			received++;
			SET(DATA_OUT);
			*buf++ = (char) b;
		}

		enable();

		msleep(50);

	} while (received < count && ok && !eoi);

	DPRINTK("received=%d, count=%d, ok=%d, eoi=%d\n",
	  received, count, ok, eoi);

	return received;
}

int
cbm_iec_raw_write(const char * buf, int cnt, int atn, int talk)
{
	unsigned char c;
	int i, rv, sent;

	rv = 0;
	sent = 0;
	eoi = irq_count = 0;

	DPRINTK("cbm_iec_raw_write: %d bytes, atn=%d\n", cnt, atn);

	if (atn)
	{
		SET(ATN_OUT);
	}
	SET(CLK_OUT);
	RELEASE(DATA_OUT);
	GET(DATA_IN);		/* WAIT */

	for (i = 0; i < 100 && !GET(DATA_IN); i++)
	{
		msleep(10);
	}

	if (!GET(DATA_IN))
	{
		printf("cbm: no devices found\n");
		RELEASE(CLK_OUT | ATN_OUT);
		return -ENODEV;
	}

	delay(20);		/* 20ms */


	while (cnt > sent && rv == 0)
	{
		c = *buf++;
		msleep(50);
		irq_count = (sent == cnt - 1 && atn == 0) ? 2 : 1;
		wait_for_listener();

		if (send_byte(c))
		{
			sent++;
			msleep(100);
		}
		else
		{
			printf("cbm: I/O error\n");
			rv = -EIO;
		}
	}
	DPRINTK("%d bytes sent, rv=%d\n", sent, rv);

	if (talk)
	{
		disable();
		SET(DATA_OUT);
		RELEASE(ATN_OUT);
		msleep(30);
		RELEASE(CLK_OUT);
		enable();
	}
	else
	{
		RELEASE(ATN_OUT);
	}
	msleep(100);

	return (rv < 0) ? rv : sent;
}

int
cbm_raw_write(int f, char * buf, int cnt)
{
	return cbm_iec_raw_write(buf, cnt, 0, 0);
}

int
cbm_ioctl(int f, unsigned int cmd, unsigned long arg)
{
	unsigned char buf[2], c, talk;
	int rv;

	rv = 0;
	buf[0] = (arg >> 8) & 0x1f;	/* device */
	buf[1] = arg & 0x0f;	/* secondary address */

	switch (cmd)
	{
	case CBMCTRL_RESET:
		do_reset();
		return 0;

	case CBMCTRL_TALK:
	case CBMCTRL_LISTEN:
		talk = (cmd == CBMCTRL_TALK);
		buf[0] |= talk ? 0x40 : 0x20;
		buf[1] |= 0x60;
		rv = cbm_iec_raw_write((char *)buf, 2, 1, talk);
		return (rv > 0 ? 0 : rv);

	case CBMCTRL_UNTALK:
	case CBMCTRL_UNLISTEN:
		buf[0] = (cmd == CBMCTRL_UNTALK) ? 0x5f : 0x3f;
		rv = cbm_iec_raw_write((char *)buf, 1, 1, 0);
		return (rv > 0 ? 0 : rv);

	case CBMCTRL_OPEN:
	case CBMCTRL_CLOSE:
		buf[0] |= 0x20;
		buf[1] |= (cmd == CBMCTRL_OPEN) ? 0xf0 : 0xe0;
		rv = cbm_iec_raw_write((char *)buf, 2, 1, 0);
		return (rv > 0 ? 0 : rv);

	case CBMCTRL_IEC_POLL:
		c = inportb(serport + 1);
		if ((c & DATA_IN) == 0)
			rv |= IEC_DATA;
		if ((c & CLK_IN) == 0)
			rv |= IEC_CLOCK;
		if ((c & ATN_IN) == 0)
			rv |= IEC_ATN;
		return (rv);

	case CBMCTRL_IEC_SET:
		switch (arg)
		{
		case IEC_DATA:
			SET(DATA_OUT);
			break;
		case IEC_CLOCK:
			SET(CLK_OUT);
			break;
		case IEC_ATN:
			SET(ATN_OUT);
			break;
		default:
			return -EINVAL;
		}
		return (0);

	case CBMCTRL_IEC_RELEASE:
		switch (arg)
		{
		case IEC_DATA:
			RELEASE(DATA_OUT);
			break;
		case IEC_CLOCK:
			RELEASE(CLK_OUT);
			break;
		case IEC_ATN:
			RELEASE(ATN_OUT);
			break;
		default:
			return -EINVAL;
		}
		return (0);

	case CBMCTRL_PP_READ:
		PARREAD();
		rv = inportb(parport);
		PARWRITE();
		return (rv);

	case CBMCTRL_PP_WRITE:
		outportb(parport, arg);
		return (0);

	case CBMCTRL_PAR_READ:
		RELEASE(DATA_OUT | CLK_OUT);
		SET(ATN_OUT);
		msleep(20);	/* 200? */
		// for (j=0; j < 20; j++) GET(DATA_IN);
		while (GET(DATA_IN));
		rv = inportb(parport);
		msleep(5);
		// for (j=0; j < 5; j++) GET(DATA_IN); // extra
		RELEASE(ATN_OUT);
		msleep(10);
		// for (j=0; j < 20; j++) GET(DATA_IN);
		while (!GET(DATA_IN));
		// PARWRITE();
		return (rv);

	case CBMCTRL_PAR_WRITE:
		RELEASE(DATA_OUT | CLK_OUT);
		SET(ATN_OUT);
		msleep(20);
		// for (j=0; j < 20; j++) GET(DATA_IN);
		while (GET(DATA_IN));
		PARWRITE();
		outportb(parport, arg);
		// for (j=0; j < 5; j++) GET(DATA_IN);
		msleep(5);
		RELEASE(ATN_OUT);
		msleep(20);
		// for (j=0; j < 20; j++) GET(DATA_IN);
		while (!GET(DATA_IN));
		PARREAD();

		return (0);
	}
	return (-EINVAL);
}

#if 0
int cbm_open(int f)
{
        MOD_INC_USE_COUNT;
        return (0);
}
#endif // 0

int
cbm_release(int f)
{
	/*
	   MOD_DEC_USE_COUNT;
	 */
	return (0);
}

int
scan_xe1541(unsigned int port)
{
	int i, rv;

	serport = port;
	RELEASE(DATA_OUT | ATN_OUT | CLK_OUT | RESET_OUT);
	delay(100);
	if (GET(RESET_IN))
		return (0);
	if (GET(DATA_IN))
		return (0);
	SET(ATN_OUT);
	SET(CLK_OUT);
	for (i = 0; i < 10 && !GET(DATA_IN); i++)
		msleep(10);
	rv = GET(DATA_IN);
	RELEASE(ATN_OUT | CLK_OUT);
	return (rv);
}

int
set_par_port(int port)
{
	if (port < lpt_num)
	{
		parport = lpt[port];
		parportval = &portval[port];
		PARREAD();
		printf("Port %d: %04x ", port, lpt[port]);
		return (1);
	}
	else
		return (0);
}

int
detect_ports(int reset)
{
	int i, cable;
	unsigned char byte[8];
	unsigned int port;
	int found, goodport;
	unsigned char ecr;

	char *ecpm[8] = {
		"SPP",
		"Byte",
		"Fast Centronics",
		"ECP",
		"EPP",
		"Reserved",
		"Test",
		"Configuration"
	};

#ifdef DJGPP
	printf("Scanning ports...\n");
	_dosmemgetb(0x411, 1, byte);
	lpt_num = (byte[0] & 0xc0) >> 6;
	// printf("Number of LPT ports found: %d\n", lpt_num);
	_dosmemgetb(0x408, lpt_num * 2, byte);

	for (i = 0; i < lpt_num; i++)
	{
		port = byte[2 * i] + byte[2 * i + 1] * 0x100;
		lpt[i] = port;
	}
#else
	lpt_num = 1;
	lpt[0] = 0x378;
#endif

	/* on ECP ports force BYTE mode */
	for (i = 0; i < lpt_num; i++)
	{
		port = lpt[i];
		ecr = inportb(port + 0x402);
		outportb(port + 0x402, 0x34);
		outportb(port + 2, 0xc6);
		if (inportb(port + 0x402) != 0x35)
			continue;	/* no ECP port */
		printf("ECP port at %04x in %s Mode\n", port,
		  ecpm[(ecr & 0xe0) >> 5]);
		// printf("Forcing Byte Mode\n");
		outportb(port + 0x402, (ecr & 0x1f) | 0x20);
	}

	for (cable = 0; cable < CABLE_TYPE_NUM; cable++)
	{
		switch (cable)
		{
		case CABLE_TYPE_XE1541:
			set_xe();
			break;
		case CABLE_TYPE_XA1541:
			set_xa();
			break;
		case CABLE_TYPE_XM1541:
			set_xm();
			break;
		}

		for (i = 0; i < lpt_num; i++)
		{
			serport = lpt[i];
			portval[i] = 0xc0;
			serportval = &portval[i];

			RELEASE(DATA_OUT | ATN_OUT | CLK_OUT);
			SET(RESET_OUT);
			delay(100);	/* 100ms */
			RELEASE(RESET_OUT);
		}
		delay(2000);	/* 2s */

		goodport = -1;
		for (i = 0; i < lpt_num; i++)
		{
			irq_count = 0;
			portval[i] = 0xc0;
			serportval = &portval[i];
			found = scan_xe1541(lpt[i]);
			if ((goodport == (-1)) && found)
				goodport = i;
			// printf("Port %d: %04x - %s\n", i, lpt[i],
			//  found ? "Found!" : "none");
		}

		if (goodport != -1)
		{
			printf("Using port %04x for X%c1541 connection.\n",
			  lpt[goodport], cableSpec[cable]);
			serport = lpt[goodport];
			parport = lpt[goodport];
			serportval = &portval[goodport];
			parportval = &portval[goodport];
			RELEASE(DATA_OUT | CLK_OUT);
			SET(CLK_OUT);
			return (1);
		}
	}

	printf("Drive not found!\n");
	return (0);
}
