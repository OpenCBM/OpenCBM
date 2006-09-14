/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis
 *
 */

/*! ************************************************************** 
** \file instcbm.c \n
** \author Spiro Trikaliotis \n
** \version $Id: instcbm.c,v 1.25 2006-09-14 19:15:17 strik Exp $ \n
** \n
** \brief Program to install and uninstall the OPENCBM driver
**
****************************************************************/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include "cbmioctl.h"
#include "version.h"
#include "arch.h"
#include "i_opencbm.h"

#include <getopt.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "INSTCBM.EXE"

/*! This file is "like" debug.c, that is, define some variables */
#define DBG_IS_DEBUG_C

#include "debug.h"

#include "instcbm.h"

/*! \brief Format a returned error code into a string

 This function formats a returned error code into a string.

 \param Error
   The error number to be formatted.

 \return 
   The string describing the error given by the error code.
*/
PCHAR
FormatErrorMessage(DWORD Error)
{
    static char ErrorMessageBuffer[2048];
    int n;

    // Format the message

    n = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        NULL,
        Error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &ErrorMessageBuffer,
        sizeof(ErrorMessageBuffer)-1,
        NULL);

    // make sure there is a trailing zero

    ErrorMessageBuffer[n] = 0;

    return ErrorMessageBuffer;
}

/*! This type describes the operating system we are running on */
typedef 
enum osversion_e
{
    WINUNSUPPORTED, //!< an unsupported operating system
    WINNT3,         //!< Windows NT 3.x (does the driver work there?
    WINNT4,         //!< Windows NT 4.x
    WIN2000,        //!< Windows 2000 (NT 5.0)
    WINXP,          //!< Windows XP (NT 5.1)
    WINNEWER        //!< newer than WIN XP
} osversion_t;


/*! \internal \brief Check if we have the needed access rights

 This function checks if we have the needed access rights for
 installing the driver on the machine.

 \param Parameter
   Pointer to parameter_t struct which contains the
   description of the parameters given on the command-line.

 \return 
   Return value which will be given on return from main()
   That is, 0 on success, everything else indicates an error.
*/
static BOOL
NeededAccessRights(VOID)
{
    SC_HANDLE scManager;

    FUNC_ENTER();

    // Check if we have arbitrary execute rights.
    // For this, open the service control manager

    scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (scManager)
    {
        CloseServiceHandle(scManager);
    }

    FUNC_LEAVE_BOOL(scManager ? TRUE : FALSE);
}

/*! \brief \internal Check out the operating system version

 This function checks out the operating system version we are
 running on.

 \return 
   The operating system version.
*/
static osversion_t
GetOsVersion(VOID)
{
    OSVERSIONINFO ovi;
    osversion_t retValue;

    FUNC_ENTER();

    // If we do not find anything, it will be an unsupported OS version

    retValue = WINUNSUPPORTED;

    // Check out the operating system version

    ovi.dwOSVersionInfoSize = sizeof(ovi);

    if (GetVersionEx(&ovi))
    {
        DBGDO(char *platform = "";)

        switch (ovi.dwPlatformId)
        {
        case VER_PLATFORM_WIN32s:
            DBGDO(platform = "WIN32s (huh?)";)
            break;

        case VER_PLATFORM_WIN32_WINDOWS:
            DBGDO(platform = "Win 95/98/Me";)
            break;

        case VER_PLATFORM_WIN32_NT:
            DBGDO(platform = "Win NT/2K/XP";)

            switch (ovi.dwMajorVersion)
            {
            case 0: case 1: case 2: /* SHOULD NOT OCCUR AT ALL! */
                /* unsupported */
                break;

            case 3:
                /* NT 3.x versions are untested */
                retValue = WINNT3;
                DBG_PRINT((DBG_PREFIX "Running on NT 3.x!"));
                printf("You're using Windows NT 3.%02u. These versions are untested, but they might work.\n"
                    "If it works for you, I would be very happy if you contact me and tell me!\n",
                    ovi.dwMinorVersion);
                break;

            case 4:
                retValue = WINNT4;
                break;

            case 5:
                switch (ovi.dwMinorVersion)
                {
                case 0:  retValue = WIN2000;  break;
                case 1:  retValue = WINXP;    break;
                default: retValue = WINNEWER; break; // comment below for default dwMajorversion applies here, too
                }
                break;

            default:
                // This is a version of Windows we do not know; anyway, since
                // it is NT based, and it's major version is >= 5, we support it
                // anyway!

                retValue = WINNEWER;
                break;
            }
            break;

        default:
            DBGDO(platform = "unknown (huh?)";)
            break;
        }

        DBG_SUCCESS((DBG_PREFIX "OS VERSION: %u.%u.%u %s (%s)",
            ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber,
            platform, ovi.szCSDVersion));
    }

    FUNC_LEAVE_TYPE(retValue, osversion_t, "%u");
}

/*! \brief \internal Output version information of instcbm */
static VOID
version(VOID)
{
    FUNC_ENTER();

    printf("instcbm Version " OPENCBM_VERSION ", built on " __DATE__ " at " __TIME__ "\n");

    FUNC_LEAVE();
}

/*! \brief \internal Print out the help screen */
static VOID
usage(VOID)
{
    FUNC_ENTER();

    version();

    printf("\nUsage: instcbm [options]\n"
            "Install the cbm4win driver on the system, or remove it.\n"
            "\n"
            "Options:\n"
            "  -h, --help      display this help and exit\n"
            "  -V, --version   display version information about cbm4win\n"
            "  -r, --remove    remove (uninstall) the driver\n"
            "  -e, --enumpport re-enumerate the parallel port driver\n"
            "  -u, --update    update parameters if driver is already installed.\n"
            "  -l, --lpt=no    set default LPT port\n"
            "  -t, --cabletype=TYPE set cabletype to 'auto', 'xa1541' or 'xm1541'.\n"
            "                  If not specified, --cabletype=auto is assumed.\n"
            "  -L, --lock=WHAT automatically lock the driver 'yes' or not 'no'.\n"
            "                  If not specified, --lock=yes is assumed.\n"
            "  -n, --nocopy    do not copy the driver files into the system directory\n"
            "  -c, --check     only check if the installation is ok\n"
#ifdef _X86_
            "  -F, --forcent4  force NT4 driver on a Win 2000, XP, or newer systems\n" 
            "                  (NOT RECOMMENDED!)\n"
#endif // #ifdef _X86_
            "  -A, --automatic (default) automatically start the driver on system boot.\n"
            "                  The driver can be used from a normal user, no need for\n"
            "                  administrator rights.\n"
            "                  The opposite of --on-demand.\n"
            "  -O, --on-demand start the driver only on demand.\n"
            "                  The opposite of --automatic.\n"
            "\n");

    FUNC_LEAVE();
}

/*! \brief \internal Print out a hint how to get help */
static VOID
hint(char *s)
{
    fprintf(stderr, "Try `%s' --help for more information.\n", s);
}

/*! \brief The parameter which are given on the command-line */
typedef
struct parameter_s
{
    /*! Do not execute anything */
    BOOL NoExecute;

    /*! Administrator privileges not needed */
    BOOL NoAdminNeeded;

    /*! Find out of more than one "execute" parameter is given */
    BOOL ExecuteParameterGiven;

    /*! --remove was given */
    BOOL Remove;

    /*! --enum was given */
    BOOL EnumerateParport;

    /*! --nocopy was given */
    BOOL NoCopy;

    /*! --forcent4 was given */
    BOOL ForceNt4;

    /*! --update was given */
    BOOL Update;

    /*! --check was given */
    BOOL CheckInstall;

    /*! --lpt was given, the number which was there */
    ULONG Lpt;

    /*! the IEC cable type was specified */
    IEC_CABLETYPE IecCableType;
    
    /*! It was specified that the driver is to be permanently locked */
    ULONG PermanentlyLock;

    /*! --automatic or --on-demand was given */
    BOOL AutomaticOrOnDemandStart;

    /*! --automatic was given, start the driver automatically */
    BOOL AutomaticStart;

#if DBG

    /*! --buffer was given */
    BOOL OutputDebuggingBuffer;

    /*! --debugflags was given */
    BOOL DebugFlagsDriverWereGiven;

    /*! --debugflags, a second parameter (for the DLL) was given */
    BOOL DebugFlagsDllWereGiven;

    /*! --debugflags, a third parameter (for INSTCBM itself) was given */
    BOOL DebugFlagsInstallWereGiven;

    /*! if --debugflags was given: the number which was there */
    ULONG DebugFlagsDriver;

    /*! if --debugflags with 2 parameters was given: the number which was there */
    ULONG DebugFlagsDll;

    /*! if --debugflags with 3 parameters was given: the number which was there */
    ULONG DebugFlagsInstall;

#endif // #if DBG

    /*! The type of the OS version */
    osversion_t OsVersion;
} parameter_t;

/*! \internal \brief Process a number

 This function processes a number which was given as a string.

 \param Argument:
   Pointer to the number in ASCII representation

 \param NextChar:
   Pointer to a PCHAR which will had the address of the next
   char not used on return. This can be NULL.

 \param ParameterGiven:
   Pointer to a BOOL which will be set to TRUE if the value
   could be calculated correctly. Can be NULL.

 \param ParameterValue:
   Pointer to a ULONG which will get the result.

 \return
   TRUE on error, FALSE on success.

 If this parameter is given more than once, the last occurence
 takes precedence.

 The number can be specified in octal (0***), hex (0x***), or
 decimal (anything else).

 If NextChar is NULL, the Argument *must* terminate at the
 end of the number. If NextChar is not NULL, the Argument might
 contain a comma.
*/
static BOOL
processNumber(const PCHAR Argument, PCHAR *NextChar, PBOOL ParameterGiven, PULONG ParameterValue)
{
    PCHAR p;
    BOOL error;
    int base;

    FUNC_ENTER();

    DBG_ASSERT(ParameterValue != NULL);

    error = FALSE;
    p = Argument;

    if (p)
    {
        // Find out which base to use (0***, 0x***, or anything else)

        switch (*p)
        {
        case 0:
            error = TRUE;
            break;

        case '0':
            switch (*++p)
            {
            case 'x': case 'X':
                base = 16;
                ++p;
                break;

            default:
                base = 8;
                break;
            };
            break;

        default:
            base = 10;
            break;
        }

        // Convert the value

        if (!error)
        {
            *ParameterValue = strtoul(p, &p, base);

            if (NextChar)
            {
                error = ((*p != 0) && (*p != ',')) ? TRUE : FALSE;
            }
            else
            {
                error = *p != 0 ? TRUE : FALSE;
            }

            if (!error)
            {
                if (NextChar != NULL)
                {
                    *NextChar = p + ((*p) ? 1 : 0);
                }

                if (ParameterGiven != NULL)
                {
                    *ParameterGiven = TRUE;
                }
            }
        }
    }

    FUNC_LEAVE_BOOL(error);
}

/*! \internal \brief Process the command line arguments

 This function processes the command line arguments.

 \param Argc
   The count of arguments, as given to main().

 \param Argv
   The arguments vector, as given to main().

 \param Parameter
   Pointer to parameter_t struct which will contain the
   description of the parameters on return

 \return 
   TRUE on error, else FALSE.
*/
static BOOL
processargs(int Argc, char **Argv, parameter_t *Parameter)
{
    BOOL error;
    int c;

    struct option longopts[] =
    {
        { "help",       no_argument,       NULL, 'h' },
        { "version",    no_argument,       NULL, 'V' },
        { "remove",     no_argument,       NULL, 'r' },
        { "enumpport",  no_argument,       NULL, 'e' },
#ifdef _X86_
        { "forcent4",   no_argument,       NULL, 'F' },
#endif // #ifdef _X86_
        { "lpt",        required_argument, NULL, 'l' },
        { "update",     no_argument,       NULL, 'u' },
        { "check",      no_argument,       NULL, 'c' },
        { "cabletype",  required_argument, NULL, 't' },
        { "lock",       required_argument, NULL, 'L' },
#if DBG
        { "debugflags", required_argument, NULL, 'D' },
        { "buffer",     no_argument,       NULL, 'B' },
#endif // #if DBG
        { "nocopy",     no_argument,       NULL, 'n' },
        { "automatic",  no_argument,       NULL, 'A' },
        { "on-demand",  no_argument,       NULL, 'O' },
        { NULL,         0,                 NULL, 0   }
    };

    const char shortopts[] = "hrel:nuct:L:AOV"
#ifdef _X86_
                             "F"
#endif // #ifdef _X86_

#if DBG
                             "D:B"
#endif // #if DBG
                             ;

    FUNC_ENTER();

    error = FALSE;

    // Clear the Parameter set 

    DBG_ASSERT(Parameter);
    memset(Parameter, 0, sizeof(*Parameter));

    // We have not specified an LPT port yet

    Parameter->Lpt = (ULONG) -1;

    // No IEC cable type was specified

    Parameter->IecCableType = IEC_CABLETYPE_UNSPEC;

    // It was not specified if the driver is to be permenently locked

    Parameter->PermanentlyLock = (ULONG) -1;

    // set the default: automaticstart -A

    Parameter->AutomaticStart = TRUE;

#if DBG

    // Until now, no DebugFlags were on the line

    Parameter->DebugFlagsDriverWereGiven = FALSE;
    Parameter->DebugFlagsDllWereGiven = FALSE;

#endif // #if DBG

    while ((c=getopt_long(Argc, Argv, shortopts, longopts, NULL)) != -1)
    {
        switch (c)
        {
        case 'h':
            usage();
            Parameter->NoExecute = TRUE;
            Parameter->NoAdminNeeded = TRUE;
            break;

        case 'V':
            version();
            Parameter->NoExecute = TRUE;
            Parameter->NoAdminNeeded = TRUE;
            break;

        case 'r':
            if (Parameter->ExecuteParameterGiven)
            {
                error = TRUE;
                printf("Colliding parameters were given, aborting!");
                hint(Argv[0]);
            }
            else
            {
                Parameter->ExecuteParameterGiven = TRUE;
                Parameter->Remove = TRUE;
            }
            break;

        case 'e':
            if (Parameter->ExecuteParameterGiven)
            {
                error = TRUE;
                printf("Colliding parameters were given, aborting!");
                hint(Argv[0]);
            }
            else
            {
                Parameter->ExecuteParameterGiven = TRUE;
                Parameter->EnumerateParport = TRUE;
            }
            break;

        case 't':
            if ((optarg == NULL) || (strcmp(optarg, "auto") == 0))
                Parameter->IecCableType = IEC_CABLETYPE_AUTO;
            else if (strcmp(optarg, "xa1541") == 0)
                Parameter->IecCableType = IEC_CABLETYPE_XA;
            else if (strcmp(optarg, "xm1541") == 0)
                Parameter->IecCableType = IEC_CABLETYPE_XM;
            else
            {
                fprintf(stderr, "you must specify 'xa1541', 'xm1541' or 'auto' for --cabletype\n");
                error = TRUE;
            }
            break;

        case 'L':
            if (optarg == NULL
                || (strcmp(optarg, "+") == 0)
                || (strcmp(optarg, "yes") == 0)
                || (strcmp(optarg, "true") == 0)
               )
            {
                Parameter->PermanentlyLock = 1;
            }
            else if (optarg != NULL &&
                    (   (strcmp(optarg, "-") == 0)
                     || (strcmp(optarg, "no") == 0)
                     || (strcmp(optarg, "false") == 0)
                    )
                    )
            {
                Parameter->PermanentlyLock = 0;
            }
            else
            {
                fprintf(stderr, "you must specify 'yes' or 'no' for --lock\n");
                error = TRUE;
            }
            break;

        case 'n':
            Parameter->NoCopy = TRUE;
            break;

        case 'c':
            Parameter->CheckInstall = TRUE;
            Parameter->NoAdminNeeded = TRUE;
            break;

#ifdef _X86_
        case 'F':
            if (Parameter->ExecuteParameterGiven)
            {
                error = TRUE;
                printf("Colliding parameters were given, aborting!");
                hint(Argv[0]);
            }
            else
            {
                Parameter->ExecuteParameterGiven = TRUE;
                Parameter->ForceNt4 = TRUE;
                Parameter->Remove = FALSE;
            }
            break;
#endif // #ifdef _X86_

        case 'l':
            error = processNumber(optarg, NULL, NULL, &Parameter->Lpt);
            break;

#if DBG

        case 'D':
            {
                PCHAR next;

                error = processNumber(optarg, &next, &Parameter->DebugFlagsDriverWereGiven,
                    &Parameter->DebugFlagsDriver);

                if (!error && next && *next)
                {
                    error = processNumber(next, &next, &Parameter->DebugFlagsDllWereGiven,
                        &Parameter->DebugFlagsDll);
                }

                if (!error && next && *next)
                {
                    error = processNumber(next, NULL, &Parameter->DebugFlagsInstallWereGiven,
                        &Parameter->DebugFlagsInstall);

                    if (!error && Parameter->DebugFlagsInstallWereGiven)
                    {
                        DbgFlags = Parameter->DebugFlagsInstall;
                    }
                }
                DBG_PRINT((DBG_PREFIX "error = %s, 1st = %08x (%s), 2nd = %08x (%s), 3rd = %08x (%s)",
                    error ? "TRUE" : "FALSE", 
                    Parameter->DebugFlagsDriver, Parameter->DebugFlagsDriverWereGiven ? "TRUE" : "FALSE", 
                    Parameter->DebugFlagsDll, Parameter->DebugFlagsDllWereGiven ? "TRUE" : "FALSE",
                    Parameter->DebugFlagsInstall, Parameter->DebugFlagsInstallWereGiven ? "TRUE" : "FALSE"));
            }
            break;

        case 'B':
            Parameter->OutputDebuggingBuffer = TRUE;
            Parameter->NoExecute = TRUE;
            Parameter->NoAdminNeeded = TRUE;
            break;

#endif // #if DBG

        case 'u':
            Parameter->Update = TRUE;
            break;

        case 'A':
            if (Parameter->AutomaticOrOnDemandStart)
            {
                fprintf(stderr, "--automatic and --on-demand cannot be specified at the same time!\n");
                error = TRUE;
            }
            else
            {
                Parameter->AutomaticStart = TRUE;
                Parameter->AutomaticOrOnDemandStart = TRUE;
            }
            break;

        case 'O':
            if (Parameter->AutomaticOrOnDemandStart)
            {
                fprintf(stderr, "--automatic and --on-demand cannot be specified at the same time!\n");
                error = TRUE;
            }
            else
            {
                Parameter->AutomaticStart = FALSE;
                Parameter->AutomaticOrOnDemandStart = TRUE;
            }
            break;

        default:
            error = TRUE;
            hint(Argv[0]);
            break;
        }
    }

    FUNC_LEAVE_BOOL(error);
}

/*! \internal \brief Concatenate two string

 This function concatenates two strings and returns the
 result in a malloc()ed memory region.

 \param String1
   The first string to concatenate.

 \param String2
   The second string to concatenate.

 \return
   The malloc()ed memory for the concatenated string, or NULL
   if there was not enough memory.
*/
static char *
AllocateConcatenatedString(const char *String1, const char *String2)
{
    char *string;

    FUNC_ENTER();

    DBG_ASSERT(String1 != NULL);
    DBG_ASSERT(String2 != NULL);

    string = malloc(strlen(String1) + strlen(String2) + 1);

    if (string)
    {
        strcpy(string, String1);
        strcat(string, String2);

        DBG_ASSERT(strlen(string) == strlen(String1) + strlen(String2));
    }

    FUNC_LEAVE_STRING(string);
}

/*! \internal \brief Copy a file from a path to another

 This function copies a file from a source directory to a
 destination directory.

 \param SourcePath
   The path from where to copy the file.
   The path has to be terminated with a backslash ("\").

 \param DestPath
   The path where to copy the file to.
   The path has to be terminated with a backslash ("\").

 \param Filename
   The name of the file to copy.

 \param ErrorCode
   The error code to return if the copy fails.

 \return
   0 on success, or ErrorCode in case of an error.
*/
static int
CopyFileToNewPath(const char *SourcePath, const char *DestPath, const char *Filename, const int ErrorCode)
{
    char *sourceFile = NULL;
    char *destFile = NULL;
    int error = 0;

    FUNC_ENTER();

    sourceFile = AllocateConcatenatedString(SourcePath, Filename);
    destFile = AllocateConcatenatedString(DestPath, Filename);

    if (sourceFile && destFile)
    {
        DBG_PRINT((DBG_PREFIX "Copying '%s' to '%s'", sourceFile, destFile));

        printf("Copying '%s' to '%s'", sourceFile, destFile);
        if (!CopyFile(sourceFile, destFile, FALSE))
        {
            error = ErrorCode;
            DBG_PRINT((DBG_PREFIX "--> FAILED!" ));
            printf(" FAILED!\n");
        }
        else
        {
            printf("\n");
        }
    }
    else
    {
        DBG_ERROR((DBG_PREFIX "Error allocating memory buffers for copying '%s'!", Filename));
        fprintf(stderr, "Error allocating memory buffers for copying '%s'.\n", Filename);
    }

    if (sourceFile)
        free(sourceFile);

    if (destFile)
        free(destFile);

    FUNC_LEAVE_INT(error);
}

/*! \internal \brief Delete a file at a path

 This function deletes a file at a specified path.

 \param Path
   The path from where to delete the file.
   The path has to be terminated with a backslash ("\").

 \param Filename
   The name of the file to copy.
*/
static VOID
DeleteFileInDirectory(const char *Path, const char *Filename)
{
    char *file = NULL;

    FUNC_ENTER();

    do {
        file = AllocateConcatenatedString(Path, Filename);

        if (!file)
            break;

        DBG_PRINT((DBG_PREFIX "Trying to delete %s", file));

        DeleteFile(file);

    } while (0);

    if (file)
        free(file);

    FUNC_LEAVE();
}

/*! \internal \brief Process a remove request

 This function removes the the driver from the machine.

 \param Parameter
   Pointer to parameter_t struct which contains the
   description of the parameters given on the command-line.

 \return 
   Return value which will be given on return from main()
   That is, 0 on success, everything else indicates an error.
*/
static int
RemoveDriver(parameter_t *Parameter)
{
    char *driverSystemPath = NULL;
    char *driverPath = NULL;

    FUNC_ENTER();

    UNREFERENCED_PARAMETER(Parameter);

    do {
        char tmpPathString[MAX_PATH];

        if (!CbmCheckPresence(OPENCBM_DRIVERNAME))
        {
            printf("No driver installed, cannot remove.\n");
            break;
        }

        printf("REMOVING driver...\n");

        CbmRemove(OPENCBM_DRIVERNAME);

        // Now, check if we copied the driver and the DLL into the
        // system directory. If this is the case, delete them from
        // there.

        GetSystemDirectory(tmpPathString, sizeof(tmpPathString));
        driverSystemPath = AllocateConcatenatedString(tmpPathString, "\\");
        driverPath = AllocateConcatenatedString(tmpPathString, "\\DRIVERS\\");

        if (!driverSystemPath || !driverPath)
            break;

        // try to delete opencbm.dll

        DeleteFileInDirectory(driverSystemPath, "OPENCBM.DLL");

        // try to delete opencbmvdd.dll

        DeleteFileInDirectory(driverSystemPath, "OPENCBMVDD.DLL");

        // try to delete cbm4nt.sys

        DeleteFileInDirectory(driverPath, "CBM4NT.SYS");

        // try to delete cbm4wdm.sys

        DeleteFileInDirectory(driverPath, "CBM4WDM.SYS");

    } while (0);

    if (driverSystemPath)
        free(driverSystemPath);

    if (driverPath)
        free(driverPath);

    FUNC_LEAVE_INT(0);
}

/*! \internal \brief Check for the correct installation

 This function checks if the driver was corretly installed.

 \param Parameter
   Pointer to parameter_t struct which contains the
   description of the parameters given on the command-line.

 \return 
   Return value which will be given on return from main().
   That is, 0 on success, everything else indicates an error.
*/
static int
CheckDriver(parameter_t *Parameter)
{
    int error;

    FUNC_ENTER();

    UNREFERENCED_PARAMETER(Parameter);

    DBG_PRINT((DBG_PREFIX "Checking configuration for cbm4win"));
    printf("Checking configuration for cbm4win\n");

    if (CbmCheckCorrectInstallation(NeededAccessRights()))
    {
        DBG_PRINT((DBG_PREFIX "There were errors in the current configuration."
            "Please fix them before trying to use the driver!"));
        printf("*** There were errors in the current configuration.\n"
            "*** Please fix them before trying to use the driver!");
        error = 11;
    }
    else
    {
        error = 0;
        DBG_PRINT((DBG_PREFIX "No problems found in current configuration"));
        printf("No problems found in current configuration\n\n");

        /*! \todo Suggested output from WoMo:
            Checking configuration for cbm4win/opencbm:
            No problems found in current configuration:

            Driver configuration:
             Port:               automatic (0), currently using LPT 1
             IRQ mode:           enabled
             Driver start mode:  manually (3)
        */
    }

    FUNC_LEAVE_INT(error);
}

/*! \internal \brief Force re-enumeration of parallel port driver

 This function forces a re-enumeration of the parallel port driver(s).

 \param Parameter
   Pointer to parameter_t struct which contains the
   description of the parameters given on the command-line.

 \return 
   Return value which will be given on return from main().
   That is, 0 on success, everything else indicates an error.
*/
static int
EnumParportDriver(parameter_t *Parameter)
{
    FUNC_ENTER();

    UNREFERENCED_PARAMETER(Parameter);

    DBG_PRINT((DBG_PREFIX "Re-enumerating parallel port driver"));
    printf("Re-enumerating parallel port driver\n");

    CbmParportRestart();

    FUNC_LEAVE_INT(0);
}

/*! \internal \brief Copy the driver files to the system path

 This function copies the driver files to the system path.

 \param Parameter
   Pointer to parameter_t struct which contains the
   description of the parameters given on the command-line.

 \return 
   Return value which will be given on return from main()
   That is, 0 on success, everything else indicates an error.
*/
static int
CopyDriverFiles(parameter_t *Parameter)
{
    char tmpPathString[MAX_PATH];

    char *driverSystemPath = NULL;
    char *driverPath = NULL;
    char *driverLocalPath = NULL;
    char *driverFilename = NULL;

    const char *driverToUse;

    int error = 0;

    FUNC_ENTER();

    printf("Installing driver...\n");

    do {
        //
        // First of all, determine the current working directory
        //

        if (GetCurrentDirectory(sizeof(tmpPathString), tmpPathString) == 0)
        {
            DBG_PRINT((DBG_PREFIX "Could not determine the current working directory!"));
            printf("Could not determine the current working directory!\n");
            error = 4;
            break;
        }

        driverLocalPath = AllocateConcatenatedString(tmpPathString, "\\");

        //
        // Get the system directory
        //

        GetSystemDirectory(tmpPathString, sizeof(tmpPathString));
        driverSystemPath = AllocateConcatenatedString(tmpPathString, "\\");
        driverPath = AllocateConcatenatedString(tmpPathString, "\\DRIVERS\\");

        if (!driverLocalPath || !driverSystemPath || !driverPath)
        {
            DBG_ERROR((DBG_PREFIX "error allocating memory for the paths" ));
            fprintf(stderr, "error allocating memory for the paths\n");
            error = 15;
            break;
        }

        //
        // Find out which driver to use (cbm4wdm.sys, cbm4nt.sys)
        //
        
        driverToUse = ((Parameter->OsVersion > WINNT4) && !Parameter->ForceNt4) ? "cbm4wdm.sys" : "cbm4nt.sys";

        printf("Using driver '%s'\n", driverLocalPath);

        //
        // If we have to copy the files, perform the copy operation for them.
        //

        if (!Parameter->NoCopy)
        {
            // copy the driver into the appropriate directory

            if ((error = CopyFileToNewPath(driverLocalPath, driverPath, driverToUse, 6)) != 0)
                break;

            if ((error = CopyFileToNewPath(driverLocalPath, driverSystemPath, "opencbm.dll", 7)) != 0)
                break;

#ifdef _X86_
            if ((error = CopyFileToNewPath(driverLocalPath, driverSystemPath, "opencbmvdd.dll", 10)) != 0)
                break;
#endif // #ifdef _X86_
        }

        printf("\n");

        //
        // Install the driver
        //

        driverFilename = AllocateConcatenatedString(Parameter->NoCopy ? driverLocalPath : driverPath, driverToUse);

        if (!driverFilename)
        {
            error = 14;
            break;
        }

        CbmInstall(OPENCBM_DRIVERNAME, driverFilename, Parameter->AutomaticStart);

    } while (0);

    if (driverSystemPath)
        free(driverSystemPath);

    if (driverPath)
        free(driverPath);

    if (driverFilename)
        free(driverFilename);

    if (driverLocalPath)
        free(driverLocalPath);

    FUNC_LEAVE_INT(error);
}

/*! \internal \brief Install the driver

 This function installs the driver on the current machine.

 \param Parameter
   Pointer to parameter_t struct which contains the
   description of the parameters given on the command-line.

 \return 
   Return value which will be given on return from main()
   That is, 0 on success, everything else indicates an error.
*/
static int
InstallDriver(parameter_t *Parameter)
{
    int error = 0;

    FUNC_ENTER();

    do {
        if (CbmCheckPresence(OPENCBM_DRIVERNAME))
        {
            printf("Driver is already installed, remove it before you try a new installation,\n"
                "or use the --update option!\n");
            error = 8;
            break;
        }

        if ((error = CopyDriverFiles(Parameter)) != 0)
            break;

        if (!CbmUpdateParameter(Parameter->Lpt,
            Parameter->IecCableType, Parameter->PermanentlyLock,
#if DBG
            Parameter->DebugFlagsDriverWereGiven, Parameter->DebugFlagsDriver,
            Parameter->DebugFlagsDllWereGiven, Parameter->DebugFlagsDll
#else
            0, 0, 0, 0
#endif // #if DBG
            ))
        {
            error = 9;
            break;
        }

        printf("\n");

        if ((error = CheckDriver(Parameter)) != 0)
            break;
        
    } while (0);

    FUNC_LEAVE_INT(error);
}

/*! \internal \brief Update driver settings

 This function updates settings for the already installed driver.

 \param Parameter
   Pointer to parameter_t struct which contains the
   description of the parameters given on the command-line.

 \return 
   Return value which will be given on return from main()
   That is, 0 on success, everything else indicates an error.
*/
static int
UpdateDriver(parameter_t *Parameter)
{
    int error = 0;

    FUNC_ENTER();

    do {
        if (!CbmCheckPresence(OPENCBM_DRIVERNAME))
        {
            printf("Driver is not installed, cannot update the parameters,\n");
            error = 12;
            break;
        }

        if (!CbmUpdateParameter(Parameter->Lpt,
            Parameter->IecCableType, Parameter->PermanentlyLock,
#if DBG
            Parameter->DebugFlagsDriverWereGiven, Parameter->DebugFlagsDriver,
            Parameter->DebugFlagsDllWereGiven, Parameter->DebugFlagsDll
#else
            0, 0, 0, 0
#endif // #if DBG
            ))
        {
            error = 13;
            break;
        }

        printf("\n");
        error = CheckDriver(Parameter);

    } while (0);

    FUNC_LEAVE_INT(error);
}

/*! \brief Main function

 This function performs the action the user has given on the command-line.

 \param Argc
   The count of arguments, as given to main().

 \param Argv
   The arguments vector, as given to main().

 \return 
   0 on success, everything else indicated an error.
*/
int __cdecl
main(int Argc, char **Argv)
{
    parameter_t parameter;
    int retValue = 0;

    FUNC_ENTER();

    WaitForIoCompletionInit();

    do {
        if (processargs(Argc, Argv, &parameter))
        {
            DBG_PRINT((DBG_PREFIX "Error processing command line arguments"));
            retValue = 1;
            break;
        }

        parameter.OsVersion = GetOsVersion();

        if (parameter.OsVersion == WINUNSUPPORTED)
        {
            DBG_PRINT((DBG_PREFIX "This version of Windows is not supported!"));
            printf("Sorry, this version of Windows is not supported!\n");
            retValue = 2;
            break;
        }

        if (parameter.NoExecute)
            break;

        if (!parameter.NoAdminNeeded && !NeededAccessRights())
        {
            DBG_PRINT((DBG_PREFIX "You do not have necessary privileges. " 
                "Please try installing only as administrator."));
            printf("You do not have necessary privileges.\n"
                "Please try installing only as administrator.\n");

            retValue = 3;
            break;
        }

        //
        // execute the command
        //

        if (parameter.CheckInstall)
        {
            retValue = CheckDriver(&parameter);
        }
        else if (parameter.Remove)
        {
            // The driver should be removed

            retValue = RemoveDriver(&parameter);
        }
        else if (parameter.EnumerateParport)
        {
            // The driver should be removed

            retValue = EnumParportDriver(&parameter);
        }
        else if (parameter.Update)
        {
            // Update driver parameters

            retValue = UpdateDriver(&parameter);
        }
        else
        {
            // The driver should be installed

            retValue = InstallDriver(&parameter);
        }
    } while (0);

#if DBG

    if (parameter.OutputDebuggingBuffer)
    {
        CbmOutputDebuggingBuffer();
    }

#endif // #if DBG

    WaitForIoCompletionDeinit();

    FUNC_LEAVE_INT(retValue);
}
