/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __REQUESTS_H_
#define __REQUESTS_H_

// USB bulk request IDs for misc operations.
#define XUAC_MISC                                   80
#define XUAC_BOARD_INFO                             (XUAC_MISC + 0)
#define XUAC_MEMORY_TRANSFERBUFFER_CLEAR            (XUAC_MISC + 1)
#define XUAC_MEMORY_TRANSFERBUFFER_ISFULL           (XUAC_MISC + 2)
#define XUAC_MEMORY_TRANSFERBUFFER_FILL             (XUAC_MISC + 3)
#define XUAC_SET_VALUE32                            (XUAC_MISC + 4)
#define XUAC_GET_VALUE32                            (XUAC_MISC + 5)

// USB bulk request IDs for tape operations.
#define XUAC_TAP                                    100
#define XUAC_TAP_MOTOR_ON                           (XUAC_TAP + 0)
#define XUAC_TAP_GET_VER                            (XUAC_TAP + 1)
#define XUAC_TAP_PREPARE_CAPTURE                    (XUAC_TAP + 2)
#define XUAC_TAP_PREPARE_WRITE                      (XUAC_TAP + 3)
#define XUAC_TAP_GET_SENSE                          (XUAC_TAP + 4)
#define XUAC_TAP_WAIT_FOR_STOP_SENSE                (XUAC_TAP + 5)
#define XUAC_TAP_WAIT_FOR_PLAY_SENSE                (XUAC_TAP + 6)
#define XUAC_TAP_CAPTURE                            (XUAC_TAP + 7)
#define XUAC_TAP_WRITE                              (XUAC_TAP + 8)
#define XUAC_TAP_MOTOR_OFF                          (XUAC_TAP + 9)

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
#define XUAC_TAP_SET_SENSE_DELAY        (uint8_t) 1
#define XUAC_TAP_GET_SENSE_DELAY        (uint8_t) 2
#define XUAC_TAP_SET_FIRST_WRITE_EDGE   (uint8_t) 3
#define XUAC_TAP_GET_FIRST_WRITE_EDGE   (uint8_t) 4
#define XUAC_TAP_GET_FIRST_CAPTURE_EDGE (uint8_t) 5
#define XUAC_TAP_GET_LOST_COUNT         (uint8_t) 6
#define XUAC_TAP_GET_DISCARD_COUNT      (uint8_t) 7
#define XUAC_TAP_GET_OVERCAPTURE_COUNT  (uint8_t) 8
#define XUAC_MEMORY_GET_BUFFER_COUNT    (uint8_t) 9
#define XUAC_SET_DBG_LEVEL              (uint8_t) 10
#define XUAC_GET_DBG_LEVEL              (uint8_t) 11

#endif /* __REQUESTS_H_ */
