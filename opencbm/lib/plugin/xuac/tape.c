/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *  Copyright 2009-2010 Nate Lawson
 *  Copyright 2017 Arnd Menge
 */

/*! ************************************************************** 
** \file lib/plugin/xuac/WINDOWS/tape.c \n
** \author Nate Lawson, Arnd Menge \n
** \n
** \brief Shared library / DLL for accessing the tape driver functions, windows specific code
**
****************************************************************/

#include <stdio.h>
#include <stdlib.h>

#define DBG_USERMODE
#define DBG_PROGNAME "OPENCBM-XUAC.DLL"
#include "debug.h"

#define OPENCBM_PLUGIN
#include "archlib.h"

#include "xuac.h"

/**************** Tape routines below ****************/

/*! \brief TAPE: Prepare capture

 This function is a helper function for tape:
 It prepares the USB device for tape capture.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_prepare_capture(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice, /* *hDev                                  */
        XUAC_TAP_PREPARE_CAPTURE, 0, 0, /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                  /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                  /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_prepare_capture: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Prepare write

 This function is a helper function for tape:
 It prepares the USB device for tape write.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_prepare_write(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice, /* *hDev                                  */
        XUAC_TAP_PREPARE_WRITE, 0, 0,   /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                  /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                  /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_prepare_write: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Get tape sense

 This function is a helper function for tape:
 It returns the current tape sense state.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_get_sense(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice, /* *hDev                                  */
        XUAC_TAP_GET_SENSE, 0, 0,       /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                  /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                  /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_get_sense: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Wait for <STOP> sense

 This function is a helper function for tape:
 It waits until the user stops the tape.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_wait_for_stop_sense(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice,     /* *hDev                                  */
        XUAC_TAP_WAIT_FOR_STOP_SENSE, 0, 0, /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                      /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                      /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                      /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_wait_for_stop_sense: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Wait for <PLAY> sense

 This function is a helper function for tape:
 It waits until the user presses play on tape.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_wait_for_play_sense(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice,     /* *hDev                                  */
        XUAC_TAP_WAIT_FOR_PLAY_SENSE, 0, 0, /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                      /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                      /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                      /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_wait_for_play_sense: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Motor on

 This function is a helper function for tape:
 It turns the tape drive motor on.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_motor_on(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice, /* *hDev                                  */
        XUAC_TAP_MOTOR_ON, 0, 0,        /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                  /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                  /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_motor_on: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Motor off

 This function is a helper function for tape:
 It turns the tape drive motor off.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_motor_off(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice, /* *hDev                                  */
        XUAC_TAP_MOTOR_OFF, 0, 0,       /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                  /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                  /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_motor_off: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Start capture

 This function is a helper function for tape:
 It starts the actual tape capture.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer
   Pointer to a buffer which receives the tape data bytes.

 \param Buffer_Length
   The length of the Buffer.

 \param Status
   Pointer to a variable that receives the return status.

 \param BytesRead
   Pointer to a variable that receives the number of read tape data bytes.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_start_capture(CBM_FILE HandleDevice,
                                 unsigned char *Buffer, unsigned int Buffer_Length,
                                 int *Status,
                                 int *BytesRead)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice,   /* *hDev                                  */
        XUAC_TAP_CAPTURE, 0, 0,           /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                    /* *SendBuffer, SendLength, *BytesWritten */
        Buffer, Buffer_Length, BytesRead, /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                    /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_start_capture: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Start write

 This function is a helper function for tape:
 It starts the actual tape write.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer
   Pointer to a buffer which holds the tape data bytes to be written.

 \param Length
   The number of bytes to write.

 \param Status
   Pointer to a variable that receives the return status.

 \param BytesWritten
   Pointer to a variable that receives the number of written tape data bytes.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_start_write(CBM_FILE HandleDevice,
                               unsigned char *Buffer, unsigned int Length,
                               int *Status,
                               int *BytesWritten)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice, /* *hDev                                  */
        XUAC_TAP_WRITE, 0, Length,      /* cmd, subcmd, cmd_data,                 */
        Buffer, Length, BytesWritten,   /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                  /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_start_write: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Return tape firmware version

 This function is a helper function for tape:
 It returns the tape firmware version.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status and firmware version.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_get_ver(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice, /* *hDev                                  */
        XUAC_TAP_GET_VER, 0, 0,         /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                  /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                  /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_get_ver: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Get information about attached USB device

 This function is a helper function for tape:
 Get technical information about the attached USB device.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param InfoRequest
   The ID of the requested information.

 \param RecvBuffer
   Pointer to a buffer for the data requested from the USB device.
   The pointer must not be NULL.

 \param RecvLength
   The RecvBuffer size, the maximum number of bytes to be received from
   the USB device.

 \param BytesRead
   Pointer to a variable that receives the number of bytes read.
   The pointer must not be NULL.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_board_info(CBM_FILE HandleDevice,
                              unsigned __int32 InfoRequest,
                              unsigned char *RecvBuffer, unsigned int RecvLength,
                              int *BytesRead,
                              int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice,     /* *hDev                                  */
        XUAC_BOARD_INFO, 0, InfoRequest,    /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                      /* *SendBuffer, SendLength, *BytesWritten */
        RecvBuffer, RecvLength, BytesRead,  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                      /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_board_info: returned with error %d", result));
    }
    return result;
}

/*! \brief TAPE: Send tape operations abort command to USB device

 This function is a helper function for tape:
 It sends an abort command to the US device that stops the current operation.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \return
   >=0 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_break(CBM_FILE HandleDevice)
{
    return Device_tap_break((usb_dev_handle *)HandleDevice);
    //printf("opencbm_plugin_tap_break: %x\n", result);
}

/*! \brief Fill data into device buffer

 This function is a helper function for tape:
 It loads the supplied data into the device's transfer buffer.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Buffer
   Pointer to a buffer which holds the tape data bytes to be loaded
   into the device's transfer buffer.

 \param Length
   The number of tape data bytes to load into the device's transfer buffer.

 \param Status
   Pointer to a variable that receives the return status.

 \param BytesWritten
   Pointer to a variable that receives the number of data bytes written into
   the device's transfer buffer.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_buffer_fill(CBM_FILE HandleDevice,
                               unsigned char *Buffer, unsigned int Length,
                               int *Status, int *BytesWritten)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice,             /* *hDev                                  */
        XUAC_MEMORY_TRANSFERBUFFER_FILL, 0, Length, /* cmd, subcmd, cmd_data,                 */
        Buffer, Length, BytesWritten,               /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                              /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                              /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_buffer_fill: returned with error %d", result));
    }
    return result;
}

/*! \brief Check if device buffer is full

 This function is a helper function for tape:
 It checks if the device's transfer buffer is completely filled with data.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_tap_buffer_isfull(CBM_FILE HandleDevice, int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice,          /* *hDev                                  */
        XUAC_MEMORY_TRANSFERBUFFER_ISFULL, 0, 0, /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                           /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                           /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                           /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_tap_buffer_isfull: returned with error %d", result));
    }
    return result;
}

/*! \brief Set the specified device's value

 This function is a helper function for tape:
 Load a supplied value into a specified device's variable.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param ValueID
   ID of the value to be set.

 \param data
   The data to be loaded into the specified value.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_dev_set_value32(CBM_FILE HandleDevice,
                               unsigned char ValueID, unsigned int data,
                               int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice,  /* *hDev                                  */
        XUAC_SET_VALUE32, ValueID, data, /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                   /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                   /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, NULL);                   /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_dev_set_value32: returned with error %d", result));
    }
    return result;
}

/*! \brief Get the specified device's value

 This function is a helper function for tape:
 Read the value of the specified device's variable.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param ValueID
   ID of the value to be read.

 \param data
   Pointer to a variable that receives the data of the specified
   device's variable.

 \param Status
   Pointer to a variable that receives the return status.

 \return
   1 on success.

 If cbm_driver_open() did not succeed, it is illegal to 
 call this function.
*/

int CBMAPIDECL
opencbm_plugin_dev_get_value32(CBM_FILE HandleDevice,
                               unsigned char ValueID, unsigned int *data,
                               int *Status)
{
    int result = Device_comm_bulk(
        (usb_dev_handle *)HandleDevice, /* *hDev                                  */
        XUAC_GET_VALUE32, ValueID, 0,   /* cmd, subcmd, cmd_data,                 */
        NULL, 0, NULL,                  /* *SendBuffer, SendLength, *BytesWritten */
        NULL, 0, NULL,                  /* *RecvBuffer, RecvLength, *BytesRead    */
        Status, data);                  /* *Status, *answer_data                  */
    if (result <= 0) {
        DBG_WARN((DBG_PREFIX "opencbm_plugin_dev_get_value32: returned with error %d", result));
    }
    return result;
}
