/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cbm.h"
#include "kernel.h"

const char cbm_dev[] = "/dev/cbm";

int
cbm_listen(int f, __u_char dev, __u_char secadr)
{
	return cbm_ioctl(f, CBMCTRL_LISTEN, (dev << 8) | secadr);
}

int
cbm_talk(int f, __u_char dev, __u_char secadr)
{
	return cbm_ioctl(f, CBMCTRL_TALK, (dev << 8) | secadr);
}

int
cbm_open(int f, __u_char dev, __u_char secadr)
{
	return cbm_ioctl(f, CBMCTRL_OPEN, (dev << 8) | secadr);
}

int
cbm_close(int f, __u_char dev, __u_char secadr)
{
	return cbm_ioctl(f, CBMCTRL_CLOSE, (dev << 8) | secadr);
}

int
cbm_unlisten(int f)
{
	return cbm_ioctl(f, CBMCTRL_UNLISTEN, 0);
}

int
cbm_untalk(int f)
{
	return cbm_ioctl(f, CBMCTRL_UNTALK, 0);
}

int
cbm_reset(int f)
{
	return cbm_ioctl(f, CBMCTRL_RESET, 0);
}

__u_char
cbm_pp_read(int f)
{
	return cbm_ioctl(f, CBMCTRL_PP_READ, 0);
}

void
cbm_pp_write(int f, __u_char c)
{
	cbm_ioctl(f, CBMCTRL_PP_WRITE, c);
}

__u_char
cbm_mnib_par_read(int f)
{
	return cbm_ioctl(f, CBMCTRL_PAR_READ, 0);
}

void
cbm_mnib_par_write(int f, __u_char c)
{
	cbm_ioctl(f, CBMCTRL_PAR_WRITE, c);
}

int
cbm_iec_poll(int f)
{
	return cbm_ioctl(f, CBMCTRL_IEC_POLL, 0);
}

int
cbm_iec_get(int f, int line)
{
	return (cbm_ioctl(f, CBMCTRL_IEC_POLL, 0) & line) != 0;
}

void
cbm_iec_set(int f, int line)
{
	cbm_ioctl(f, CBMCTRL_IEC_SET, line);
}

void
cbm_iec_release(int f, int line)
{
	cbm_ioctl(f, CBMCTRL_IEC_RELEASE, line);
}

int
cbm_device_status(int f, int drv, char *buf, int bufsize)
{
	strncpy(buf, "99, DRIVER ERROR,00,00\r", bufsize);
	if (cbm_talk(f, drv, 15) == 0)
	{
		int bytes_read = cbm_raw_read(f, buf, bufsize);
		if (bytes_read == bufsize)
		{
			bytes_read--;
		}
		buf[bytes_read] = '\0';
		cbm_untalk(f);
	}
	return atoi(buf);
}

int
cbm_exec_command(int f, int drv, char *cmd, int len)
{
	int rv;
	rv = cbm_listen(f, drv, 15);
	if (rv == 0)
	{
		if (len == 0)
		{
			len = strlen(cmd);
		}
		rv = cbm_raw_write(f, cmd, len) != len;
		cbm_unlisten(f);
	}
	return rv;
}

int
cbm_upload(int f, __u_char dev, int adr, __u_char *prog, int size)
{
	int c, i, ret, rv;
	char cmd[40];

	rv = 0;
	for (i = 0; i < size; i += 32)
	{
		cbm_listen(f, dev, 15);
		c = size - i;
		if (c > 32)
			c = 32;
		sprintf(cmd, "M-W%c%c%c", adr % 256, adr / 256, c);
		adr += c;
		ret = cbm_raw_write(f, cmd, 6);
		if (ret < 0)
			return (ret);
		ret = cbm_raw_write(f, (char *)prog, c);
		if (ret < 0)
			return (ret);
		prog += c;
		rv += c;
		cbm_unlisten(f);
	}
	return rv;
}
