// This demo file only works with the memory models:
// TINY, SMALL.
// Other's may work, but are not tested.
// Essentially, this program assumes that all data is accessed
// via DS, and DS and ES are always the same.


/*
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis
*/

/*! ************************************************************** 
** \file sample/dos/c/opencbn.c \n
** \author Spiro Trikaliotis \n
** \version $Id: OPENCBM.C,v 1.1 2004-12-22 14:57:04 strik Exp $ \n
** \n
** \brief Library for accessing the driver from DOS
**
****************************************************************/

#include "opencbm.h"

#include <stdio.h>
#include <stdlib.h>

typedef unsigned int WORD;


#define RegisterModule   db 0xc4, 0xc4, 0x58, 0x00
#define UnRegisterModule db 0xc4, 0xc4, 0x58, 0x01
#define DispatchCall     db 0xc4, 0xc4, 0x58, 0x02

#define CbmDispatchCallNoBx(_no_) \
     mov dl,_no_; mov ax,[VDD_HANDLE]; DispatchCall

/*
#define CbmDispatchCall(_no_) \
     mov bx,[f]; mov dl,_no_; mov ax,[VDD_HANDLE]; DispatchCall
*/
#define CbmDispatchCall(_no_) \
     mov bx,[f]; CbmDispatchCallNoBx(_no_)

#define CbmDispatchCallRetVal(_no_) \
        CbmDispatchCall(_no_); mov [retVal],ax


static WORD VDD_HANDLE = -1;

static int vdd_initialized = 0;

static int
vdd_init(void)
{
    static const char DllName[] = "OpencbmVDD.DLL";
    static const char InitFunc[] = "VDDRegisterInit";
    static const char DispFunc[] = "VDDDispatch";

    WORD retVal;
    WORD error;

    if (vdd_initialized)
    {
        return 0;
    }

    asm {
        push es

        push ds
        pop es

        lea si,[word ptr DllName] /* ds:si */
        lea di,[InitFunc]  /* es:di */
        lea bx,[DispFunc]  /* ds:bx */

        RegisterModule

        mov [VDD_HANDLE],ax

        /* make sure we notice of an error occurred */
        xor ax,ax
        adc ax,0
        mov [error],ax

        pop es
    }

    if (error)
    {
        switch (VDD_HANDLE)
        {
        case 1: printf("vdd_init(): DLL not found!\n"); break;
        case 2: printf("vdd_init(): Dispatch routine not found!\n"); break;
        case 3: printf("vdd_init(): Init routine not found!\n"); break;
        case 4: printf("vdd_init(): Insufficient memory!\n"); break;
        default: printf("vdd_init(): unknown error reason: %u\n", retVal); break;
        }

        VDD_HANDLE = -1;
    }
    else
    {
        vdd_initialized = 1;
    }

    return error ? 1 : 0;
}

static void
vdd_uninit(void)
{
    if (vdd_initialized)
    {
        vdd_initialized = 0;

        asm {
            mov ax,[VDD_HANDLE]
            UnRegisterModule
        }

        VDD_HANDLE = -1;
    }
}

int
cbm_driver_open(CBM_FILE *f, int port)
{
    CBM_FILE cbmfile;
    WORD retax;

    vdd_init();
    atexit(vdd_uninit);

    asm {
        mov cx,[port]
        CbmDispatchCallNoBx(0)
        mov [cbmfile],BX
        mov [retax],AX
    }

    *f = cbmfile;

    return retax;
}


void
cbm_driver_close(CBM_FILE f)
{
    asm {
        CbmDispatchCall(1)
    }
}


// const char * cbm_get_driver_name(int port);


int
cbm_listen(CBM_FILE f, __u_char dev, __u_char secadr)
{
    WORD retVal;

    asm {
        mov dh,[dev]
        mov cl,[secadr]
        CbmDispatchCallRetVal(2)
    }
    return retVal;
}

int
cbm_talk(CBM_FILE f, __u_char dev, __u_char secadr)
{
    WORD retVal;

    asm {
        mov dh,[dev]
        mov cl,[secadr]
        CbmDispatchCallRetVal(3)
    }
    return retVal;
}

int
cbm_open(CBM_FILE f, __u_char dev, __u_char secadr, const void *fname, size_t len)
{
    WORD retVal;

    asm {
        mov dh,[dev]
        mov cl,[secadr]
        mov si,[fname]
        mov di,[len]
        CbmDispatchCallRetVal(4)
    }
    return retVal;
}

int
cbm_close(CBM_FILE f, __u_char dev, __u_char secadr)
{
    WORD retVal;

    asm {
        mov dh,[dev]
        mov cl,[secadr]
        CbmDispatchCallRetVal(5)
    }
    return retVal;
}


int
cbm_raw_read(CBM_FILE f, void *buf, size_t size)
{
    WORD retVal;

    asm {
        mov di,[buf]
        mov cx,[size]
        CbmDispatchCallRetVal(6)
    }
    return retVal;
}


int
cbm_raw_write(CBM_FILE f, const void *buf, size_t size)
{
    WORD retVal;

    asm {
        mov di,[buf]
        mov cx,[size]
        CbmDispatchCallRetVal(7)
    }
    return retVal;
}


int
cbm_unlisten(CBM_FILE f)
{
    WORD retVal;

    asm {
        CbmDispatchCallRetVal(8)
    }
    return retVal;
}

int
cbm_untalk(CBM_FILE f)
{
    WORD retVal;

    asm {
        CbmDispatchCallRetVal(9)
    }
    return retVal;
}

int
cbm_get_eoi(CBM_FILE f)
{
    WORD retVal;

    asm {
        CbmDispatchCallRetVal(10)
    }
    return retVal;
}

// @@@untested
int
cbm_clear_eoi(CBM_FILE f)
{
    WORD retVal;

    asm {
        CbmDispatchCallRetVal(11)
    }
    return retVal;
}


int
cbm_reset(CBM_FILE f)
{
    WORD retVal;

    asm {
        CbmDispatchCallRetVal(12)
    }
    return retVal;
}


// @@@untested
__u_char
cbm_pp_read(CBM_FILE f)
{
    WORD retVal;

    asm {
        CbmDispatchCallRetVal(13)
    }
    return retVal;
}

// @@@untested
void
cbm_pp_write(CBM_FILE f, __u_char c)
{
    asm {
        mov dh,[c]
        CbmDispatchCall(14)
    }
}

// @@@untested
int
cbm_iec_poll(CBM_FILE f)
{
    WORD retVal;

    asm {
        CbmDispatchCallRetVal(15)
    }
    return retVal;
}

// @@@untested
int
cbm_iec_get(CBM_FILE f, int line)
{
    WORD retVal;

    asm {
        mov dh,byte ptr [line]
        CbmDispatchCallRetVal(16)
    }
    return retVal;
}

// @@@untested
void
cbm_iec_set(CBM_FILE f, int line)
{
    asm {
        mov dh,byte ptr [line]
        CbmDispatchCall(17)
    }
}

// @@@untested
void
cbm_iec_release(CBM_FILE f, int line)
{
    asm {
        mov dh,byte ptr [line]
        CbmDispatchCall(18)
    }
}

// @@@untested
int
cbm_iec_wait(CBM_FILE f, int line, int state)
{
    WORD retVal;

    asm {
        mov dh,byte ptr [line]
        mov cl,byte ptr [state]
        CbmDispatchCallRetVal(19)
    }
    return retVal;
}


int
cbm_upload(CBM_FILE f, __u_char dev, int adr, const void *prog, size_t size)
{
    WORD retVal;

    asm {
        mov dh,[dev]
        mov di,[adr]
        mov si,[prog]
        mov cx,[size]
        CbmDispatchCallRetVal(20)
    }
    return retVal;
}


int
cbm_device_status(CBM_FILE f, __u_char dev, void *buf, size_t bufsize)
{
    WORD retVal;

    asm {
        mov dh,[dev]
        mov di,[buf]
        mov cx,[bufsize]
        CbmDispatchCallRetVal(21)
    }
    return retVal;
}

int
cbm_exec_command(CBM_FILE f, __u_char dev, const void *cmd, size_t len)
{
    WORD retVal;

    asm {
        mov dh,[dev]
        mov si,[cmd]
        mov cx,[len]
        CbmDispatchCallRetVal(22)
    }
    return retVal;
}


int
cbm_identify(CBM_FILE f, __u_char drv,
                                   enum cbm_device_type_e *t,
                                   const char **type_str)
{
    WORD retVal;
    static char local_type_str[80];
    enum cbm_device_type_e local_devicetype;

    asm {
        mov dh,[drv]
        mov si,offset [local_type_str]
        mov cx,80
        CbmDispatchCallRetVal(23)
        mov [local_devicetype],di
    }
    if (t)
    {
        *t = local_devicetype;
    }
    if (type_str)
    {
        *type_str = local_type_str;
    }
    return retVal;
}
