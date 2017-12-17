/*
 *  CBM 1530/1531 tape routines.
 *  Copyright 2012-2017 Arnd Menge
*/

// Compatible tape firmware version (check tape_153x.c)
#define TapeFirmwareVersion 0x0001

// Tape application version.
#define APP_VERSION_STRING       "v0.49.0-alpha"

// Tape device SENSE delay to prevent noise.
// Default delay: 10ms.
#define TAPE_SENSE_DELAY_CAPTURE (unsigned __int32) 10
#define TAPE_SENSE_DELAY_WRITE   (unsigned __int32) 10

// Board info: returned request maximum answer size.
#define BOARD_INFO_MAX_SIZE 50

// Device debug level. Check serial port for debug output.
// Default: Output all messages with level<=2.
#define DEV_DEBUG_LEVEL          (unsigned __int32) 2

// XUAC USB bulk request status values: Success ------------------------------

#define XUAC_Status_OK                              1

// Tape status values
#define Tape_Status_OK                              10
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

// Board info: status values.
#define Info_Status_OK                              30
#define Info_Status_OK_Info_Sent                    (Info_Status_OK + 1)

// Buffer related status values.
#define Memory_Status_OK                            40
#define Memory_Status_OK_Cleared                    (Memory_Status_OK + 1)
#define Memory_Status_OK_Buffer_Full                (Memory_Status_OK + 2)
#define Memory_Status_OK_Buffer_Not_Full            (Memory_Status_OK + 3)
#define Memory_Status_OK_Buffer_Filled              (Memory_Status_OK + 4)

// XUAC USB bulk request status values: Errors -------------------------------

#define XUAC_Status_ERROR                           255
#define XUAC_Status_ERROR_Unsupported_Function      (XUAC_Status_ERROR - 1)
#define XUAC_Status_ERROR_Unsupported_Subfunction   (XUAC_Status_ERROR - 2)

// Tape status values
#define Tape_Status_ERROR                           246
#define Tape_Status_ERROR_Unsupported_Function      (Tape_Status_ERROR - 1)
#define Tape_Status_ERROR_Device_Disconnected       (Tape_Status_ERROR - 2)
#define Tape_Status_ERROR_Device_Not_Configured     (Tape_Status_ERROR - 3)
#define Tape_Status_ERROR_Sense_Not_On_Record       (Tape_Status_ERROR - 4)
#define Tape_Status_ERROR_Sense_Not_On_Play         (Tape_Status_ERROR - 5)
#define Tape_Status_ERROR_Write_Interrupted_By_Stop (Tape_Status_ERROR - 6)
//#define Tape_Status_ERROR_usbSendByte               (Tape_Status_ERROR - 7)
#define Tape_Status_ERROR_RecvBufferEmpty           (Tape_Status_ERROR - 8)
#define Tape_Status_ERROR_External_Break            (Tape_Status_ERROR - 9)
#define Tape_Status_ERROR_Buffer_Full               (Tape_Status_ERROR - 10) // +++
#define Tape_Status_ERROR_Wrong_Tape_Firmware       (Tape_Status_ERROR - 11) // Only for user mode applications.

// Board info: status values.
#define Info_Status_ERROR                           220
#define Info_Status_ERROR_Unknown_Request           (Info_Status_ERROR - 1)

// Buffer related status values.
#define Memory_Status_ERROR                         210
#define Memory_Status_ERROR_Not_Enough_Space        (Memory_Status_ERROR - 1)

// XUAC USB bulk subrequest IDs ----------------------------------------------

// Board info: request IDs.
#define INFO_BOARD_NAME                 1
#define INFO_MCU_NAME                   2
#define INFO_MCU_MHZ                    3
#define INFO_FIRMWARE_VERSION           4
#define INFO_FIRMWARE_VERSION_MAJOR     5
#define INFO_FIRMWARE_VERSION_MINOR     6
#define INFO_FIRMWARE_VERSION_SUB       7
#define INFO_TIMER_SPEED_MHZ            8
#define INFO_BUFFER_SIZE_STR            9
#define INFO_BUFFER_SIZE_VAL_STR        10
#define INFO_BOARD_CAPABILITIES         11
#define INFO_SAMPLING_RATE              12

// SetValue32 & GetValue32: request IDs.
#define XUAC_TAP_SET_SENSE_DELAY        1
#define XUAC_TAP_GET_SENSE_DELAY        2
#define XUAC_TAP_SET_FIRST_WRITE_EDGE   3
#define XUAC_TAP_GET_FIRST_WRITE_EDGE   4
#define XUAC_TAP_GET_FIRST_CAPTURE_EDGE 5
#define XUAC_TAP_GET_LOST_COUNT         6
#define XUAC_TAP_GET_DISCARD_COUNT      7
#define XUAC_TAP_GET_OVERCAPTURE_COUNT  8
#define XUAC_MEMORY_GET_BUFFER_COUNT    9
#define XUAC_SET_DBG_LEVEL              10
#define XUAC_GET_DBG_LEVEL              11

// ---------------------------------------------------------------------------

// Tape support error return value.
#define XUAC_Error_NoTapeSupport      -100

// Signal edge type definition.
typedef enum {
	Unspecified = 0,
	FallingEdge = 1,
	RisingEdge  = 2,
} eSignalEdge_t;
