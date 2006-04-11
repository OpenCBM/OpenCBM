/*
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 *  Copyright 2001 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 */

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: cbmcopy.c,v 1.11 2006-04-11 20:24:35 wmsr Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "opencbm.h"
#include "cbmcopy.h"

#include "cbmcopy_int.h"

#include "arch.h"

static const unsigned char turboread1541[] = {
#include "turboread1541.inc"
};

static const unsigned char turboread1571[] = {
#include "turboread1571.inc"
};

static const unsigned char turboread1581[] = {
#include "turboread1581.inc"
};

static const unsigned char turbowrite1541[] = {
#include "turbowrite1541.inc"
};

static const unsigned char turbowrite1571[] = {
#include "turbowrite1571.inc"
};

static const unsigned char turbowrite1581[] = {
#include "turbowrite1581.inc"
};

extern transfer_funcs cbmcopy_s1_transfer,
                      cbmcopy_s2_transfer,
                      cbmcopy_pp_transfer;

static struct _transfers
{
    const transfer_funcs *trf;
    const char *name, *abbrev;
}
transfers[] =
{
    { &cbmcopy_s1_transfer, "auto", "a%1" },
    { &cbmcopy_s1_transfer, "serial1", "s1" },
    { &cbmcopy_s2_transfer, "serial2", "s2" },
    { &cbmcopy_pp_transfer, "parallel", "p%" },
    { NULL, NULL, NULL }
};

#ifdef CBMCOPY_DEBUG
signed int debugTransferMode=0, debugBlockCount=0, debugByteCount=0;

void printDebugCounters(cbmcopy_message_cb msg_cb)
{
    msg_cb( sev_info, "transferMode=%d, blockCount=%d, byteCount=%d\n",
                      debugTransferMode, debugBlockCount, debugByteCount);
}
#endif

static int check_drive_type(CBM_FILE fd, unsigned char drive,
                            cbmcopy_settings *settings,
                            cbmcopy_message_cb msg_cb)
{
    const char *type_str;

    if( settings->drive_type == cbm_dt_unknown )
    {
        if(cbm_identify( fd, drive, &settings->drive_type, &type_str ))
        {
            msg_cb( sev_warning, "could not identify drive, using 1541 turbo" );
            settings->drive_type = cbm_dt_cbm1541;
        }
        else
        {
            msg_cb( sev_info, "identified a %s drive", type_str );
        }
    }
    return 0;
}


static int send_turbo(CBM_FILE fd, unsigned char drive, int write,
                      const cbmcopy_settings *settings,
                      const unsigned char *turbo, size_t turbo_size,
                      const unsigned char *start_cmd, size_t cmd_len,
                      cbmcopy_message_cb msg_cb)
{
    const transfer_funcs *trf;

    trf = transfers[settings->transfer_mode].trf;
    cbm_upload( fd, drive, 0x500, turbo, turbo_size );
    msg_cb( sev_debug, "uploading %d bytes turbo code", turbo_size );
    if(trf->upload_turbo(fd, drive, settings->drive_type, write) == 0)
    {
        cbm_exec_command( fd, drive, start_cmd, cmd_len );
        msg_cb( sev_debug, "initializing transfer code" );
        if(trf->start_turbo(fd, write) == 0)
        {
            msg_cb( sev_debug, "done" );
            return 0;
        }
        else
        {
            msg_cb( sev_fatal, "could not start turbo" );
        }
    }
    else
    {
        msg_cb( sev_fatal, "Unsupported transfer mode for this device" );
    }
    return -1;
}


static int cbmcopy_read(CBM_FILE fd,
                        cbmcopy_settings *settings,
                        unsigned char drive,
                        int track, int sector,
                        const char *cbmname,
                        int cbmname_len,
                        unsigned char **filedata,
                        size_t *filedata_size,
                        cbmcopy_message_cb msg_cb,
                        cbmcopy_status_cb status_cb)
{
    int rv;
    int i;
    int turbo_size;
    int error;
    unsigned char c;
    unsigned char *cptr;
    unsigned char buf[48];
    const unsigned char *turbo;
    const transfer_funcs *trf;
    int blocks_read;

    *filedata = NULL;
    *filedata_size = 0;

    msg_cb( sev_debug, "using transfer mode `%s´",
            transfers[settings->transfer_mode].name);
    trf = transfers[settings->transfer_mode].trf;

    if(check_drive_type( fd, drive, settings, msg_cb ))
    {
        return -1;
    }

    switch(settings->drive_type)
    {
        case cbm_dt_cbm1541:
            turbo = turboread1541;
            turbo_size = sizeof(turboread1541);
            break;
        case cbm_dt_cbm1570:
        case cbm_dt_cbm1571:
            cbm_exec_command( fd, drive, "U0>M1", 0 );
            turbo = turboread1571;
            turbo_size = sizeof(turboread1571);
            break;
        case cbm_dt_cbm1581:
            turbo = turboread1581;
            turbo_size = sizeof(turboread1581);
            break;
        default: /* unreachable */
            return -1;
    }

    if(cbmname)
    {
        /* start by file name */
        track = 0;
        sector = 0;
        cbm_open( fd, drive, 0, NULL, 0 );
        if(cbmname_len == 0) cbmname_len = strlen( cbmname );
        cbm_raw_write( fd, cbmname, cbmname_len );
        cbm_unlisten( fd );
    }
    else
    {
        /* start by track/sector */
        cbm_open( fd, drive, 0, "#", 1 );
    }
    rv = cbm_device_status( fd, drive, (char*)buf, sizeof(buf) );

    if(rv)
    {
        msg_cb( sev_fatal, "could not open file for reading: %s", buf );
        cbm_driver_close(fd);
        return rv;
    }

    blocks_read = 0;
    error = 0;

    if(track)
    {
        msg_cb( sev_debug, "start read at %d/%d", track, sector );
    }
    sprintf( (char*)buf, "U4:%c%c", (unsigned char)track, (unsigned char)sector );

    if(send_turbo(fd, drive, 0, settings,
                  turbo, turbo_size, buf, 5, msg_cb) == 0)
    {
        msg_cb( sev_debug, "start of copy" );
        status_cb( blocks_read );

#ifdef CBMCOPY_DEBUG
        debugTransferMode=1;    // read mode
        debugBlockCount=0;
        debugByteCount=-99;    // not actually in use
#endif

        for(c = 0xff;
            c == 0xff && (error = trf->check_error(fd, 0)) == 0;
            /* nothing */ )
        {
                // Hangup position following? Yes, seems so.
                //
                //  debugByteCount ==
                //      -99  -- if coming from loop entry
                //      -10  -- if coming from loop repetation
                //
                //
                // Fix proposion: add a little delay at the end of the loop

            c = trf->read_byte( fd );
#ifdef CBMCOPY_DEBUG
            debugByteCount=-90;    // afterwait condition
#endif 
            i = (c == 0xff) ? 0xfe : c;
            *filedata_size += i;

#ifdef CBMCOPY_DEBUG
            debugBlockCount++;
            debugByteCount=-80;    // preset condition
#endif 

            /* @SRT: FIXME! the next statement is dangerous: If there 
             * is no memory block large enough for reallocating, the
             * memory block is not freed, but realloc() returns NULL,
             * thus, we have a memory leak.
             */
            *filedata = realloc(*filedata, *filedata_size);
            if(*filedata)
            {
#ifdef CBMCOPY_DEBUG
                msg_cb( sev_debug, "receive block data (%d)", c );
                debugByteCount=0;
#endif 
                for(cptr = (*filedata) + blocks_read * 254; i; i--)
                {
#ifdef CBMCOPY_DEBUG
                    debugByteCount++;
#endif
                    *(cptr++) = trf->read_byte( fd );
                }
                /* (drive is busy now) */

                // Fix proposion: add a little delay at the end of the loop
                //    "hmmmm, if we know that the drive is busy now,
                //     shouldn't we wait for it then?"
                {
#if CBMCOPY_DEBUG+0>=3
                    int s1=0, s2=0;
                    s1=cbm_iec_poll(fd);
#endif
                    // arch_usleep(1000);
                    // arch_usleep(50000);
                    
                        // even a simple task schedule seems to be enough
                        // to fix for the hangup issues
                    // arch_usleep(0);
                        // no, not at Spiro's setup

                    arch_usleep(1000);
#if CBMCOPY_DEBUG+0>=3
                    s2=cbm_iec_poll(fd);

                    if(s1!=s2)
                    {
                        msg_cb( sev_warning, "pre- and postdelay IEC bus conditions differ: 0x%02X!=0x%02X", s1, s2);
                        /*
                         * [Warning] pre- and postdelay IEC bus conditions differ: 0x0B!=0x0A
                         * [Warning] pre- and postdelay IEC bus conditions differ: 0x0B!=0x0A
                         * [Warning] pre- and postdelay IEC bus conditions differ: 0x0B!=0x0A
                         * [Warning] pre- and postdelay IEC bus conditions differ: 0x0B!=0x0A
                         * ...
                         * This really happens very often !!!
                         *
                         * Result: Do a wait for DATA becoming 0 (active)
                         * ATTENTION: This may be protocol dependent and may not work!
                         */
                    }
                    else if(s2!=0x0b)
                    {
                        msg_cb( sev_warning, "postdelay IEC bus condition !=0x0B: 0x%02X", s2);
                        /*
                         * 0x0B seems to be the standard value after the last transferred
                         * byte, hmmm....
                         *
                         * Why is the bus state sometimes 0x0A after the wait then?
                         *     And why doesn't this produce hangups then?
                         * Would we see that 0x0A value more often, if the delay
                         * becomes much bigger, e.g.: 50000 ?
                         *
                         * OK, after such a long delay of 50000 the pre- and postdelay
                         * bus conditions almost always differ and the final IEC bus
                         * state becomes 0x0A.
                         *
                         *
                         * Since waiting for a dedicated line state seems to be not
                         * applicable in respect to the different protocols, what's
                         * the effect, if we do only a 0ms-sleep (effectively giving
                         * up one time slice)?
                         * This also seems to fix the hangups. Even more, pre-/
                         * postdelay differencies can be watched very, very rarely.
                         *
                         */
                    }
#endif
                }
                // Is there another possibility to wait for a decent
                // IEC bus state instead of sillily waiting a dedicated
                // amount of time and perhaps too less time?

#if CBMCOPY_DEBUG+0>=5
                msg_cb( sev_info, "After block byteCount=%u", debugByteCount );
#endif
                
#ifdef CBMCOPY_DEBUG
                debugByteCount=-1;    // afterread condition
#endif 
                status_cb( ++blocks_read );
            }
            else
            {
                /* FIXME */
            }

#ifdef CBMCOPY_DEBUG
            debugByteCount=-10;    // pre loop condition
#endif 
        }
        msg_cb( sev_debug, "done" );
#ifdef CBMCOPY_DEBUG
        debugByteCount=-200;    // end loop condition
#endif 
        trf->exit_turbo( fd, 0 );
#ifdef CBMCOPY_DEBUG
        debugByteCount=-300;    // turbo exited condition
#endif 
    }

    return rv;
}


char *cbmcopy_get_transfer_modes()
{
    const struct _transfers *t;
    int size;
    char *buf;
    char *dst;

    size = 1; /* for terminating '\0' */
    for(t = transfers; t->name; t++)
    {
        size += (strlen(t->name) + 1);
    }

    buf = malloc(size);

    if(buf)
    {
        dst = buf;
        for(t = transfers; t->name; t++)
        {
            strcpy(dst, t->name);
            dst += (strlen(t->name) + 1);
        }
        *dst = '\0';
    }

    return buf;
}


int cbmcopy_get_transfer_mode_index(const char *name)
{
    const struct _transfers *t;
    int i;
    int abbrev_len;
    int tm_len;

    if(NULL == name)
    {
        /* default transfer mode */
        return 0;
    }

    tm_len = strlen(name);
    for(i = 0, t = transfers; t->name; i++, t++)
    {
        if(strcmp(name, t->name) == 0)
        {
            /* full match */
            return i;
        }
        if(t->abbrev[strlen(t->abbrev)-1] == '%')
        {
            abbrev_len = strlen(t->abbrev) - 1;
            if(abbrev_len <= tm_len && strncmp(t->name, name, tm_len) == 0)
            {
                return i;
            }
        }
        else
        {
            if(strcmp(name, t->abbrev) == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

int cbmcopy_check_auto_transfer_mode(CBM_FILE cbm_fd, int auto_transfermode, int drive)
{
    int transfermode = auto_transfermode;

    /* We assume auto is the first transfer mode */
    assert(strcmp(transfers[0].name, "auto") == 0);

    if (auto_transfermode == 0)
    {
        do {
            enum cbm_cable_type_e cable_type;
            unsigned char testdrive;

            /*
             * Test the cable
             */

            if (cbm_identify_xp1541(cbm_fd, (unsigned char)drive, NULL, &cable_type) == 0)
            {
                if (cable_type == cbm_ct_xp1541)
                {
                    /*
                     * We have a parallel cable, use that
                     */
                    transfermode = cbmcopy_get_transfer_mode_index("parallel");
                    break;
                }
            }

            /*
             * We do not have a parallel cable. Check if we are the only drive
             * on the bus, so we can use serial2, at least.
             */

            for (testdrive = 4; testdrive < 31; ++testdrive)
            {
                enum cbm_device_type_e device_type;

                /* of course, the drive to be transfered to is present! */
                if (testdrive == drive)
                    continue;

                if (cbm_identify(cbm_fd, testdrive, &device_type, NULL) == 0)
                {
                    /*
                     * My bad, there is another drive -> only use serial1
                     */
                    transfermode = cbmcopy_get_transfer_mode_index("serial1");
                    break;
                }
            }

            /*
             * If we reached here with transfermode 0, we are the only
             * drive, thus, use serial2.
             */
            if (transfermode == 0)
                transfermode = cbmcopy_get_transfer_mode_index("serial2");

        } while (0);
    }

    return transfermode;
}

cbmcopy_settings *cbmcopy_get_default_settings(void)
{
    cbmcopy_settings *settings;

    settings = malloc(sizeof(cbmcopy_settings));

    if(NULL != settings)
    {
        settings->drive_type    = cbm_dt_unknown; /* auto detect later on */
        settings->transfer_mode = 0;
    }
    return settings;
}



int cbmcopy_write_file(CBM_FILE fd,
                       cbmcopy_settings *settings,
                       int drivei,
                       const char *cbmname,
                       int cbmname_len,
                       const unsigned char *filedata,
                       int filedata_size,
                       cbmcopy_message_cb msg_cb,
                       cbmcopy_status_cb status_cb)
{
    int rv;
    int i;
    int turbo_size;
    unsigned char drive = (unsigned char) drivei; //! \todo Find better solution
    int error;
    unsigned char c;
    unsigned char buf[48];
    const unsigned char *turbo;
    const transfer_funcs *trf;
    int blocks_written;

    msg_cb( sev_debug, "using transfer mode `%s´",
            transfers[settings->transfer_mode].name);
    trf = transfers[settings->transfer_mode].trf;

    if(check_drive_type( fd, drive, settings, msg_cb ))
    {
        return -1;
    }

    switch(settings->drive_type)
    {
        case cbm_dt_cbm1541:
            turbo = turbowrite1541;
            turbo_size = sizeof(turbowrite1541);
            break;
        case cbm_dt_cbm1570:
        case cbm_dt_cbm1571:
            cbm_exec_command( fd, drive, "U0>M1", 0 );
            turbo = turbowrite1571;
            turbo_size = sizeof(turbowrite1571);
            break;
        case cbm_dt_cbm1581:
            turbo = turbowrite1581;
            turbo_size = sizeof(turbowrite1581);
            break;
        default: /* unreachable */
            return -1;
    }

    cbm_open( fd, drive, 1, NULL, 0 );
    if(cbmname_len == 0) cbmname_len = strlen( cbmname );
    cbm_raw_write( fd, cbmname, cbmname_len );
    cbm_unlisten( fd );
    rv = cbm_device_status( fd, drive, buf, sizeof(buf) );

    if(rv)
    {
        msg_cb( sev_fatal, "could not open file for writing: %s", buf );
        cbm_driver_close(fd);
        return rv;
    }

    blocks_written = 0;
    error = 0;

    if(send_turbo(fd, drive, 1, settings,
                  turbo, turbo_size, (unsigned char*)"U4:", 3, msg_cb) == 0)
    {
        msg_cb( sev_debug, "start of copy" );
        status_cb( blocks_written );

#ifdef CBMCOPY_DEBUG
        debugTransferMode=2;    // write mode
        debugBlockCount=0;
        debugByteCount=-99;    // not actually in use
#endif

        for(i = 0;
            (i == 0) || (i < filedata_size && !error );
            i+=254)
        {
            if( filedata_size - i <= 254 )
            {
                c = filedata_size - i;
            }
            else
            {
                c = 255;
            }
#ifdef CBMCOPY_DEBUG
            msg_cb( sev_debug, "send byte count: %d", c );
            debugBlockCount++;
#endif
                // Hangup position following?
                //
                //  debugByteCount ==
                //      -99  -- if coming from loop entry
                //      -10  -- if coming from loop repetation
                //
                //
                // Fix proposion: add a little delay at the end of the loop
/*
SIGINT caught X-(  Resetting IEC bus...
[Info] transferMode=2, blockCount=502, byteCount=-10                

*/
            trf->write_byte( fd, c );
#ifdef CBMCOPY_DEBUG
            debugByteCount=-90;    // afterwait condition
#endif 

            if(c)
            {
#ifdef CBMCOPY_DEBUG
                msg_cb( sev_debug, "send block data" );
                debugByteCount=0;
#endif 
                if( c == 0xff ) c = 0xfe;
                while(c)
                {
#ifdef CBMCOPY_DEBUG
                    debugByteCount++;
#endif 
                    trf->write_byte( fd, *(filedata++) );
                    c--;
                }

                /* (drive is busy now) */

#if CBMCOPY_DEBUG+0>=5
                msg_cb( sev_info, "After block byteCount=%u", debugByteCount );
#endif
                
#ifdef CBMCOPY_DEBUG
                debugByteCount=-1;    // afterread condition
#endif 

            }
            error = trf->check_error( fd, 1 );
#ifdef CBMCOPY_DEBUG
            debugByteCount=-4;    // aftercheck condition
#endif 

            // Fix proposion: add a little delay at the end of the loop
            //    "hmmmm, if we know that the drive is busy now,
            //     shouldn't we wait for it then?"
            arch_usleep(1000);
            // Is there another possibility to wait for a decent
            // IEC bus state instead of sillily waiting a dedicated
            // amount of time and perhaps too less time?

            if(!error)
            {
                status_cb( ++blocks_written );
            }
#ifdef CBMCOPY_DEBUG
            debugByteCount=-10;    // pre loop condition
#endif 
        }
        msg_cb( sev_debug, "done" );

#ifdef CBMCOPY_DEBUG
        debugByteCount=-200;    // end loop condition
#endif 
        trf->exit_turbo( fd, 0 );
    }
    return rv;
}


/* just a wrapper */
int cbmcopy_read_file_ts(CBM_FILE fd,
                         cbmcopy_settings *settings,
                         int drive,
                         int track, int sector,
                         unsigned char **filedata,
                         size_t *filedata_size,
                         cbmcopy_message_cb msg_cb,
                         cbmcopy_status_cb status_cb)
{
    return cbmcopy_read(fd, settings, (unsigned char) drive,
                        track, sector,
                        NULL, 0,
                        filedata, filedata_size,
                        msg_cb, status_cb);
}


/* just a wrapper */
int cbmcopy_read_file(CBM_FILE fd,
                      cbmcopy_settings *settings,
                      int drive,
                      const char *cbmname,
                      int cbmname_len,
                      unsigned char **filedata,
                      size_t *filedata_size,
                      cbmcopy_message_cb msg_cb,
                      cbmcopy_status_cb status_cb)
{
    return cbmcopy_read(fd, settings, (unsigned char) drive,
                        0, 0,
                        cbmname, cbmname_len,
                        filedata, filedata_size,
                        msg_cb, status_cb);
}
