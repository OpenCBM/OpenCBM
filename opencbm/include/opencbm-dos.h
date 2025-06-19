/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2025 Spiro Trikaliotis
 */

/** **************************************************************
** @file include/opencbm.h \n
** @author Spiro Trikaliotis \n
** \n
** @brief Definitions for DOS level functions
**
****************************************************************/

#ifndef OPENCBM_DOS_H
#define OPENCBM_DOS_H

#include "opencbm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @{ @ingroup opencbm_dos */

/** @brief Describes a DOS command
 *
 * This is used to build a DOS command that is to be sent to the
 * floppy, and communicate how the answer is received.
 */
typedef
struct opencbm_dos_cmd {
    /** the address of the device to which this command is (to be) sent */
    uint8_t   deviceAddress;

    /** the size (in bytes) of the command to the drive */
    uint8_t   command_size;

    /** how much bytes the command expects for its answer */
    uint16_t  answer_expected_size;

    /** @brief how many extra bytes are in the answer of the commands.
     * For example, a M-R returns the number of bytes, followed by a CR.
     * As the CR is not part of the answer, it is treated as extra byte.
     */
    uint8_t   answer_expected_extra_size;


    /** @brief pointer to buffer that will contain the answer to the command.
     * This can be NULL if and only if answer_expected_size is 0. */
    uint8_t * answer_buffer;

    /** @brief the size of the answer_buffer.
     *
     * This must be equal or greater than answer_expected_size
     */
    size_t    answer_buffer_size;

    union {
        /** buffer for the command to be sent to the drive */
        uint8_t command[42];

        /** alternative type for the buffer so it doesn't have to best cast */
        char    command_char[42];

        struct {
            char    command[3];
            uint8_t address_low;
            uint8_t address_high;
            uint8_t count;
        } m_r;

        struct {
            char    command[3];
            uint8_t address_low;
            uint8_t address_high;
        } m_r_dos1;

        struct {
            char    command[3];
            uint8_t address_low;
            uint8_t address_high;
            uint8_t count;
        } m_w;

        struct {
            char    command[3];
            uint8_t address_low;
            uint8_t address_high;
        } m_e;
    };
} opencbm_dos_cmd;

/** @brief DOS: Function prototype for callback when reading/writing floppy memory

 When the floppy memory is written to with cbm_dos_memory_write(),
 or read from with cbm_dos_memory_read(), you can give a callback
 function that is called when a specific amount of data is read or
 written.

 This type gives the prototype of this function.

 @param[in] Context
   Pointer to a context that was given to cbm_dos_memory_write() or
   cbm_dos_memory_read() by the caller. This is copied verbatim, without
   any processing, to the callback, and can be used to communicate
   with the callback.

 @param[in] MemoryAddress
   The start address of the floppy memory that is currently processed

 @param[in] MemoryAddressEnd
   The end address of the floppy memory that is currently processed

 @param[in] Count
   The number of bytes that are currently processed

 @param[in] Percent
   The percentage of already processed bytes

 @retval 0 if the operation should continue
 @retval "!= 0" to force the operation to abort

 @remark
   If the callback returns a value "!= 0", the cbm_dos_memory_read() or
   cbm_dos_memory_write() function will abort the operation.
*/
typedef int
opencbm_dos_memory_read_write_callback(
        void *   Context,
        uint16_t MemoryAddress,
        uint16_t MemoryAddressEnd,
        uint16_t Count,
        uint8_t  Percent
        );

EXTERN int CBMAPIDECL
cbm_dos_set_dos1_compatibility(
        int On
        );

EXTERN int CBMAPIDECL
cbm_dos_get_dos1_compatibility(
        void
        );

EXTERN int CBMAPIDECL
cbm_dos_memory_read_set_max_retries(
        unsigned int Number
        );

EXTERN int CBMAPIDECL
cbm_dos_memory_write_set_max_retries(
        unsigned int Number
        );

EXTERN int CBMAPIDECL
cbm_dos_cmd_memory_write(
        CBM_FILE        HandleDevice,
        uint8_t         DeviceAddress,
        uint16_t        Address,
        uint8_t         Count,
        const uint8_t * Buffer
        );

EXTERN int CBMAPIDECL
cbm_dos_cmd_memory_read(
        CBM_FILE  HandleDevice,
        uint8_t * Buffer,
        size_t    BufferSize,
        uint8_t   DeviceAddress,
        uint16_t  MemoryAddress,
        uint16_t  Count
        );

EXTERN int CBMAPIDECL
cbm_dos_cmd_memory_read_dos1(
        CBM_FILE  HandleDevice,
        uint8_t * Buffer,
        size_t    BufferSize,
        uint8_t   DeviceAddress,
        uint16_t  MemoryAddress
        );

EXTERN int CBMAPIDECL
cbm_dos_cmd_memory_execute(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint16_t  MemoryAddress
        );

EXTERN int CBMAPIDECL
cbm_dos_cmd_u1_block_read(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   Channel,
        uint8_t   Unit,
        uint8_t   Track,
        uint8_t   Sector
        );

EXTERN int CBMAPIDECL
cbm_dos_cmd_u2_block_write(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   Channel,
        uint8_t   Unit,
        uint8_t   Track,
        uint8_t   Sector
        );

EXTERN int CBMAPIDECL
cbm_dos_open_channel_generic(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   Channel
        );

EXTERN int CBMAPIDECL
cbm_dos_open_channel_specific(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   Channel,
        uint8_t   BufferNumber
        );

/* higher level functions: */

EXTERN int CBMAPIDECL
cbm_dos_memory_write(
        CBM_FILE                               HandleDevice,
        uint8_t                                DeviceAddress,
        uint16_t                               Address,
        uint16_t                               Count,
        const uint8_t *                        Buffer,
        opencbm_dos_memory_read_write_callback Callback,
        void *                                 Callback_Context
        );

EXTERN int CBMAPIDECL
cbm_dos_memory_read(
        CBM_FILE                               HandleDevice,
        uint8_t *                              Buffer,
        size_t                                 BufferSize,
        uint8_t                                DeviceAddress,
        uint16_t                               MemoryAddress,
        uint16_t                               Count,
        opencbm_dos_memory_read_write_callback Callback,
        void *                                 Callback_Context
        );

EXTERN int CBMAPIDECL
cbm_dos_channel_read(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   ChannelNumber,
        uint16_t  MaxCount,
        uint8_t * Buffer,
        size_t    BufferSize
        );

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* OPENCBM_DOS_H */
