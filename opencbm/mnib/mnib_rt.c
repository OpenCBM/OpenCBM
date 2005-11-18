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
#include "mnib_rt.h"
#endif // DJGPP

int
cbm_mnib_write_track(int fd, unsigned char * buffer, int length, int mode)
{
	int i;

	disable();

	for (i = 0; i < length; i++)
	{
		if (!cbm_nib_write(buffer[i], i & 1))
		{
			// timeout
			enable();
			return 0;
		}
	}

	cbm_nib_write(0, i & 1);
	cbm_mnib_par_read(fd);

	enable();

	return (1);
}

int
cbm_nib_write(char data, int toggle)
{
	int to = 0;

	RELEASE(CLK_IN);

	if (!toggle)
	{
		while (GET(DATA_IN))
			if (to++ > 1000000)
				return (0);
	}
	else
	{
		while (!GET(DATA_IN))
			if (to++ > 1000000)
				return (0);
	}
	outportb(parport, data);

	return (1);
}

int
cbm_mnib_read_track(int fd, unsigned char * buffer, unsigned int length, int mode)
{
	int i, byte;

	disable();

	for (i = 0; i < 0x2000; i++)
	{
		byte = cbm_nib_read(i & 1);

		if (byte < 0)
		{
			// timeout
			enable();
			return 0;
		}

		buffer[i] = byte;
	}

	cbm_mnib_par_read(fd);

	enable();
	return (1);
}

int
cbm_nib_read(int toggle)
{
	int to = 0;

	RELEASE(DATA_IN);	// not really needed?

	if (!toggle)
	{
		while (GET(DATA_IN))
			if (to++ > 1000000)
				return (-1);
	}
	else
	{
		while (!GET(DATA_IN))
			if (to++ > 1000000)
				return (-1);
	}

	return (inportb(parport));
}
