/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999 Michael Klein <michael.klein@puffin.lb.shuttle.de>
 */

#ifndef _CBM_H
#define _CBM_H

#define IEC_DATA   0x01
#define IEC_CLOCK  0x02
#define IEC_ATN    0x04

#define __u_char unsigned char


extern const char cbm_dev[];

extern int cbm_listen(int f, __u_char dev, __u_char secadr);
extern int cbm_talk(int f, __u_char dev, __u_char secadr);

extern int cbm_open(int f, __u_char dev, __u_char secadr);
extern int cbm_close(int f, __u_char dev, __u_char secadr);

extern int cbm_unlisten(int f);
extern int cbm_untalk(int f);

extern int cbm_reset(int f);

extern __u_char cbm_pp_read(int f);
extern void cbm_pp_write(int f, __u_char c);
extern __u_char cbm_mnib_par_read(int f);
extern void cbm_mnib_par_write(int f, __u_char c);

extern int cbm_iec_poll(int f);
extern int cbm_iec_get(int f, int line);
extern void cbm_iec_set(int f, int line);
extern void cbm_iec_release(int f, int line);

extern int cbm_upload(int f, __u_char dev, int adr, __u_char *prog,
  int size);

extern int cbm_device_status(int f, int drv, char *buf, int bufsize);
extern int cbm_exec_command(int f, int drv, char *cmd, int len);

#endif
