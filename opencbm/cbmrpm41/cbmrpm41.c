/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright (C) 2006 Wolfgang Moser (http://d81.de)
 *  Copyright (C) 1999 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 */

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: cbmrpm41.c,v 1.7 2006-06-02 22:51:55 wmsr Exp $";
#endif

#include "cbmrpm41.h"

static unsigned char cbmrpm41[] = {
#include "cbmrpm41.inc"
};

#define _ASCII_PARAMETER_PASSING 1
#define _MINMAX_VALUES_PRINTOUT  0

static CBM_FILE fd;
static unsigned char drive;

const static unsigned short timerShotMain = sizeof(cbmDev_StartAddress) + offsetof(struct ExecBuffer_MemoryLayout, Timer24BitGroup);
const static unsigned short UcmdTblAddr   = sizeof(cbmDev_StartAddress) + offsetof(struct ExecBuffer_MemoryLayout, CommandVectorsTable_impl);

   /*
    * The virtual 23.589 bit timer value is reconstructed from
    * a 16 bit timer and an 8 bit timer with a modulus of 187
    * with the help of the Chinese Remainder Theorem. All the
    * constants below are precalculated coefficients for a
    * modulus of 65536 for one timer and a modulus of 187 for
    * the other timer.
    */
#define Via1Timer2Max (256 * 256)           // 16 bit timer
#define Via2Timer2Max (185 + 2)             // == 187, with latch value 185

const static int Modulus  = Via1Timer2Max * Via2Timer2Max;
const static int V1T2rec1 =    27081;       // --16-Bit timer (inverse of a mod b)
const static int V1T2rec2 =      121;       // -´
const static int V2T2rec  =  8978432;       // 7.89-Bit timer (inverse of b mod a)


#ifdef CBMRPM41_DEBUG
static signed int debugLineNumber=0;

#   define SETSTATEDEBUG(_x)  \
    debugLineNumber=__LINE__; \
    (_x)

void printDebugCounters()
{
    fprintf(stderr, "file: " __FILE__
           "\n\tversion: " OPENCBM_VERSION ", built: " __DATE__ " " __TIME__
           "\n\tlineNumber=%d\n", debugLineNumber);
}
#else
#    define SETSTATEDEBUG(_x) (void)0
#endif


static void
help()
{
    printf
    (
        "Usage: cbmrpm41 [OPTION]... DRIVE\n"
        "High precision CBM-1541 rpm measurement\n"
        "\n"
        "  -h, --help       display this help and exit\n"
        "  -V, --version    display version information and exit\n"
        "\n"
        "  -x, --extended   measure out a 40 track disk\n"
        "  -s, --status     display drive status after the measurements\n"
        "  -r, --retries n  number of measurement retries for each track\n"
        "\n"
    );
}

static void
hint(char *s)
{
    fprintf(stderr, "Try `%s' -h for more information.\n", s);
}

static void ARCH_SIGNALDECL
handle_CTRL_C(int dummy)
{
//    const static char CmdBuffer[]="M-W\013\034\001\101\r";

    fprintf(stderr, "\nSIGINT caught, resetting IEC bus...\n");
#ifdef CBMRPM41_DEBUG
    printDebugCounters();
#endif
    arch_sleep(1);
    cbm_reset(fd);

#if 0   // reset automatically restores the VIA shift register
    arch_usleep(100000);
    fprintf(stderr, "Emergency resetting VIA2 shift register to default.\n");
    cbm_exec_command(fd, drive, CmdBuffer, sizeof(CmdBuffer));
#endif

    cbm_driver_close(fd);
    exit(1);
}

static int
cbm_sendUxCommand(CBM_FILE HandleDevice, __u_char DeviceAddress, enum UcmdVectorNames UxCommand)
{
    char UxCmdBuffer[3]="U_";

    UxCmdBuffer[1]=UxCommand;
// printf("Sending user command \"%s\" to floppy\n", UxCmdBuffer);
    return cbm_exec_command(HandleDevice, DeviceAddress, UxCmdBuffer, 2);
}

static unsigned int
reconstruct_v32bitInc(struct Timer24bitValues TimerRegisters)
{
       /*
        * The virtual 23.589 bit timer can further be extended to
        * 32 bits with a software method. But this works only
        * correctly, as long as two consecutive time measurements
        * (from the very same timing source of course) always
        * differ in less than a complete wrap around of the
        * virtual timer. On a base clock of 1Mhz and the Modulus
        * of 256*256*187 this is a time window of somewhat around
        * 12s (Modulus / 1MHz).
        */
    static int lastVTimer = 0;
    static unsigned int ModulusDecrementor = 0;

    register int vTimer;

// #define Timer23Debug 1
#if Timer23Debug >= 1
    printf("Plain 23.589 bit timer values read: 0x%02x 0x%02x 0x%02x\n",
           TimerRegisters.V2T2__LOW , TimerRegisters.V1T2__LOW, TimerRegisters.V1T2_HIGH);
#endif

    vTimer   = TimerRegisters.V1T2_HIGH;
    vTimer <<= 8;
    vTimer  |= TimerRegisters.V1T2__LOW;
    vTimer  *= V1T2rec1;
    vTimer  %= Modulus;
    vTimer  *= V1T2rec2;
    vTimer  %= Modulus;

    vTimer  += V2T2rec * TimerRegisters.V2T2__LOW;
    vTimer  %= Modulus;

#if Timer23Debug >= 1
    printf("Reconstructed 23.589 bit timer value: 0x%06x / %8d\n", vTimer, vTimer);
#endif

        // obey that the timer decrements on each tick
    if ( vTimer > lastVTimer )
    {
            // the timer increased, thus we have got a wrap
            // around ==> decrement the 32 bits software
            // timer by the modulus of the virtual timer
        ModulusDecrementor -= Modulus;
    }

    lastVTimer = vTimer;

        // by taking the (( 2^n ) - 1 )-Inverse, we "convert"
        // the timer an increasing one instead of decreasing
        // with each tick
    return ~(ModulusDecrementor + vTimer);
}

static float
measure_2cyleJitter(CBM_FILE HandleDevice, __u_char DeviceAddress, __u_char diskTrack, __u_char count)
{
    unsigned char cmd[10], insts[40];
    unsigned int mNo, timerValue, lastTvalue, firstTvalue;
#if _MINMAX_VALUES_PRINTOUT
    unsigned int dMin=~0, dMax=0;
#endif
    struct Timer24bitValues T24Sample;

    SETSTATEDEBUG((void)0);

#if _ASCII_PARAMETER_PASSING
        // must be: "Ux <track> <sector>"
        // sprintf(cmd, "U%c %d %d", ExecuteJobInBuffer, i, i & 0x0f);
    sprintf(cmd, "U%c %d 0", ExecuteJobInBuffer, diskTrack);
#else
        // must be: "Ux<track><sector>" with directly encoded bytes
    sprintf(cmd, "U%c%c%c", ExecuteJobInBuffer, diskTrack, 1);
#endif

        // for each track do 1 initialisation and then
        // several measurements
    timerValue = 0;
    for(mNo = 0; mNo <= count; mNo++)
    {
        lastTvalue = timerValue;

#if _ASCII_PARAMETER_PASSING
        if( cbm_exec_command(HandleDevice, DeviceAddress, cmd, strlen(cmd))
            != 0) return -1.0;
#else
        if( cbm_exec_command(HandleDevice, DeviceAddress, cmd, 4)
            != 0) return -1.0;
#endif
        SETSTATEDEBUG((void)0);

            // wait for job to finish
        if( cbm_device_status(HandleDevice, DeviceAddress, insts, sizeof(insts)) )
        {
            printf("%s\n", insts);
        }

        if( cbm_download(HandleDevice, DeviceAddress,
                     timerShotMain, (__u_char *) & T24Sample,
                     sizeof(T24Sample))
             != sizeof(T24Sample)) return -1.0;

            // read out sample that was shot by the jobcode
        timerValue = reconstruct_v32bitInc(T24Sample);

        if(mNo > 0){
            lastTvalue = timerValue - lastTvalue;
            printf("%6u ", lastTvalue);
#if _MINMAX_VALUES_PRINTOUT
            if(lastTvalue > dMax) dMax = lastTvalue;
            if(lastTvalue < dMin) dMin = lastTvalue;
#endif
        }
        else
        {
            firstTvalue = timerValue;
            printf(" %2d | %10u ||", diskTrack, timerValue);
        }
    }
#if _MINMAX_VALUES_PRINTOUT
    printf(" %6u..%6u=%2u", dMin, dMax, dMax - dMin);
#endif
    return (float)(timerValue - firstTvalue) / (mNo - 1);
}


int ARCH_MAINDECL
main(int argc, char *argv[])
{
    int status = 0;
    unsigned char cmd[40], endtrack = 35, retries = 5;
    __u_char track;
    char c, *arg;
    float meanTime;
    int berror = 0;

    struct option longopts[] =
    {
        { "help"       , no_argument      , NULL, 'h' },
        { "version"    , no_argument      , NULL, 'V' },
        { "extended"   , no_argument      , NULL, 'x' },
        { "status"     , no_argument      , NULL, 's' },
        { "retries"    , required_argument, NULL, 'r' },
        { NULL         , 0                , NULL, 0   }
    };

    const char shortopts[] ="hVxsr:";

    while((c=(unsigned char)getopt_long(argc, argv, shortopts, longopts, NULL)) != -1)
    {
        switch(c)
        {
            case 'h': help();
                      return 0;
            case 'V': printf("cbmrpm41 %s\n", OPENCBM_VERSION ", built on " __DATE__ " at " __TIME__ "\n");
                      return 0;
            case 's': status = 1;
                      break;
            case 'x': endtrack = 40;
                      break;

            case 'r': retries = arch_atoc(optarg);
                      if(retries<1)       retries= 1;
                      else if(retries>63) retries=63;
                      break;

            default : hint(argv[0]);
                      return 1;
        }
    }

    if(optind + 1 != argc)
    {
        fprintf(stderr, "Usage: %s [OPTION]... DRIVE\n", argv[0]);
        hint(argv[0]);
        return 1;
    }

    arg = argv[optind++];
    drive = arch_atoc(arg);
    if(drive < 8 || drive > 11)
    {
        fprintf(stderr, "Invalid drive number (%s)\n", arg);
        return 1;
    }

    SETSTATEDEBUG((void)0);
    printf("Please remove any diskettes used with production data on it. Insert a freshly\n"
           "formatted disk into drive %d; you can format a disk with e.g. the command:\n\n"
           "        cbmforng -o -v %d freshdisk,fd\n\n"
           "Press <Enter>, when ready or press <CTRL>-C to abort.\r", drive, drive);
    getchar();

    if(cbm_driver_open(&fd, 0) == 0) do
    {
        signal(SIGINT, handle_CTRL_C);

        SETSTATEDEBUG((void)0);
        if( cbm_upload(fd, drive, sizeof(cbmDev_StartAddress), cbmrpm41, sizeof(cbmrpm41))
            != sizeof(cbmrpm41)) break;

            // location of the new U vector user commands table
        sprintf(cmd, "%c%c", UcmdTblAddr & 0xFF, UcmdTblAddr >> 8);
            // install the new U vector table
        SETSTATEDEBUG((void)0);
        if( cbm_upload(fd, drive, sizeof(cbmDev_UxCMDtVector), cmd, 2)
            != 2) break;

            // execute Ux command behind the symbolic name Init23_BitTimersStd
        SETSTATEDEBUG((void)0);
        if( cbm_sendUxCommand(fd, drive, Init23_BitTimersStd)
            != 0) break;

            // read disk ID and initialise other parameters
            // from the currently inserted disk into the
            // drive's RAM locations
        SETSTATEDEBUG((void)0);
        if( cbm_exec_command(fd, drive, "I0", 2)
            != 0) break;

        SETSTATEDEBUG((void)0);
        berror = cbm_device_status(fd, drive, cmd, sizeof(cmd));
        if(berror && status)
        {
            printf("%s\n", cmd);
        }

        printf(" TR | timer abs. ||delta1,delta2,...                 |mean delta|mean rpm\n"
               "----+------------++------+---------------------------+----------+---------\n");
#if 1
        for(track=1; track <= endtrack; track++)
        {
            meanTime = measure_2cyleJitter(fd, drive, track, retries);
            if(meanTime < 0.0) break;
            printf(" %8.1f | %7.3f\n", meanTime, 60000000.0 / meanTime);
        }
#else
        for( track=17; track<=31; track += (track == 18 ? 12 : 1) ) // 17,18,30,31 for all zones
        {
            // meanTime = 
            measure_2cyleJitter(fd, drive, track, retries);
            // printf(" %8.1f | %10.6f\n", meanTime, 60000000.0 / meanTime);
            printf("\n");
        }
#endif

        if( cbm_sendUxCommand(fd, drive, ResetVIA2ShiftRegConfig)
            !=0 ) break;
        if( cbm_sendUxCommand(fd, drive,      ResetUxVectorTable)
            !=0 ) break;

        if( cbm_exec_command(fd, drive, "I", 2)
            !=0 ) break;

        if(!berror && status)
        {
            cbm_device_status(fd, drive, cmd, sizeof(cmd));
            printf("%s\n", cmd);
        }
        cbm_driver_close(fd);
        return 0;
    } while(0);
    else
    {
        arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name(0));
        return 1;
    }
        // if the do{}while(0) loop is exited with a break, we get here
    arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name(0));
    cbm_driver_close(fd);
    return 1;
}
