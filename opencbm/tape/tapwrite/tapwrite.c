/*
 *  CBM 1530/1531 tape routines.
 *  Copyright 2012-2017 Arnd Menge
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opencbm.h>
#include <arch.h>
#include "cap.h"
#include "tape.h"
#include "misc.h"

// Global variables
unsigned __int8  CAP_Machine, CAP_Video, CAP_StartEdge, CAP_SignalFormat;
unsigned __int32 CAP_Precision, CAP_SignalWidth, CAP_StartOfs;
unsigned __int32 TimerSpeed, uiBufferSize;
CBM_FILE         fd;

// Break handling variables
CRITICAL_SECTION CritSec_fd, CritSec_BreakHandler;
BOOL             fd_Initialized = FALSE, AbortTapeOps = FALSE;

// Start/stop delay
BOOL             StartDelayActivated = FALSE,
                 StopDelayActivated = FALSE;
unsigned __int32 StartDelay = 0, // Write start delay (replaces first timestamp)
                 StopDelay = 0;  // Motor stop delay after last signal edge was written


void usage(void)
{
	printf("Usage: tapwrite [-aX] [-bY] [-bz] <filename.cap>\n");
	printf("\n");
	printf("  -aX: wait X seconds before writing first signal (optional)\n");
	printf("  -bY: keep on record Y seconds after last signal (optional)\n");
	printf("  -bz: keep on record max time after last signal (optional)\n");
	printf("\n");
	printf("Examples:\n");
	printf("  tapwrite myfile.cap\n");
	printf("  tapwrite -a15 myfile.cap\n");
	printf("  tapwrite -a15 -b30 myfile.cap\n");
	printf("  tapwrite -a15 -bz myfile.cap\n");
}


__int32 EvaluateCommandlineParams(__int32 argc, __int8 *argv[], __int8 filename[_MAX_PATH])
{
	unsigned __int8 bStartDelay = 0, bStopDelay = 0, bMaxStopDelay = 0;

	if ((argc < 2) || (4 < argc))
	{
		printf("Error: invalid number of commandline parameters.\n\n");
		return -1;
	}

	// Evaluate flags.
	while (--argc && (*(++argv)[0] == '-'))
	{
		if ((*argv)[1] == 'a')
		{
			StartDelayActivated = TRUE;
			StartDelay = atoi(&(argv[0][2]));
			bStartDelay++;
		}
		else if (strcmp(*argv,"-bz") == 0)
		{
			StopDelayActivated = TRUE;
			StopDelay = 0xffffffff; // max seconds
			bMaxStopDelay++;
		}
		else if ((*argv)[1] == 'b')
		{
			StopDelay = atoi(&(argv[0][2]));
			if (StopDelay > 0) StopDelayActivated = TRUE;
			bStopDelay++;
		}
		else
		{
			printf("\nError: invalid commandline parameter.\n\n");
			return -1;
		}
	}

	if (bStartDelay == 0)
	{
		printf("* Start delay: not activated\n"); // no start delay
	}
	else if (bStartDelay == 1)
	{
		if (StartDelay == 0)
			printf("* Start delay: minimum / 100us\n", StartDelay);
		else if (StartDelay == 1)
			printf("* Start delay: %u second\n", StartDelay);
		else
			printf("* Start delay: %u seconds\n", StartDelay);
	}
	else if (bStartDelay > 1)
	{
		printf("\nError: Start delay specified more than once.\n\n");
		return -1;
	}

	if ((bMaxStopDelay+bStopDelay) > 1)
	{
		printf("\nError: Stop delay specified more than once.\n\n");
		return -1;
	}
	else if ((bMaxStopDelay+bStopDelay) == 0)
	{
		printf("* Stop delay: not activated\n"); // no stop delay
	}
	else if (bMaxStopDelay == 1)
	{
		printf("* Stop delay: maximum (19 hours) / until <STOP>\n");
	}
	else if (bStopDelay == 1)
	{
		if (StopDelay == 0)
		{
			printf("* Stop delay: not activated\n"); // no stop delay
		}
		else if (StopDelay == 1)
			printf("* Stop delay: %u second\n", StopDelay);
		else
			printf("* Stop delay: %u seconds\n", StopDelay);
	}

	if (strlen(argv[0]) >= _MAX_PATH)
	{
		printf("\nError: Filename too long.\n\n");
		return -1;
	}

	strcpy(filename, argv[0]);

	printf("\n");

	return 0;
}


__int32 AllocateImageBuffer(HANDLE hCAP, void **ppucTapeBuffer, __int32 *piTapeBufferSize)
{
	__int32 FuncRes;

	// Return file size of image file (moves file pointer).
	FuncRes = CAP_GetFileSize(hCAP, piTapeBufferSize);
	if (FuncRes != CAP_Status_OK)
	{
		CAP_OutputError(FuncRes);
		return -1;
	}

	if (*piTapeBufferSize < 0)
	{
		printf("Error: Illegal tape image buffer size.\n");
		return -1;
	}

	// Allocate memory for tape image.
	*ppucTapeBuffer = malloc(*piTapeBufferSize);
	if (*ppucTapeBuffer == NULL)
	{
		printf("Error: Could not allocate memory for capture data.\n");
		return -1;
	}

	// Initialize memory for tape image.
	memset(*ppucTapeBuffer, 0x00, *piTapeBufferSize);

	return 0;
}


// Print tape length to console.
void OutputTapeLength(unsigned __int32 uiTotalTapeTimeSeconds)
{
	unsigned __int32 hours, mins, secs;

	hours = (uiTotalTapeTimeSeconds/3600);
	printf("Tape recording time: %uh", hours);
	mins = ((uiTotalTapeTimeSeconds - hours*3600)/60);
	printf(" %um", mins);
	secs = ((uiTotalTapeTimeSeconds - hours*3600) - mins*60);
	printf(" %us\n\n", secs);
}


// Read tape image into memory.
__int32 ReadCaptureFile(HANDLE hCAP, unsigned __int8 *pucTapeBuffer, __int32 *piCaptureLen)
{
	unsigned __int64 ui64Delta = 0, ShortWarning, ShortError, MinLength, ui64TotalTapeTime = 0;
	unsigned __int32 uiTotalTapeTimeSeconds;
	__int32          FuncRes;
	BOOL             FirstSignal = TRUE;
	unsigned __int32 uiShortSignalWarnings = 0, uiShortSignalErrors = 0;

	// Seek to start of image file and read image header, extract & verify header contents, seek to start of image data.
	FuncRes = CAP_ReadHeader(hCAP);
	if (FuncRes != CAP_Status_OK)
	{
		CAP_OutputError(FuncRes);
		return -1;
	}

	// Get all header entries at once.
	FuncRes = CAP_GetHeader(hCAP, &CAP_Precision, &CAP_Machine, &CAP_Video, &CAP_StartEdge, &CAP_SignalFormat, &CAP_SignalWidth, &CAP_StartOfs);
	if (FuncRes != CAP_Status_OK)
	{
		CAP_OutputError(FuncRes);
		return -1;
	}

	// Init tape data pointer.
	*piCaptureLen = 0;

	// Read timestamps, convert to hardware resolution if necessary.
	while ((FuncRes = CAP_ReadSignal(hCAP, &ui64Delta, NULL)) == CAP_Status_OK)
	{
		if (FirstSignal)
		{
			// Replace first timestamp with start delay if requested
			if (StartDelayActivated)
			{
				if (StartDelay == 0)
					ui64Delta = TimerSpeed*100; // 100us minimum
				else
				{
					ui64Delta = StartDelay;
					ui64Delta *= TimerSpeed*15625;
					ui64Delta <<= 6;
				}
			}
			FirstSignal = FALSE;
		}
		else
		{
			// Convert timestamp (CAP_Precision -> TimerSpeed).
			// No calculation needed if CAP_Precision==TimerSpeed.

			if (CAP_Precision == 1)
			{
				ui64Delta *= TimerSpeed;
			}
			else if (CAP_Precision != TimerSpeed)
			{
				// Effective timestamp width is 39bit unsigned, delta is 64bit unsigned.
				// Hence we can safely multiply:
				ui64Delta *= TimerSpeed;
				ui64Delta += CAP_Precision/2; // Correct rounding.
				ui64Delta /= CAP_Precision;
			}
		}

		ShortWarning = TimerSpeed*40; // 40us
		ShortError = TimerSpeed*25;   // 25us

		if (ui64Delta < ShortWarning)
		{
			if (ui64Delta < ShortError)
			{
				uiShortSignalErrors++;
				ui64Delta = ShortError;
				//printf("Warning - Replaced with minimum signal length.\n");
			}
			else
			{
				uiShortSignalWarnings++;
				//printf("Warning - Short signal (0x%.10X)\n", ui64Delta);
			}
		}

		ui64TotalTapeTime += ui64Delta;

		if (ui64Delta < 0x8000)
		{
			// Short signal.
			(*piCaptureLen) += 2;
		}
		else
		{
			// Long signal.
			(*piCaptureLen) += 5;
			pucTapeBuffer[*piCaptureLen-5] = (unsigned __int8) (((ui64Delta >> 32) & 0x7f) | 0x80); // MSB must be 1.
			pucTapeBuffer[*piCaptureLen-4] = (unsigned __int8)  ((ui64Delta >> 24) & 0xff);
			pucTapeBuffer[*piCaptureLen-3] = (unsigned __int8)  ((ui64Delta >> 16) & 0xff);
		}
		pucTapeBuffer[*piCaptureLen-2] = (unsigned __int8) ((ui64Delta >>  8) & 0xff);
		pucTapeBuffer[*piCaptureLen-1] = (unsigned __int8) (ui64Delta & 0xff);
	}

	if (FuncRes == CAP_Status_Error_Reading_data)
	{
		CAP_OutputError(FuncRes);
		return -1;
	}

	// Add final timestamp for stop delay
	if (StopDelayActivated)
	{
		if (StopDelay == 0xffffffff)
			ui64Delta = 0xffffffffff;
		else
		{
			ui64Delta = StopDelay;
			ui64Delta *= TimerSpeed*15625;
			ui64Delta <<= 6;
		}

		ui64TotalTapeTime += ui64Delta;

		if (ui64Delta < 0x8000)
		{
			// Short signal.
			(*piCaptureLen) += 2;
		}
		else
		{
			// Long signal.
			(*piCaptureLen) += 5;
			pucTapeBuffer[*piCaptureLen-5] = (unsigned __int8) (((ui64Delta >> 32) & 0x7f) | 0x80); // MSB must be 1.
			pucTapeBuffer[*piCaptureLen-4] = (unsigned __int8)  ((ui64Delta >> 24) & 0xff);
			pucTapeBuffer[*piCaptureLen-3] = (unsigned __int8)  ((ui64Delta >> 16) & 0xff);
		}
		pucTapeBuffer[*piCaptureLen-2] = (unsigned __int8) ((ui64Delta >>  8) & 0xff);
		pucTapeBuffer[*piCaptureLen-1] = (unsigned __int8) (ui64Delta & 0xff);
	}

	// Output short signal warnings/errors.
	if (uiShortSignalWarnings > 0)
	{
		printf("Warning - %u short signal%s detected\n",
			uiShortSignalWarnings, (uiShortSignalWarnings == 1) ? "" : "s");
	}
	if (uiShortSignalErrors > 0)
	{
		printf("Warning - %u short signal%s replaced with minimum signal length.\n",
			uiShortSignalErrors, (uiShortSignalErrors == 1) ? "" : "s");
	}
	if ((uiShortSignalWarnings > 0) || (uiShortSignalErrors > 0))
	{
		printf("\n");
	}

	// Calculate tape recording length (rounded down).
	uiTotalTapeTimeSeconds = (unsigned __int32) ((ui64TotalTapeTime >> 6)/15625/TimerSpeed);
	OutputTapeLength(uiTotalTapeTimeSeconds);

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

	// ---- Get buffer size (description string) -------------------------------

	FuncRes = GetBoardInfo(fd, INFO_BUFFER_SIZE_STR, InfoResult, BOARD_INFO_MAX_SIZE, &BytesRead);
	if (FuncRes < 0)
		return -1;

	printf("* Device buffer: %s\n", InfoResult);

	// ---- Get buffer size (value string) -------------------------------------

	FuncRes = GetBoardInfo(fd, INFO_BUFFER_SIZE_VAL_STR, InfoResult, BOARD_INFO_MAX_SIZE, &BytesRead);
	if (FuncRes < 0)
		return -1;

	// Remember buffer size.
	uiBufferSize = atoi(InfoResult);

	// ---- Get timer speed ----------------------------------------------------

	FuncRes = GetBoardInfo(fd, INFO_TIMER_SPEED_MHZ, InfoResult, BOARD_INFO_MAX_SIZE, &BytesRead);
	if (FuncRes < 0)
		return -1;

	// Remember timer speed from the board.
	TimerSpeed = atoi(InfoResult);
	if (TimerSpeed == 0)
	{
		printf("* Timer speed: error (=0)\n");
		return -1;
	}

	printf("* Timer speed: %d MHz\n", TimerSpeed);

	// -------------------------------------------------------------------------

	printf("\n");

	return 0;
}


__int32 WriteTape(CBM_FILE fd, unsigned __int8 *pucTapeBuffer, unsigned __int32 uiCaptureLen)
{
	__int32          Status, BytesRead, BytesWritten, FuncRes;
	unsigned __int8  WriteConfig, WriteConfig2;
	unsigned __int32 WriteConfig3;
	unsigned __int32 uiNumParts, uiBufferPartSize, i;

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	// Check tape firmware version compatibility.
	//   Status value:
	//   - tape firmware version
	//   - XUAC_Error_NoTapeSupport
	FuncRes = cbm_tap_get_ver(fd, &Status);
	if (FuncRes != 1)
	{
		printf("\nReturned error [get_ver]: %d\n", FuncRes);
		return -1;
	}
	if (Status < 0)
	{
		printf("\nReturned error [get_ver]: ");
		if (OutputError(Status) < 0)
			if (OutputFuncError(Status) < 0)
				printf("%d\n", Status);
		return -1;
	}
	if (Status != TapeFirmwareVersion)
	{
		printf("\nError [get_ver]: ");
		OutputError(Tape_Status_ERROR_Wrong_Tape_Firmware);
		return -1;
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	// Set device debug level. Check serial port for debug output.
	FuncRes = dev_set_value32(fd, XUAC_SET_DBG_LEVEL, DEV_DEBUG_LEVEL, &Status);
	if (FuncRes < 0)
	{
		printf("\nReturned error [set_dbg_level]: ");
		if (OutputFuncError(FuncRes) < 0)
			printf("%d\n", FuncRes);
		return -1;
	}
	if (Status != XUAC_Status_OK)
	{
		printf("\nReturned error [set_dbg_level]: ");
		if (OutputError(Status) < 0)
			printf("%d\n", Status);
		return -1;
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	// Set tape device SENSE delay to prevent noise.
	FuncRes = dev_set_value32(fd, XUAC_TAP_SET_SENSE_DELAY, TAPE_SENSE_DELAY_WRITE, &Status);
	if (FuncRes < 0)
	{
		printf("\nReturned error [set_sense_delay]: ");
		if (OutputFuncError(FuncRes) < 0)
			printf("%d\n", FuncRes);
		return -1;
	}
	if (Status != XUAC_Status_OK)
	{
		printf("\nReturned error [set_sense_delay]: ");
		if (OutputError(Status) < 0)
			printf("%d\n", Status);
		return -1;
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	// Prepare tape write configuration.
	// Write signal is inverted read signal.
	WriteConfig = (unsigned __int8)
		((CAP_StartEdge == CAP_StartEdge_Falling) ? RisingEdge : FallingEdge);

	// Upload tape write configuration.
	FuncRes = dev_set_value32(fd, XUAC_TAP_SET_FIRST_WRITE_EDGE, WriteConfig, &Status);
	if (FuncRes < 0)
	{
		printf("\nReturned error [set_first_write_edge]: ");
		if (OutputFuncError(FuncRes) < 0)
			printf("%d\n", FuncRes);
		return -1;
	}
	if (Status != XUAC_Status_OK)
	{
		printf("\nReturned error [set_first_write_edge]: ");
		if (OutputError(Status) < 0)
			printf("%d\n", Status);
		return -1;
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	// Download tape write configuration and check.
	FuncRes = dev_get_value32(fd, XUAC_TAP_GET_FIRST_WRITE_EDGE, &WriteConfig3, &Status);
	if (FuncRes < 0)
	{
		printf("\nReturned error [get_first_write_edge]: ");
		if (OutputFuncError(FuncRes) < 0)
			printf("%d\n", FuncRes);
		return -1;
	}
	if (Status != XUAC_Status_OK)
	{
		printf("\nReturned error [get_first_write_edge]: ");
		if (OutputError(Status) < 0)
			printf("%d\n", Status);
		return -1;
	}
	if (WriteConfig != (unsigned __int8) WriteConfig3)
	{
		printf("\nError [get_first_write_edge]: Configuration mismatch.\n");
		return -1;
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	// Prepare tape firmware for write.
	//   Status values:
	//   - Tape_Status_OK_Device_Configured_for_Write
	//   - Tape_Status_ERROR_Device_Disconnected
	//   - Tape_Status_ERROR_Not_In_Tape_Mode
	//   FuncRes values concerning tape mode:
	//   - XUAC_Error_NoTapeSupport
	FuncRes = cbm_tap_prepare_write(fd, &Status);
	if (FuncRes < 0)
	{
		printf("\nReturned error [prepare_write]: ");
		if (OutputFuncError(FuncRes) < 0)
			printf("%d\n", FuncRes);
		return -1;
	}
	if (Status != Tape_Status_OK_Device_Configured_for_Write)
	{
		printf("\nReturned error [prepare_write]: ");
		if (OutputError(Status) < 0)
			printf("%d\n", Status);
		return -1;
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	// Cache tape data in device buffer if possible.
	// Some devices may not have a buffer for USB transfers.
	if (uiBufferSize > 0)
	{

		// Split buffer size into 'uiNumParts' parts.
		uiNumParts = 16;
		uiBufferPartSize = uiBufferSize/uiNumParts;

		// Print status line.
		printf("Buffering tape data [");
		for (i = 0; i < uiNumParts; i++) { printf(" "); }
		printf("]");
		for (i = 0; i < uiNumParts+1; i++) { printf("\b"); }

		// Send buffer parts.
		for (i = 0; i < uiNumParts; i++)
		{
			FuncRes = cbm_tap_buffer_fill(fd, pucTapeBuffer, uiBufferPartSize, &Status, &BytesWritten);
			if (FuncRes < 0)
			{
				printf("\nReturned error [fill_buffer]: ");
				if (OutputFuncError(FuncRes) < 0)
					printf("%d\n", FuncRes);
				return -1;
			}
			if (Status != Memory_Status_OK_Buffer_Filled)
			{
				printf("\nReturned error [fill_buffer]: ");
				if (OutputError(Status) < 0)
					printf("%d\n", Status);
				return -1;
			}
			if (uiBufferPartSize != BytesWritten)
			{
				printf("\nError [fill_buffer]: Not enough buffered.\n");
				return -1;
			}

			// Update status line.
			printf("*");

			// Update tape data pointer & remaining amount.
			pucTapeBuffer += uiBufferPartSize;
			uiCaptureLen -= uiBufferPartSize;

			// Check abort flag.
			if (AbortTapeOps)
				return -1;
		}

		// ---------------------------------------------------------------------

		// Make sure buffer is full.
		FuncRes = cbm_tap_buffer_isfull(fd, &Status);
		if (FuncRes != 1)
		{
			printf("\nReturned error [buffer_isfull]: %d\n", FuncRes);
			return -1;
		}
		if (Status != Memory_Status_OK_Buffer_Full)
		{
			printf("\nReturned error [buffer_isfull]: ");
			if (OutputError(Status) < 0)
				if (OutputFuncError(Status) < 0)
					printf("%d\n", Status);
			return -1;
		}

		// ---------------------------------------------------------------------

		// Update status line.
		printf("] ok\n\n");

	} // if (uiBufferSize > 0)

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	// Get tape SENSE state.
	//   Status values:
	//   - Tape_Status_OK_Sense_On_Play
	//   - Tape_Status_OK_Sense_On_Stop
	//   - Tape_Status_ERROR_Device_Not_Configured
	//   - Tape_Status_ERROR_Device_Disconnected
	//   - Tape_Status_ERROR_Not_In_Tape_Mode
	//   - XUAC_Error_NoTapeSupport
	FuncRes = cbm_tap_get_sense(fd, &Status);
	if (FuncRes != 1)
	{
		printf("\nReturned error [get_sense]: %d\n", FuncRes);
		return -1;
	}
	if ((Status != Tape_Status_OK_Sense_On_Play) && (Status != Tape_Status_OK_Sense_On_Stop))
	{
		printf("\nReturned error [get_sense]: ");
		if (OutputError(Status) < 0)
			if (OutputFuncError(Status) < 0)
				printf("%d\n", Status);
		return -1;
	}

	// -------------------------------------------------------------------------

	if (Status == Tape_Status_OK_Sense_On_Play)
	{
		// Check abort flag.
		if (AbortTapeOps)
			return -1;

		printf("Please <STOP> the tape.\n");

		//   Status values:
		//   - Tape_Status_OK_Sense_On_Stop
		//   - Tape_Status_ERROR_Device_Not_Configured
		//   - Tape_Status_ERROR_Device_Disconnected
		//   - Tape_Status_ERROR_Not_In_Tape_Mode
		//   - XUAC_Error_NoTapeSupport
		FuncRes = cbm_tap_wait_for_stop_sense(fd, &Status);
		if (FuncRes != 1)
		{
			printf("\nReturned error [wait_for_stop_sense]: %d\n", FuncRes);
			return -1;
		}
		if (Status != Tape_Status_OK_Sense_On_Stop)
		{
			printf("\nReturned error [wait_for_stop_sense]: ");
			if (OutputError(Status) < 0)
				if (OutputFuncError(Status) < 0)
					printf("%d\n", Status);
			return -1;
		}
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	printf("Press <RECORD> and <PLAY> on tape.\n");

	// -------------------------------------------------------------------------

	//   Status values:
	//   - Tape_Status_OK_Sense_On_Play
	//   - Tape_Status_ERROR_Device_Not_Configured
	//   - Tape_Status_ERROR_Device_Disconnected
	//   - Tape_Status_ERROR_Not_In_Tape_Mode
	//   - XUAC_Error_NoTapeSupport
	FuncRes = cbm_tap_wait_for_play_sense(fd, &Status);
	if (FuncRes != 1)
	{
		printf("\nReturned error [wait_for_play_sense]: %d\n", FuncRes);
		return -1;
	}
	if (Status != Tape_Status_OK_Sense_On_Play)
	{
		printf("\nReturned error [wait_for_play_sense]: ");
		if (OutputError(Status) < 0)
			if (OutputFuncError(Status) < 0)
				printf("%d\n", Status);
		return -1;
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	printf("\nWriting tape...\n");

	// -------------------------------------------------------------------------

	//   Status values:
	//   - Tape_Status_OK_Write_Finished
	//   - Tape_Status_ERROR_Write_Interrupted_By_Stop
	//   - Tape_Status_ERROR_Sense_Not_On_Record
	//   - Tape_Status_ERROR_Device_Not_Configured
	//   - Tape_Status_ERROR_Device_Disconnected
	//   FuncRes values concerning tape mode:
	//   - XUAC_Error_NoTapeSupport
	FuncRes = cbm_tap_start_write(fd, pucTapeBuffer, uiCaptureLen, &Status, &BytesWritten);
	if (FuncRes < 0)
	{
		printf("\nReturned error [write]: ");
		if (OutputFuncError(FuncRes) < 0)
			printf("%d\n", FuncRes);
		return -1;
	}
	if (Status != Tape_Status_OK_Write_Finished)
	{
		printf("\nReturned error [write]: ");
		if (OutputError(Status) < 0)
			printf("%d\n", Status);
		return -1;
	}
	if (uiCaptureLen != BytesWritten)
	{
		printf("\nError [write]: Short write.\n");
		return -1;
	}

	// -------------------------------------------------------------------------

	// Check abort flag.
	if (AbortTapeOps)
		return -1;

	// -------------------------------------------------------------------------

	printf("\nWriting finished OK.\n");

	return 0;
}


// Break handler.
// Initialized by SetConsoleCtrlHandler() after critical sections initialized.
BOOL BreakHandler(DWORD fdwCtrlType)
{
	if (fdwCtrlType == CTRL_C_EVENT)
	{
		printf("\nAborting...\n");
		AbortTapeOps = TRUE; // Flag tape ops abort.

		if (!TryEnterCriticalSection(&CritSec_BreakHandler))
			return TRUE; // Already aborting.

		EnterCriticalSection(&CritSec_fd); // Acquire handle flag access.

		if (fd_Initialized)
		{
			cbm_tap_break(fd); // Handle valid.
			LeaveCriticalSection(&CritSec_fd); // Release handle flag access.
			return TRUE;
		}

		LeaveCriticalSection(&CritSec_fd); // Release handle flag access.
	}
	return FALSE;
}


// Main routine.
//   Return values:
//    0: tape writing finished ok
//   -1: an error occurred
int ARCH_MAINDECL main(int argc, char *argv[])
{
	HANDLE          hCAP;
	unsigned __int8 *pucTapeBuffer = NULL;
	__int8          filename[_MAX_PATH];
	__int32         iCaptureLen = 0, iTapeBufferSize;
	__int32         FuncRes, RetVal = -1;

	printf("\ntapwrite %s -- Commodore 1530/1531 tape mastering software\n", APP_VERSION_STRING);
	printf("Copyright 2012-2017 Arnd Menge\n\n");

	InitializeCriticalSection(&CritSec_fd);
	InitializeCriticalSection(&CritSec_BreakHandler);

	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) BreakHandler, TRUE))
		printf("Ctrl-C break handler not installed.\n\n");

	// Set defaults.
	TimerSpeed = 0;

	do {

		if (EvaluateCommandlineParams(argc, argv, filename) == -1)
		{
			usage();
			break;
		}

		EnterCriticalSection(&CritSec_fd); // Acquire handle flag access.

		if (cbm_driver_open_ex(&fd, NULL) != 0)
		{
			printf("Driver error.\n");
			LeaveCriticalSection(&CritSec_fd);
			break;
		}

		fd_Initialized = TRUE;
		LeaveCriticalSection(&CritSec_fd); // Release handle flag access.

		// Print some hardware info about the attached board.
		PrintBoardInfo(fd);

		// Open specified image file for reading.
		FuncRes = CAP_OpenFile(&hCAP, filename);
		if (FuncRes != CAP_Status_OK)
		{
			CAP_OutputError(FuncRes);
			break;
		}

		// Allocate memory for tape image.
		if (AllocateImageBuffer(hCAP, &pucTapeBuffer, &iTapeBufferSize) == -1)
		{
			CAP_CloseFile(&hCAP);
			break;
		}

		// Read tape image into memory.
		RetVal = ReadCaptureFile(hCAP, pucTapeBuffer, &iCaptureLen);

		CAP_CloseFile(&hCAP);

		if (RetVal == -1)
			break;

		RetVal = WriteTape(fd, pucTapeBuffer, iCaptureLen);

	} while(0);

	EnterCriticalSection(&CritSec_fd); // Acquire handle flag access.
	if (fd_Initialized)
	{
		cbm_driver_close(fd);
		fd_Initialized = FALSE;
	}
	LeaveCriticalSection(&CritSec_fd); // Release handle flag access.

	DeleteCriticalSection(&CritSec_fd);
	DeleteCriticalSection(&CritSec_BreakHandler);

   	if (pucTapeBuffer != NULL) free(pucTapeBuffer);

   	printf("\n");

   	return RetVal;
}
