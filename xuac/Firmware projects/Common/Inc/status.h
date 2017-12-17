/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __STATUS_H_
#define __STATUS_H_

// XUAC USB bulk request status values.

// Success -------------------------------------------------------------------

#define XUAC_Status_OK                              1

// Device status values.
#define XUAC_IO_READY                               2

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

// Memory status values
#define Memory_Status_OK                            40
#define Memory_Status_OK_Cleared                    (Memory_Status_OK + 1)
#define Memory_Status_OK_Buffer_Full                (Memory_Status_OK + 2)
#define Memory_Status_OK_Buffer_Not_Full            (Memory_Status_OK + 3)
#define Memory_Status_OK_Buffer_Filled              (Memory_Status_OK + 4)

// Errors --------------------------------------------------------------------

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
//#define Tape_Status_ERROR_usbSendByte             (Tape_Status_ERROR - 7)
#define Tape_Status_ERROR_RecvBufferEmpty           (Tape_Status_ERROR - 8)
#define Tape_Status_ERROR_External_Break            (Tape_Status_ERROR - 9)
#define Tape_Status_ERROR_Buffer_Full               (Tape_Status_ERROR - 10)
//#define Tape_Status_ERROR_Wrong_Tape_Firmware     (Tape_Status_ERROR - 11)

// Board info: status values.
#define Info_Status_ERROR                           220
#define Info_Status_ERROR_Unknown_Request           (Info_Status_ERROR - 1)

// Memory status values
#define Memory_Status_ERROR                         210
#define Memory_Status_ERROR_Not_Enough_Space        (Memory_Status_ERROR - 1)

#endif /* __STATUS_H_ */
