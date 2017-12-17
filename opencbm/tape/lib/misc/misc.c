/*
 *  CBM 1530/1531 tape routines.
 *  Copyright 2012 Arnd Menge, arnd(at)jonnz(dot)de
*/

#include <Windows.h>
#include <stdio.h>

#include "tape.h"

__int32 OutputError(__int32 Status)
{
	switch (Status)
	{
		case Tape_Status_ERROR:
			printf("General error.\n");
			break;
		case Tape_Status_ERROR_Unsupported_Function:
			printf("Device firmware does not recognize USB request ID.\n");
			printf("Please update device firmware and try again.\n");
			break;
		case Tape_Status_ERROR_Device_Disconnected:
			printf("ZoomTape device was disconnected.\n");
			printf("Please disconnect and reconnect ZoomFloppy to continue.\n");
			break;
		case Tape_Status_ERROR_Device_Not_Configured:
			printf("Device not configured for tape operations.\n");
			break;
		case Tape_Status_ERROR_Sense_Not_On_Record:
			printf("Tape drive not on <RECORD>.\n");
			break;
		case Tape_Status_ERROR_Sense_Not_On_Play:
			printf("Tape drive not on <PLAY>.\n");
			break;
		case Tape_Status_ERROR_Write_Interrupted_By_Stop:
			printf("Write interrupted by <STOP>.\n");
			break;
		case Tape_Status_ERROR_RecvBufferEmpty:
			printf("Receive buffer empty. Could not get timestamp.\n");
			break;
		case Tape_Status_ERROR_External_Break:
			printf("External break.\n");
			break;
		case Tape_Status_ERROR_Buffer_Full:
			printf("Device buffer full. Could not store timestamp.\n");
			break;
		case Tape_Status_ERROR_Wrong_Tape_Firmware:
			printf("Incompatible tape firmware version.\n");
			break;
		case Memory_Status_ERROR_Not_Enough_Space:
			printf("Buffer already full.\n");
			break;
		default:
			// printf("Unknown error: %d\n", Status);
			return -1;
	}
	return 0;
}


__int32 OutputInfoError(__int32 Status)
{
	switch (Status)
	{
		case Info_Status_ERROR:
			printf("General error.\n");
			break;
		case Info_Status_ERROR_Unknown_Request:
			printf("Device firmware does not recognize USB request ID.\n");
			printf("Please update device firmware and try again.\n");
			break;
		default:
			// printf("Unknown error: %d\n", Status);
			return -1;
	}
	return 0;
}


__int32 OutputFuncError(__int32 Status)
{
	switch (Status)
	{
		case XUAC_Error_NoTapeSupport:
			printf("No tape support. Update ZoomTape firmware.\n");
			break;
		default:
			// printf("Unknown error: %d\n", Status);
			return -1;
	}
	return 0;
}
