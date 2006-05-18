/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright (C) 2006 Wolfgang Moser, http://wmsr.de
 *  Copyright (C) 1999 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 */

#ifdef SAVE_RCSID
static char *rcsid =
    "@(#) $Id: cbmrpm41.c,v 1.2 2006-05-18 14:42:02 wmsr Exp $";
#endif

#if _MSC_VER >= 1400
#   pragma warning( disable : 4996 )
#endif

#include "cbmrpm41.h"

static unsigned char cbmrpm41[] = {
#include "cbmrpm41.inc"
};

#define _ASCII_PARAMETER_PASSING 1

const unsigned short loadAddress  = 0x0300;
const unsigned short timerValAddr = 0x0303;    // not used currently
const unsigned short UcmdTblAddr  = 0x0306;

static CBM_FILE fd;
static unsigned char drive;

#ifdef CBMRPM41_DEBUG
static signed int debugLineNumber=0;    // , debugBlockCount=0, debugByteCount=0;

#   define SETSTATEDEBUG(_x)  \
    debugLineNumber=__LINE__; \
    (_x)

void printDebugCounters()
{
    printf("file: " __FILE__
           "\n\tversion: " OPENCBM_VERSION ", built: " __DATE__ " " __TIME__
#   if 1
           "\n\tlineNumber=%d\n", debugLineNumber);
#   else
           "\n\tlineNumber=%d, blockCount=%d, byteCount=%d\n",
           debugLineNumber, debugBlockCount, debugByteCount);
#   endif
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
    const static char CmdBuffer[]="M-W\013\034\001\101";

    fprintf(stderr, "\nSIGINT caught, resetting IEC bus...\n");
#ifdef CBMRPM41_DEBUG
    printDebugCounters();
#endif
    arch_sleep(1);
    cbm_reset(fd);

#if 0
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
cbm_readTimer(CBM_FILE HandleDevice, __u_char DeviceAddress)
{
       /*
        * The virtual 23.589 bit timer value is reconstructed from
        * a 16 bit timer and an 8 bit timer with a modulus of 187
        * with the help of the Chinese Remainder Theorem. All the
        * constants below are precalculated coefficients for a
        * modulus of 65536 for one timer and a modulus of 187 for
        * the other timer.
        */
    const static int Via1Timer2Max = 256 * 256;
    const static int Via2Timer2Max = (185 + 2);   // == 187

    const static int Modulus  = 12255232;
    const static int V1T2rec1 =    27081;      // 16-Bit timer
    const static int V1T2rec2 =      121;
    const static int V2T2rec  =  8978432;      // 7.89-Bit timer

       /*
        * The virtual 23.589 bit timer can further be extended to
        * 32 bits with a software method. But this works only
        * correctly, as long as two consecutive time measurements
        * always differ in less than a complete wrap around of
        * the virtual timer. On a base clock of 1Mhz this is a
        * time window of somewhat around 12s (Modulus / 1MHz).
        */
    static int lastVTimer = 0;
    static unsigned int ModulusDecrementor = 0;


    static unsigned char trashcan[16];
    struct Timer24bitValues TimerTriple;
    register int vTimer;

    // SETSTATEDEBUG((void)0);
        // assume the user command table with the symbolic command
        // "T2_23Bit_TimerSampling" to be installed correctly
        // actually don't do an explicit sample command
    // cbm_sendUxCommand(HandleDevice, DeviceAddress,  T2_23Bit_TimerSampling);
    SETSTATEDEBUG((void)0);

    cbm_exec_command (HandleDevice, DeviceAddress, "M-R\x3\x3\x3", 6);
    SETSTATEDEBUG((void)0);
    cbm_talk         (HandleDevice, DeviceAddress, 15);
    cbm_raw_read     (HandleDevice, &TimerTriple, sizeof(TimerTriple));
        // sends one more than the requested 3 bytes, get the rest into the trashcan
    cbm_raw_read     (HandleDevice, trashcan, sizeof(trashcan));
    cbm_untalk       (HandleDevice);
    SETSTATEDEBUG((void)0);

// #define Timer23Debug 1
#if Timer23Debug >= 1
    printf("Plain 23.589 bit timer values read: 0x%02x 0x%02x 0x%02x\n",
           TimerTriple.V2T2__LOW , TimerTriple.V1T2__LOW, TimerTriple.V1T2_HIGH);
#endif

    vTimer   = TimerTriple.V1T2_HIGH;
    vTimer <<= 8;
    vTimer  |= TimerTriple.V1T2__LOW;
    vTimer  *= V1T2rec1;
    vTimer  %= Modulus;
    vTimer  *= V1T2rec2;
    vTimer  %= Modulus;

    vTimer  += TimerTriple.V2T2__LOW * V2T2rec;
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

int ARCH_MAINDECL
main(int argc, char *argv[])
{
    int status = 0;
    unsigned char cmd[40], insts[40], endtrack = 35, retries = 5;
    unsigned int track, mNo, timerValue, lastTvalue, firstTvalue;
    char c, *arg;
    int berror = 0;
    float meanTime;

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
           "Press <Enter>, when ready or press <CTRL>-C to abort.", drive, drive);
    getchar();

    if(cbm_driver_open(&fd, 0) == 0)
    {
        signal(SIGINT, handle_CTRL_C);

        cbm_upload(fd, drive, loadAddress, cbmrpm41, sizeof(cbmrpm41));

            // location of the new U vector user commands table
        sprintf(cmd, "%c%c", UcmdTblAddr & 0xFF, UcmdTblAddr >> 8);
            // install the new U vector table
        cbm_upload(fd, drive, 0x006b, cmd, 2);

            // execute Ux command behind the symbolic name Init23_BitTimersStd
        cbm_sendUxCommand(fd, drive, Init23_BitTimersStd);

            // read disk ID and initialise other parameters
            // from the currently inserted disk into the
            // drive's RAM locations
        cbm_exec_command(fd, drive, "I0", 2);

        berror = cbm_device_status(fd, drive, insts, sizeof(insts));
        if(berror && status)
        {
            printf("%s\n", insts);
        }

        printf(" TR | timer abs. ||delta1,delta2,...                 | mean DLT |   mean rpm\n"
               "----+------------++------+---------------------------+----------+------------\n");
        for(track=1; track <= endtrack; track++)
        {
            SETSTATEDEBUG((void)0);

#if _ASCII_PARAMETER_PASSING
                // must be: "Ux <track> <sector>"
                // sprintf(cmd, "U%c %d %d", ExecuteJobInBuffer, i, i & 0x0f);
            sprintf(cmd, "U%c %d 0", ExecuteJobInBuffer, track);
#else
                // must be: "Ux<track><sector>" with directly encoded bytes
            sprintf(cmd, "U%c%c%c", ExecuteJobInBuffer, track, 1);
#endif

                // for each track do 1 initialisation and then
                // several measurements
            timerValue = 0;
            for(mNo = 0; mNo <= retries; mNo++)
            {
                lastTvalue = timerValue;

#if _ASCII_PARAMETER_PASSING
                cbm_exec_command(fd, drive, cmd, strlen(cmd));
#else
                cbm_exec_command(fd, drive, cmd, 4);
#endif
                SETSTATEDEBUG((void)0);

                    // wait for job to finish
                if( cbm_device_status(fd, drive, insts, sizeof(insts)) )
                {
                    printf("%s\n", insts);
                }

                    // read out sample that was shot by the jobcode
                timerValue = cbm_readTimer(fd, drive);

                if(mNo == 0)
                {
                    firstTvalue = timerValue;
                    printf(" %2d | %10u ||", track, timerValue);
                } else {
                    printf("%6u|", timerValue - lastTvalue);
                }
            }
            meanTime = (float)(timerValue - firstTvalue) / (mNo - 1);
            printf(" %8.1f | %10.6f\n", meanTime, 60000000.0 / meanTime);
        }

        cbm_sendUxCommand(fd, drive, ResetVIA2ShiftRegConfig);
        cbm_sendUxCommand(fd, drive,      ResetUxVectorTable);

        cbm_exec_command(fd, drive, "I", 2);

        if(!berror && status)
        {
            cbm_device_status(fd, drive, insts, sizeof(insts));
            printf("%s\n", insts);
        }
        cbm_driver_close(fd);
        return 0;
    }
    else
    {
        arch_error(0, arch_get_errno(), "%s", cbm_get_driver_name(0));
        return 1;
    }
}
