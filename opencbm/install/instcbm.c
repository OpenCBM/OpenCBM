/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2004 Spiro Trikaliotis <cbm4win@trikaliotis.net>
 *
 */

/*! ************************************************************** 
** \file instcbm.c \n
** \author Spiro Trikaliotis \n
** \version $Id: instcbm.c,v 1.2 2004-11-15 16:11:52 strik Exp $ \n
** \n
** \brief Program to install and uninstall the OPENCBM driver
**
****************************************************************/


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#include "instcbm.h"
#include "cbmioctl.h"

#include <getopt.h>

/*! Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/*! The name of the executable */
#define DBG_PROGNAME "INSTCBM.EXE"

/*! This file is "like" debug.c, that is, define some variables */
#define DBG_IS_DEBUG_C

#include "debug.h"


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

/*
        DBG_PRINT((DBG_PREFIX "OS VERSION: %u.%u.%u %s (%s)",
            ovi.dwMajorVersion, ovi.dwMinorVersion, ovi.dwBuildNumber,
            platform, ovi.szCSDVersion));
*/
    }

    FUNC_LEAVE_TYPE(retValue, osversion_t, "%u");
}

/*! \brief \internal Print out the help screen */
static VOID
usage(VOID)
{
    FUNC_ENTER();

    printf("Usage: instcbm [options]\n"
            "Install the cbm4win driver on the system, or remove it.\n"
            "\n"
            "Options:\n"
            "  -h, --help      display this help and exit\n"
            "  -r, --remove    remove (uninstall) the driver\n"
            "  -u, --update    update parameters if driver is already installed.\n"
            "  -l, --lpt=no    set default LPT port\n"
            "  -n, --nocopy    do not copy the driver files into the system directory\n"
            "  -c, --check     only check if the installation is ok\n"
            "  -F, --forcent4  force NT4 driver on a Win 2000, XP, or newer systems\n" 
            "                  (NOT RECOMMENDED!)\n"
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

    /*! Find out of more than one "execute" parameter is given */
    BOOL ExecuteParameterGiven;

    /*! --remove was given */
    BOOL Remove;

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

#if DBG

    /*! --debugflags was given */
    BOOL DebugFlagsDriverWereGiven;

    /*! --debugflags, a second parameter (for the DLL) was given */
    BOOL DebugFlagsDllWereGiven;

    /*! if --debugflags was given: the number which was there */
    ULONG DebugFlagsDriver;

    /*! if --debugflags was given: the number which was there */
    ULONG DebugFlagsDll;

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
        { "remove",     no_argument,       NULL, 'r' },
        { "forcent4",   no_argument,       NULL, 'F' },
        { "lpt",        required_argument, NULL, 'l' },
        { "update",     no_argument,       NULL, 'u' },
        { "check",      no_argument,       NULL, 'c' },
#if DBG
        { "debugflags", required_argument, NULL, 'D' },
#endif // #if DBG
        { "nocopy",     no_argument,       NULL, 'n' },
        { NULL,         0,                 NULL, 0   }
    };

    const char shortopts[] ="hrFl:nuD:c";

    FUNC_ENTER();

    error = FALSE;

    // Clear the Parameter set 

    DBG_ASSERT(Parameter);
    memset(Parameter, 0, sizeof(*Parameter));

    // We have no specified LPT port

    Parameter->Lpt = (ULONG) -1;

#if DBG

    // Until now, now DebugFlags were on the line

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

        case 'n':
            Parameter->NoCopy = TRUE;
            break;

        case 'c':
            Parameter->CheckInstall = TRUE;
            break;

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
                    error = processNumber(next, NULL, &Parameter->DebugFlagsDllWereGiven,
                        &Parameter->DebugFlagsDll);
                }
            }
            break;

#endif // #if DBG

        case 'u':
            Parameter->Update = TRUE;
            break;

        default:
            error = TRUE;
            hint(Argv[0]);
            break;
        }
    }

    FUNC_LEAVE_BOOL(error);
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
    FUNC_ENTER();

    UNREFERENCED_PARAMETER(Parameter);

    if (CbmCheckPresence(OPENCBM_DRIVERNAME))
    {
        char driverSystemPath[MAX_PATH];
        int driverSystemLen;

        printf("REMOVING driver...\n");

        CbmRemove(OPENCBM_DRIVERNAME);

        // Now, check if we copied the driver and the DLL into the
        // system directory. If this is the case, delete them from
        // there.

        GetSystemDirectory(driverSystemPath, sizeof(driverSystemPath));

        // Remember the length of the system path

        driverSystemLen = strlen(driverSystemPath);

        // try to delete opencbm.dll

        strcpy(&driverSystemPath[driverSystemLen ++], "\\OPENCBM.DLL");
        DBG_PRINT((DBG_PREFIX "Trying to delete %s", driverSystemPath));
        DeleteFile(driverSystemPath);

        strcpy(&driverSystemPath[driverSystemLen], "DRIVERS\\");

        // Remember the new length of the system driver path

        driverSystemLen = strlen(driverSystemPath);

        // try to delete cbm4nt.sys

        strcpy(&driverSystemPath[driverSystemLen], "CBM4NT.SYS");
        DBG_PRINT((DBG_PREFIX "Trying to delete %s", driverSystemPath));
        DeleteFile(driverSystemPath);

        // try to delete cbm4wdm.sys

        strcpy(&driverSystemPath[driverSystemLen], "CBM4WDM.SYS");
        DBG_PRINT((DBG_PREFIX "Trying to delete %s", driverSystemPath));
        DeleteFile(driverSystemPath);
    }
    else
    {
        printf("No driver installed, cannot remove.\n");
    }

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

    if (CbmCheckCorrectInstallation())
    {
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
             Port:               automatic (0), actually using LPT 1
             IRQ mode:           enabled
             Driver start mode:  manually (3)
        */
    }

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
    int error;

    FUNC_ENTER();

    error = 0;

    if (CbmCheckPresence(OPENCBM_DRIVERNAME))
    {
        if (!Parameter->Update)
        {
            printf("Driver is already installed, remove it before you try a new installation,\n"
                "or use the --update option!\n");
            error = 8;
        }
    }
    else
    {
        char *workingDirectory;

        printf("Installing driver...\n");

        //! \todo Replace with GetCurrentDirectory()

        workingDirectory = _getcwd(NULL,1);

        if (!workingDirectory)
        {
            DBG_PRINT((DBG_PREFIX "Could not determine the current working directory!"));
            printf("Could not determine the current working directory!\n");
            error = 4;
        }
        else
        {
            char driverSystemPath[MAX_PATH];
            char driverLocalPath[MAX_PATH];

//          printf("Current working directory is '%s'\n", workingDirectory);

            GetSystemDirectory(driverSystemPath, sizeof(driverSystemPath));

            if (!driverLocalPath)
            {
                DBG_PRINT((DBG_PREFIX "Could not allocate memory for driver path, exiting!"));
                printf("Could not allocate memory for driver path, exiting!\n");
                error = 5;
            }
            else
            {
                const char *driverToUse;
                BOOL useWdm;
                int driverSystemLen;
                int driverLocalLen;

                strcpy(driverLocalPath, workingDirectory);
                strcat(driverLocalPath, "\\");
                driverLocalLen = strlen(driverLocalPath);

                driverSystemLen = strlen(driverSystemPath);
                strcpy(&driverSystemPath[driverSystemLen ++], "\\DRIVERS\\");

                if (Parameter->OsVersion > WINNT4)
                {
                    useWdm = TRUE;

                    if (Parameter->ForceNt4)
                    {
                        useWdm = FALSE;
                    }
                }
                else
                {
                    useWdm = FALSE;
                }

                driverToUse = useWdm ? "cbm4wdm.sys" : "cbm4nt.sys";

                strcpy(&driverLocalPath[driverLocalLen], driverToUse);
                strcat(&driverSystemPath[driverSystemLen], driverToUse);

                printf("Using driver '%s'\n", driverLocalPath);

                if (Parameter->NoCopy)
                {
                    CbmInstall(OPENCBM_DRIVERNAME, driverLocalPath);
                }
                else
                {
                    // copy the driver into the appropriate directory

                    printf("Copying '%s' to '%s'", driverLocalPath, driverSystemPath);
                    if (!CopyFile(driverLocalPath, driverSystemPath, FALSE))
                    {
                        error = 6;
                        printf(" FAILED!\n");
                    }
                    else
                    {
                        strcpy(&driverLocalPath[driverLocalLen], "opencbm.dll");
                        strcpy(&driverSystemPath[driverSystemLen], "opencbm.dll");
                        printf("\nCopying '%s' to '%s'", driverLocalPath, driverSystemPath);
                        if (!CopyFile(driverLocalPath, driverSystemPath, FALSE))
                        {
                            error = 7;
                            printf(" FAILED!\n");
                        }
                        else
                        {
                            printf("\n");
                            strcpy(driverSystemPath, "System32\\DRIVERS\\");
                            strcat(driverSystemPath, driverToUse);
                            CbmInstall(OPENCBM_DRIVERNAME, driverSystemPath);
                        }
                    }

                }
            }

            free(workingDirectory);
        }
    }

    if (error == 0)
    {
        if (!CbmUpdateParameter(Parameter->Lpt,
#if DBG
            Parameter->DebugFlagsDriverWereGiven, Parameter->DebugFlagsDriver,
            Parameter->DebugFlagsDllWereGiven, Parameter->DebugFlagsDll
#else
            0, 0, 0, 0
#endif // #if DBG
            ))
        {
            error = 9;
        }
    }

    if (error == 0)
    {
        printf("\n");
        error = CheckDriver(Parameter);
    }

    FUNC_LEAVE_INT(error);
}

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
    int retValue;

    FUNC_ENTER();

    if (processargs(Argc, Argv, &parameter))
    {
        DBG_PRINT((DBG_PREFIX "Error processing command line arguments"));
        retValue = 1;
    }
    else
    {
        retValue = 0;
    }

    if (retValue == 0)
    {
        parameter.OsVersion = GetOsVersion();

        if (parameter.OsVersion == WINUNSUPPORTED)
        {
            DBG_PRINT((DBG_PREFIX "This version of Windows is not supported!"));
            printf("Sorry, this version of Windows is not supported!\n");
            retValue = 2;
        }
    }

    if (!parameter.NoExecute)
    {
        if (retValue == 0)
        {
            if (!NeededAccessRights())
            {
                retValue = 3;
                DBG_PRINT((DBG_PREFIX "You do not have necessary privileges. Please try as administrator."));
                printf("You do not have necessary privileges. Please try as administrator.\n");
            }
        }

        if (retValue == 0)
        {
            // now, execute the command

            if (parameter.CheckInstall)
            {
                retValue = CheckDriver(&parameter);
            }
            else if (parameter.Remove)
            {
                // The driver should be removed

                retValue = RemoveDriver(&parameter);
            }
            else
            {
                // The driver should be installed

                retValue = InstallDriver(&parameter);
            }
        }
    }

    FUNC_LEAVE_INT(retValue);
}
