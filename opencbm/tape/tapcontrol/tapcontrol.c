/*
 *  CBM 1530/1531 tape routines.
 *  Copyright 2012-2017 Arnd Menge
*/

#include <stdio.h>
#include <stdlib.h>

#include <opencbm.h>
#include <arch.h>
#include "tape.h"
#include "misc.h"

// Global variables
unsigned __int8 MotorOn  = 0,
                MotorOff = 0;


void usage(void)
{
	printf("Usage: tapcontrol <command>\n");
	printf("\n");
	printf("Available commands:\n\n");
	printf("  on : Turn tape drive motor on\n");
	printf("  off: Turn tape drive motor off\n");
	printf("\n");
	printf("Examples:\n");
	printf("  tapcontrol on\n");
	printf("  tapcontrol off\n");
}


__int32 EvaluateCommandlineParams(__int32 argc, __int8 *argv[])
{
	if (argc != 2)
		return -1;

	argv++;

	// Evaluate commandline flag.
	if (strcmp(*argv,"on") == 0)
	{
	   	printf("* Turning tape motor on.\n");
	   	MotorOn = 1;
	}
	else if (strcmp(*argv,"off") == 0)
	{
	   	printf("* Turning tape motor off.\n");
	   	MotorOff = 1;
	}
	else
		return -1;

	printf("\n");

	return 0;
}


// Get the specified hardware info from the attached board.
__int32 GetBoardInfo(CBM_FILE fd,
                     unsigned __int32 InfoRequest,
                     unsigned __int8 *InfoResult,
                     unsigned __int32 InfoResultSize,
                     __int32 *Length)
{
	__int32         Status, BytesRead, BytesWritten, FuncRes;

	FuncRes = cbm_tap_board_info(fd, InfoRequest, InfoResult, BOARD_INFO_MAX_SIZE, &BytesRead, &Status);
	if (FuncRes < 0)
	{
		printf("\nReturned error [board_info]: ");
		if (OutputFuncError(FuncRes) < 0)
			printf("%d\n", FuncRes);
		return -1;
	}
	if (Status != Info_Status_OK_Info_Sent)
	{
		printf("\nReturned error [board_info]: ");
		if (OutputInfoError(Status) < 0)
			printf("%d\n", Status);
		return -1;
	}

	// Add zero termination to the returned string.
	InfoResult[BytesRead] = '\0';

	return Status;
}


// Get some hardware info from the attached board.
__int32 PrintBoardInfo(CBM_FILE fd)
{
	unsigned __int8 InfoResult[BOARD_INFO_MAX_SIZE+1], InfoResult2[BOARD_INFO_MAX_SIZE+1];
	__int32         Status, BytesRead, BytesWritten, FuncRes;

	// ---- Get board name -----------------------------------------------------

	FuncRes = GetBoardInfo(fd, INFO_BOARD_NAME, InfoResult, BOARD_INFO_MAX_SIZE, &BytesRead);
	if (FuncRes < 0)
		return -1;

	printf("* Board: %s\n", InfoResult);

	// ---- Get MCU name & speed -----------------------------------------------

	FuncRes = GetBoardInfo(fd, INFO_MCU_NAME, InfoResult, BOARD_INFO_MAX_SIZE, &BytesRead);
	if (FuncRes < 0)
		return -1;

	FuncRes = GetBoardInfo(fd, INFO_MCU_MHZ, InfoResult2, BOARD_INFO_MAX_SIZE, &BytesRead);
	if (FuncRes < 0)
		return -1;

	printf("* MCU: %s @ %s MHz\n", InfoResult, InfoResult2);

	// ---- Get firmware version -----------------------------------------------

	FuncRes = GetBoardInfo(fd, INFO_FIRMWARE_VERSION, InfoResult, BOARD_INFO_MAX_SIZE, &BytesRead);
	if (FuncRes < 0)
		return -1;

	printf("* Firmware: %s\n", InfoResult);

	// -------------------------------------------------------------------------

	printf("\n");

	return 0;
}


int ARCH_MAINDECL main(int argc, char *argv[])
{
	CBM_FILE fd;
	__int32  Status, FuncRes, RetVal = 0;
	BOOL     fd_Initialized = FALSE;

	printf("\ntapcontrol %s -- Commodore 1530/1531 tape control\n", APP_VERSION_STRING);
	printf("Copyright 2012-2017 Arnd Menge\n\n");

	do {

		// ---------------------------------------------------------------------

		if ((RetVal = EvaluateCommandlineParams(argc, argv)) == -1)
		{
			usage();
			break;
		}

		// ---------------------------------------------------------------------

		if (cbm_driver_open_ex(&fd, NULL) != 0)
		{
			printf("\nDriver error.\n");
			RetVal = -1;
			break;
		}

		fd_Initialized = TRUE;

		// ---------------------------------------------------------------------

		// Check tape firmware version compatibility.
		//   Status value:
		//   - tape firmware version
		//   - XUAC_Error_NoTapeSupport
		FuncRes = cbm_tap_get_ver(fd, &Status);
		if (FuncRes != 1)
		{
			printf("\nReturned error [get_ver]: %d\n", FuncRes);
			RetVal = -1;
			break;
		}
		if (Status < 0)
		{
			printf("\nReturned error [get_ver]: ");
			if (OutputError(Status) < 0)
				if (OutputFuncError(Status) < 0)
					printf("%d\n", Status);
			RetVal = -1;
			break;
		}
		if (Status != TapeFirmwareVersion)
		{
			printf("\nError [get_ver]: ");
			OutputError(Tape_Status_ERROR_Wrong_Tape_Firmware);
			RetVal = -1;
			break;
		}

		// ---------------------------------------------------------------------

		// Print some hardware info about the attached board.
		PrintBoardInfo(fd);

		// ---------------------------------------------------------------------

		if (MotorOn == 1)
		{
			//   Status values:
			//   - Tape_Status_OK_Motor_On
			//   - Tape_Status_ERROR_Device_Disconnected
			//   - XUAC_Error_NoTapeSupport
			FuncRes = cbm_tap_motor_on(fd, &Status);
			if (FuncRes != 1)
			{
				printf("\nReturned error [motor_on]: %d\n", FuncRes);
				RetVal = -1;
			}
			if (Status != Tape_Status_OK_Motor_On)
			{
				printf("\nReturned error [motor_on]: ");
				if (OutputError(Status) < 0)
					if (OutputFuncError(Status) < 0)
						printf("%d\n", Status);
				RetVal = -1;
			}
		}

		// ---------------------------------------------------------------------

		if (MotorOff == 1)
		{
			//   Status values:
			//   - Tape_Status_OK_Motor_Off
			//   - Tape_Status_ERROR_Device_Disconnected
			//   - XUAC_Error_NoTapeSupport
			FuncRes = cbm_tap_motor_off(fd, &Status);
			if (FuncRes != 1)
			{
				printf("\nReturned error [motor_off]: %d\n", FuncRes);
				RetVal = -1;
			}
			if (Status != Tape_Status_OK_Motor_Off)
			{
				printf("\nReturned error [motor_off]: ");
				if (OutputError(Status) < 0)
					if (OutputFuncError(Status) < 0)
						printf("%d\n", Status);
				RetVal = -1;
			}
		}

		// ---------------------------------------------------------------------

	} while (0);

	if (fd_Initialized)
	{
		cbm_driver_close(fd);
		fd_Initialized = FALSE;
	}

   	printf("\n");
   	return RetVal;
}
