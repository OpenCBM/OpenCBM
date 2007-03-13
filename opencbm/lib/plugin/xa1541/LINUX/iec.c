/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2005 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
*/

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: iec.c,v 1.1.2.1 2007-03-13 10:01:35 strik Exp $";
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "opencbm.h"
#include "cbm_module.h"

static char *cbm_dev_name = "/dev/cbm";

const char *cbmarch_get_driver_name(int port)
{
    return cbm_dev_name;
}

int cbmarch_driver_open(CBM_FILE *f, int port)
{
    *f = open(cbm_dev_name, O_RDWR);
    return (*f < 0) ? -1 : 0; /* FIXME */
}

void cbmarch_driver_close(CBM_FILE f)
{
    if(f >= 0) {
        close(f);
    }
}

/*! \brief Lock the parallel port for the driver

 This function locks the driver onto the parallel port. This way,
 no other program or driver can allocate the parallel port and
 interfere with the communication.

 \param HandleDevice  
   A CBM_FILE which contains the file handle of the driver.

 If cbm_driver_open() did not succeed, it is illegal to 
 call cbm_driver_close().

 \remark
 A call to cbm_lock() is undone with a call to cbm_unlock().

 Note that it is *not* necessary to call this function
 (or cbm_unlock()) when all communication is done with
 the handle to opencbm open (that is, between 
 cbm_driver_open() and cbm_driver_close(). You only
 need this function to pin the driver to the port even
 when cbm_driver_close() is to be executed (for example,
 because the program terminates).
*/

void
cbmarch_lock(CBM_FILE f)
{
}

/*! \brief Unlock the parallel port for the driver

 This function unlocks the driver from the parallel port.
 This way, other programs and drivers can allocate the
 parallel port and do their own communication with
 whatever device they use.

 \param HandleDevice  
   A CBM_FILE which contains the file handle of the driver.

 If cbm_driver_open() did not succeed, it is illegal to 
 call cbm_driver_close().

 \remark
 Look at cbm_lock() for an explanation of this function.
*/

void
cbmarch_unlock(CBM_FILE f)
{
}

int cbmarch_raw_write(CBM_FILE f, const void *buf, size_t size)
{
    return write(f, buf, size);
}

int cbmarch_raw_read(CBM_FILE f, void *buf, size_t size)
{
    return read(f, buf, size);
}

int cbmarch_listen(CBM_FILE f, __u_char dev, __u_char secadr)
{
    return ioctl(f, CBMCTRL_LISTEN, (dev<<8) | secadr);
}

int cbmarch_talk(CBM_FILE f, __u_char dev, __u_char secadr)
{
    return ioctl(f, CBMCTRL_TALK, (dev<<8) | secadr);
}

int cbmarch_open(CBM_FILE f, __u_char dev, __u_char secadr)
{
    int rv;

    rv = ioctl(f, CBMCTRL_OPEN, (dev<<8) | secadr);
    return rv;
}

int cbmarch_close(CBM_FILE f, __u_char dev, __u_char secadr)
{
    return ioctl(f, CBMCTRL_CLOSE, (dev<<8) | secadr);
}

int cbmarch_unlisten(CBM_FILE f)
{
    return ioctl(f, CBMCTRL_UNLISTEN);
}

int cbmarch_untalk(CBM_FILE f)
{
    return ioctl(f, CBMCTRL_UNTALK);
}

int cbmarch_get_eoi(CBM_FILE f)
{
    return ioctl(f, CBMCTRL_GET_EOI);
}

int cbmarch_clear_eoi(CBM_FILE f)
{
    return ioctl(f, CBMCTRL_CLEAR_EOI);
}

int cbmarch_reset(CBM_FILE f)
{
    return ioctl(f, CBMCTRL_RESET);
}

__u_char cbmarch_pp_read(CBM_FILE f)
{
    return ioctl(f, CBMCTRL_PP_READ);
}

void cbmarch_pp_write(CBM_FILE f, __u_char c)
{
    ioctl(f, CBMCTRL_PP_WRITE, c);
}

int cbmarch_iec_poll(CBM_FILE f)
{
    return ioctl(f, CBMCTRL_IEC_POLL);
}

int cbmarch_iec_get(CBM_FILE f, int line)
{
    return (ioctl(f, CBMCTRL_IEC_POLL) & line) != 0;
}

void cbmarch_iec_set(CBM_FILE f, int line)
{
    ioctl(f, CBMCTRL_IEC_SET, line);
}

void cbmarch_iec_release(CBM_FILE f, int line)
{
    ioctl(f, CBMCTRL_IEC_RELEASE, line);
}

int cbmarch_iec_wait(CBM_FILE f, int line, int state)
{
    return ioctl(f, CBMCTRL_IEC_WAIT, (line<<8) | state);
}

void cbmarch_iec_setrelease(CBM_FILE f, int set, int release)
{
    ioctl(f, CBMCTRL_IEC_SETRELEASE, (set<<8) | release);
}
