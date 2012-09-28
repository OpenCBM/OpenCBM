/*
 *  CBM 1530/1531 tape routines.
 *  Copyright 2012 Arnd Menge, arnd(at)jonnz(dot)de
*/

#include <opencbm.h>
#include <arch.h>

// Compatible tape firmware version (check tape_153x.c)
#define TapeFirmwareVersion 0x0001

// Tape status values (must match xum1541 firmware values in xum1541.h)
#define Tape_Status_OK                              1
#define Tape_Status_OK_Tape_Device_Present          (Tape_Status_OK + 1)
#define Tape_Status_OK_Tape_Device_Not_Present      (Tape_Status_OK + 2)
#define Tape_Status_OK_Device_Configured_for_Read   (Tape_Status_OK + 3)
#define Tape_Status_OK_Device_Configured_for_Write  (Tape_Status_OK + 4)
#define Tape_Status_OK_Sense_On_Play                (Tape_Status_OK + 5)
#define Tape_Status_OK_Sense_On_Stop                (Tape_Status_OK + 6)
#define Tape_Status_OK_Motor_On                     (Tape_Status_OK + 7)
#define Tape_Status_OK_Motor_Off                    (Tape_Status_OK + 8)
#define Tape_Status_OK_Capture_Finished             (Tape_Status_OK + 9)
#define Tape_Status_OK_Write_Finished               (Tape_Status_OK + 10)
#define Tape_Status_OK_Config_Uploaded              (Tape_Status_OK + 11)
#define Tape_Status_OK_Config_Downloaded            (Tape_Status_OK + 12)
#define Tape_Status_ERROR                           255
#define Tape_Status_ERROR_Device_Disconnected       (Tape_Status_ERROR - 1)
#define Tape_Status_ERROR_Device_Not_Configured     (Tape_Status_ERROR - 2)
#define Tape_Status_ERROR_Sense_Not_On_Record       (Tape_Status_ERROR - 3)
#define Tape_Status_ERROR_Sense_Not_On_Play         (Tape_Status_ERROR - 4)
#define Tape_Status_ERROR_Write_Interrupted_By_Stop (Tape_Status_ERROR - 5)
#define Tape_Status_ERROR_usbSendByte               (Tape_Status_ERROR - 6)
#define Tape_Status_ERROR_usbRecvByte               (Tape_Status_ERROR - 7)
#define Tape_Status_ERROR_External_Break            (Tape_Status_ERROR - 8)
#define Tape_Status_ERROR_Wrong_Tape_Firmware       (Tape_Status_ERROR - 9) // Not returned by firmware.

// Signal edge definitions.
#define XUM1541_TAP_WRITE_STARTFALLEDGE 0x20 // start writing with falling edge (1 = true)
#define XUM1541_TAP_READ_STARTFALLEDGE  0x40 // start reading with falling edge (1 = true)

// Tape/disk mode error return values for xum1541_ioctl, xum1541_read, xum1541_write.
#define XUM1541_Error_NoTapeSupport      -100
#define XUM1541_Error_NoDiskTapeMode     -101
#define XUM1541_Error_TapeCmdInDiskMode  -102
#define XUM1541_Error_DiskCmdInTapeMode  -103


int OutputError(int Status)
{
	switch (Status)
	{
		case Tape_Status_ERROR:
			printf("General error.\n");
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
		case Tape_Status_ERROR_usbSendByte:
			printf("usbSendByte failed.\n");
			break;
		case Tape_Status_ERROR_usbRecvByte:
			printf("usbRecvByte failed.\n");
			break;
		case Tape_Status_ERROR_External_Break:
			printf("External break.\n");
			break;
		case Tape_Status_ERROR_Wrong_Tape_Firmware:
			printf("Wrong tape firmware version.\n");
			break;
		default:
//			printf("Unknown error: %d\n", Status);
			return -1;
	}
	return 0;
}


int OutputFuncError(int Status)
{
	switch (Status)
	{
		case XUM1541_Error_NoTapeSupport:
			printf("No tape support. Update ZoomFloppy firmware.\n");
			break;
		case XUM1541_Error_NoDiskTapeMode:
			printf("ZoomFloppy not correctly initialized.\n");
			printf("Disconnect ZoomFloppy from USB and reconnect.\n");
			break;
		case XUM1541_Error_TapeCmdInDiskMode:
			printf("ZoomTape not detected at ZoomFloppy start.\n");
			printf("Disconnect ZoomFloppy from USB and reconnect.\n");
			break;
		default:
//			printf("Unknown error: %d\n", Status);
			return -1;
	}
	return 0;
}
