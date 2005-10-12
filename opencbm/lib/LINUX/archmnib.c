/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 2005      Tim Schürmann
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: archmnib.c,v 1.1.2.2 2005-10-12 15:10:00 tischuer Exp $";
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "opencbm.h"
#include "cbm_module.h"

#include <linux/errno.h>  /* error codes */


/*linux functions needed by mnib */

__u_char cbmarch_mnib_par_read(CBM_FILE f)
{
	return ioctl(f, CBMCTRL_MNIB_PAR_READ);
}

void cbmarch_mnib_par_write(CBM_FILE f, __u_char c)
{
	ioctl(f, CBMCTRL_MNIB_PAR_WRITE, c);
}

int cbmarch_mnib_read_track(CBM_FILE f, __u_char *buffer, unsigned int length, __u_char mode)
{
	int retval;

	MNIB_RW_VALUE mv;
	mv.buffer=buffer;
	mv.length=length; /* only needed in write_track */
	mv.mode=mode;
	retval=ioctl(f, CBMCTRL_MNIB_READ_TRACK, &mv);  /* we have to catch retval to check on EFAULT */
	if(retval==(-EFAULT)) {printf("cbm4linux: cbm.c: cbm_mnib_read_track: ioctl returned -EFAULT"); return 0;}
	return retval;
}

int cbmarch_mnib_write_track(CBM_FILE f,  __u_char *buffer, unsigned int length, __u_char mode)
{
	int retval;
	
	MNIB_RW_VALUE mv;
	mv.buffer=buffer;
	mv.length=length;
	mv.mode=mode;
	retval=ioctl(f, CBMCTRL_MNIB_WRITE_TRACK, &mv);
	if(retval==(-EFAULT)) {printf("cbm4linux: cbm.c: cbm_mnib_write_track: ioctl returned -EFAULT"); return 0;}
	return retval;
}
