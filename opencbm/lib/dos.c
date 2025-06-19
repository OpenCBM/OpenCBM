/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *  Copyright 2025 Spiro Trikaliotis
 *
*/

/** **************************************************************
** @file lib/dos.c \n
** @author Spiro Trikaliotis \n
** \n
** @brief DOS level functions
**
** @defgroup opencbm_dos OpenCBM DOS level functions
**
****************************************************************/

/** Mark: We are in user-space (for debug.h) */
#define DBG_USERMODE

/** The name of the executable */
#define DBG_PROGNAME "OPENCBM.DLL"

#include "debug.h"

#include <stdio.h>
#include <string.h>

/** mark: We are building the DLL */
#define DLL
#include "opencbm-dos.h"
#include "archlib.h"

#undef DBG_ASSERT
#include <assert.h>
#define DBG_ASSERT(_x) assert(_x)


/** @page opencbm_dos_overview DOS level functions
 *
 * [TOC]
 *
 * These functions can be used to access the floppy drive on the DOS
 * level. That is, they provide a higher abstraction layer than the
 * IEC/IEEE-layer which works with TALK, LISTEN, OPEN and CLOSE commands
 * directly. \n
 *
 * @section functions Available functions
 *
 * @subsection functions_raw Raw functions
 *
 * The following functions are available:
 * - cbm_dos_cmd_memory_read()
 * - cbm_dos_cmd_memory_read_dos1()
 * - cbm_dos_cmd_memory_write()
 * - cbm_dos_cmd_memory_execute()
 * - cbm_dos_cmd_u1_block_read()
 * - cbm_dos_cmd_u2_block_write()
 *
 * @subsection functions_higher Higher-level functions
 *
 * The following functions implement higher level
 * functionality on top of the lower-level functions:
 *
 * - cbm_dos_memory_read()
 * - cbm_dos_memory_write()
 *
 * These functions can be configured with the help of:
 *
 * - cbm_dos_memory_read_set_max_retries()
 * - cbm_dos_memory_write_set_max_retries()
 * - cbm_dos_set_dos1_compatibility()
 */

/** @{ @ingroup opencbm_dos */

#define CBM_DOS_MAX_MEMORY_READ  0x100 /**< @brief maximum number of bytes that can be read with M-R command */
#define CBM_DOS_MAX_MEMORY_WRITE 0x23  /**< @brief maximum number of bytes that can be written with M-W command */

/** @brief DOS1 compatibility flag
 *
 * If this value is not 0, the functions that are DOS1 aware operate more "careful"
 * so they work also on very early DOS1 devices.
 *
 * DOS1 drives were very early 2040, 3040 and 4040 drives. Most of them
 * have been upgraded to DOS2, though.
 *
 * This value is modified with cbm_dos_set_dos1_compatibility().
 */
static int cbm_dos_dos1_compatibility = 0;

/** @brief maximum number of retries for cbm_dos_memory_read()
 *
 * The maximum number of retries that cbm_dos_memory_read()
 * is allowed to perform.
 *
 * This value is modified with cbm_dos_memory_read_set_max_retries).
 */
static int cbm_dos_memory_read_max_retries = 0;

/** @brief maximum number of retries for cbm_dos_memory_write()
 *
 * The maximum number of retries that cbm_dos_memory_write()
 * is allowed to perform.
 *
 * This value is modified with cbm_dos_memory_write_set_max_retries).
 */
static int cbm_dos_memory_write_max_retries = 0;

/** @brief DOS: Set DOS1 compatibility flag

 This function sets the functions that are DOS1 aware in a
 state where they are more careful to handle DOS1 devices correctly.

 @param[in] On
   - set to != 0 if the DOS1 compatibility shoule be switched on
   - set to 0 if the DOS1 compatibility should be switched off

 @retval 0 if the DOS1 compatibility was switched off before calling this function
 @retval 1 if the DOS1 compatibility was switched on before calling this function

 @remark
   - Do not switch DOS1 compatibility on unless you have to! This slows down
     some commands. Especially memory read is slower in order of magnitudes!
   - DOS1 drives were very early 2040, 3040 and 4040 drives. Most of them
     have been upgraded to DOS2, though.
   - cf. cbm_dos_get_dos1_compatibility()

*/
int CBMAPIDECL
cbm_dos_set_dos1_compatibility(
        int On
        )
{
    int dos1_compatibility_old = cbm_dos_dos1_compatibility;

    FUNC_ENTER();

    DBG_ASSERT(On >= 0);
    DBG_ASSERT(On <= 1);

    cbm_dos_dos1_compatibility = On ? 1 : 0;

    FUNC_LEAVE_INT(dos1_compatibility_old);
}

/** @brief DOS: Get DOS1 compatibility flag

 This function return the status about the DOS1 compatibility mode.

 @retval 0 if the DOS1 compatibility is switched off
 @retval 1 if the DOS1 compatibility is switched on

 @remark
   - DOS1 drives were very early 2040, 3040 and 4040 drives. Most of them
     have been upgraded to DOS2, though.
   - cf. cbm_dos_set_dos1_compatibility()

*/
int CBMAPIDECL
cbm_dos_get_dos1_compatibility(
        void
        )
{
    FUNC_ENTER();

    FUNC_LEAVE_INT(cbm_dos_dos1_compatibility);
}


/** @brief @internal DOS: Initialize command structure

 This function initializes the command structure which is used
 for all the cbm_dos_init_cmd*() functions.

 @param[inout] Command
   Pointer to the opencbm_dos_cmd struct that
   should be initialized

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @retval 0 if the struct construction succeeded
 @retval "< 0" if an error occurred

*/
static int
cbm_dos_init_cmd(
        opencbm_dos_cmd * Command,
        uint8_t           DeviceAddress
        )
{
    int rv = -1;

    FUNC_ENTER();

    DBG_ASSERT(Command != NULL);

    memset(Command, 0, sizeof *Command);

    Command->deviceAddress = DeviceAddress;

    rv = 0;

    FUNC_LEAVE_INT(rv);
}


/** @brief @internal DOS: Initialize memory read command

 This function initializes the function to read from the floppy memory
 with the help of the M-R command.

 @param[inout] Command
   Pointer to the opencbm_dos_cmd struct that
   should be initialized

 @param[inout] AnswerBuffer
   Pointer to a memory buffer that will contain the read bytes
   on exit

 @param[in] AnswerBufferSize
   The size of the memory buffer at AnswerBuffer. This value must be
   equal or greater than Count.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to read from

 @param[in] Count
   The number of bytes to be read

 @retval 0 if the struct construction succeeded
 @retval "< 0" if an error occurred

*/
static int
cbm_dos_init_cmd_memory_read(
        opencbm_dos_cmd * Command,
        uint8_t *         AnswerBuffer,
        size_t            AnswerBufferSize,
        uint8_t           DeviceAddress,
        uint16_t          MemoryAddress,
        uint16_t          Count
        )
{
    int rv = -1;

    FUNC_ENTER();

    DBG_ASSERT(Command != NULL);
    DBG_ASSERT(Count <= CBM_DOS_MAX_MEMORY_READ);

    cbm_dos_init_cmd(Command, DeviceAddress);

    if (Count <= CBM_DOS_MAX_MEMORY_READ) {
        strcpy(Command->m_r.command, "M-R");
        Command->m_r.address_low  = 0xFFu & MemoryAddress;
        Command->m_r.address_high = 0xFFu & (MemoryAddress >> 8);
        Command->m_r.count        = Count;
        Command->command_size     = sizeof(Command->m_r);

        if (Command->command_size < sizeof(Command->command)) {
            Command->answer_expected_size       = Count;
            Command->answer_expected_extra_size = 1;
            Command->answer_buffer              = AnswerBuffer;
            Command->answer_buffer_size         = AnswerBufferSize;

            rv = 0;
        }

        DBG_ASSERT(Command->command_size < sizeof(Command->command));
    }

    FUNC_LEAVE_INT(rv);
}

/** @brief @internal DOS: Initialize memory read command, DOS1 version

 This function initializes the function to read from the floppy memory
 with the help of the M-R command.

 DOS1 had a special version of the M-R command, where you could not give a
 Count of bytes to read. You could only read 1 byte at a time.

 @param[inout] Command
   Pointer to the opencbm_dos_cmd struct that
   should be initialized

 @param[inout] AnswerBuffer
   Pointer to a memory buffer that will contain the read bytes
   on exit

 @param[in] AnswerBufferSize
   The size of the memory buffer at AnswerBuffer. This value must be
   equal or greater than Count.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to read from

 @retval 0 if the struct construction succeeded
 @retval "< 0" if an error occurred

*/
static int
cbm_dos_init_cmd_memory_read_dos1(
        opencbm_dos_cmd * Command,
        uint8_t *         AnswerBuffer,
        size_t            AnswerBufferSize,
        uint8_t           DeviceAddress,
        uint16_t          MemoryAddress
        )
{
    int rv = -1;

    FUNC_ENTER();

    DBG_ASSERT(Command != NULL);

    cbm_dos_init_cmd(Command, DeviceAddress);

    strcpy(Command->m_r_dos1.command, "M-R");
    Command->m_r_dos1.address_low  = 0xFFu & MemoryAddress;
    Command->m_r_dos1.address_high = 0xFFu & (MemoryAddress >> 8);
    Command->command_size          = sizeof(Command->m_r_dos1);

    if (Command->command_size < sizeof(Command->command)) {
        Command->answer_expected_size       = 1;
        Command->answer_expected_extra_size = 1;
        Command->answer_buffer_size         = AnswerBufferSize;
        Command->answer_buffer              = AnswerBuffer;

        rv = 0;
    }

    DBG_ASSERT(Command->command_size < sizeof(Command->command));

    FUNC_LEAVE_INT(rv);
}

/** @brief @internal DOS: Initialize memory write command

 This function initializes the function to write to the floppy memory
 with the help of the M-W command.

 @param[inout] Command
   Pointer to the opencbm_dos_cmd struct that
   should be initialized

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to write to

 @param[in] Count
   The number of bytes to be written

 @param[in] Buffer
   Pointer to a memory buffer that contains the bytes
   to be written.

 @retval 0 if the struct construction succeeded
 @retval "< 0" if an error occurred

*/
static int
cbm_dos_init_cmd_memory_write(
        opencbm_dos_cmd * Command,
        uint8_t           DeviceAddress,
        uint16_t          MemoryAddress,
        uint8_t           Count,
        const uint8_t *   Buffer
        )
{
    int rv = -1;

    FUNC_ENTER();

    DBG_ASSERT(Command != NULL);
    DBG_ASSERT(Count <= CBM_DOS_MAX_MEMORY_WRITE);

    cbm_dos_init_cmd(Command, DeviceAddress);

    if (Count <= CBM_DOS_MAX_MEMORY_WRITE) {

        strcpy(Command->m_w.command, "M-W");
        Command->m_w.address_low  = 0xFFu & MemoryAddress;
        Command->m_w.address_high = 0xFFu & (MemoryAddress >> 8);
        Command->m_w.count        = Count;
        Command->command_size     = sizeof(Command->m_w);

        if (Command->command_size + Count < sizeof(Command->command)) {
            rv = 0;
            memcpy(Command->command + Command->command_size, Buffer, Count);
            Command->command_size += Count;
        }

        DBG_ASSERT(Command->command_size < sizeof(Command->command));
    }

    FUNC_LEAVE_INT(rv);
}

/** @brief @internal DOS: Initialize memory execute command

 This function initializes the function to execute code in the floppy memory
 with the help of the M-E command.

 @param[inout] Command
   Pointer to the opencbm_dos_cmd struct that
   should be initialized

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to write to

 @retval 0 if the struct construction succeeded
 @retval "< 0" if an error occurred

*/
static int
cbm_dos_init_cmd_memory_execute(
        opencbm_dos_cmd * Command,
        uint8_t           DeviceAddress,
        uint16_t          MemoryAddress
        )
{
    int rv = -1;

    FUNC_ENTER();

    DBG_ASSERT(Command != NULL);

    cbm_dos_init_cmd(Command, DeviceAddress);

    strcpy(Command->m_e.command, "M-E");
    Command->m_e.address_low  = 0xFFu & MemoryAddress;
    Command->m_e.address_high = 0xFFu & (MemoryAddress >> 8);
    Command->command_size     = sizeof(Command->m_e);

    DBG_ASSERT(Command->command_size < sizeof(Command->command));

    if (Command->command_size < sizeof(Command->command)) {
        rv = 0;
    }

    FUNC_LEAVE_INT(rv);
}

/** @brief @internal DOS: Initialize U1 (block read) or U2 (block write) command

 This function initializes the function to read a block from the
 floppy disk (U1) or to write a block to the floppy disk (U2).
 As the command is almost identical, this function contains the parts
 that are identical in both cases.

 @param[inout] Command
   Pointer to the opencbm_dos_cmd struct that
   should be initialized

 @param[in] FloppyCommand
   String that contains the command to send to the floppy ("U1" or "U2").

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] Channel
   The number of the channel on which this command operates.
   The channel number is the secondary address that was used when opening the
   channel.

 @param[in] Unit
   The unit from which to read or write.
   That is, for dual-floppy drives, this specifies if the block is read from
   unit 0 or unit 1. For single-floppy drives, this must be 0. \n
   Often, this is also called "drive".

 @param[in] Track
   The number of the track that should be read or written to.

 @param[in] Sector
   The number of the sector that should be read or written to.

 @retval 0 if the struct construction succeeded
 @retval "< 0" if an error occurred

*/
static int
cbm_dos_init_cmd_u1_u2_block_read_write(
        opencbm_dos_cmd * Command,
        char *            FloppyCommand,
        uint8_t           DeviceAddress,
        uint8_t           Channel,
        uint8_t           Unit,
        uint8_t           Track,
        uint8_t           Sector
        )
{
    int rv = -1;

    FUNC_ENTER();

    DBG_ASSERT(Command != NULL);

    cbm_dos_init_cmd(Command, DeviceAddress);

    do {
        int len = snprintf(Command->command_char, sizeof(Command->command_char),
            "%s:%u %u %u %u",
            FloppyCommand,
            Channel,
            Unit,
            Track,
            Sector);

        if (len >= sizeof(Command->command)) {
            /* the output did not fit, leave with an error */
            rv = -1;
            break;
        }

        Command->command_size = len;

        rv = 0;

    } while (0);

    FUNC_LEAVE_INT(rv);
}


/** @brief @internal DOS: Initialize U1 (block read) command

 This function initializes the function to read a block from the
 floppy disk with the help of the U1 command.

 @param[inout] Command
   Pointer to the opencbm_dos_cmd struct that
   should be initialized

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] Channel
   The number of the channel in which buffer the block should be read into.
   The channel number is the secondary address that was used when opening the
   channel.


 @param[in] Unit
   The unit from which to read.
   That is, for dual-floppy drives, this specifies if the block is read from
   unit 0 or unit 1. For single-floppy drives, this must be 0. \n
   Often, this is also called "drive".

 @param[in] Track
   The number of the track that should be read.

 @param[in] Sector
   The number of the sector that should be read.

 @retval 0 if the struct construction succeeded
 @retval "< 0" if an error occurred

*/
static int
cbm_dos_init_cmd_u1_block_read(
        opencbm_dos_cmd * Command,
        uint8_t           DeviceAddress,
        uint8_t           Channel,
        uint8_t           Unit,
        uint8_t           Track,
        uint8_t           Sector
        )
{
    int rv = -1;

    FUNC_ENTER();

    rv = cbm_dos_init_cmd_u1_u2_block_read_write(
        Command,
        "U1",
        DeviceAddress,
        Channel,
        Unit,
        Track,
        Sector
        );

    FUNC_LEAVE_INT(rv);
}

/** @brief @internal DOS: Initialize U2 (block write) command

 This function initializes the function to write a block to the
 floppy disk with the help of the U2 command.

 @param[inout] Command
   Pointer to the opencbm_dos_cmd struct that
   should be initialized

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] Channel
   The number of the channel which buffer the block should be written to.
   The channel number is the secondary address that was used when opening the
   channel.

 @param[in] Unit
   The unit from which to write to.
   That is, for dual-floppy drives, this specifies if the block is written to
   unit 0 or unit 1. For single-floppy drives, this must be 0. \n
   Often, this is also called "drive".

 @param[in] Track
   The number of the track that should be written.

 @param[in] Sector
   The number of the sector that should be written.

 @retval 0 if the struct construction succeeded
 @retval "< 0" if an error occurred

*/
static int
cbm_dos_init_cmd_u2_block_write(
        opencbm_dos_cmd * Command,
        uint8_t           DeviceAddress,
        uint8_t           Channel,
        uint8_t           Unit,
        uint8_t           Track,
        uint8_t           Sector
        )
{
    int rv = -1;

    FUNC_ENTER();

    rv = cbm_dos_init_cmd_u1_u2_block_read_write(
        Command,
        "U2",
        DeviceAddress,
        Channel,
        Unit,
        Track,
        Sector
        );

    FUNC_LEAVE_INT(rv);
}

/** @brief @internal DOS: Send a command to the floppy

 This function sends a command to the floppy.

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] Command
   Pointer to the opencbm_dos_cmd struct that contains the command.

 @retval 0 if the command could be sent
 @retval "< 0" if an error occurred

 @remark
   If the command expects an answer, call cbm_dos_cmd_read_answer()
   afterwards or use cbm_dos_cmd_send_and_read_answer() in the first
   place, which combines both sending and reading the answer.
*/
static int
cbm_dos_cmd_send(
        CBM_FILE          HandleDevice,
        opencbm_dos_cmd * Command
        )
{
    int rv = -1;

    FUNC_ENTER();

    rv = cbm_exec_command(HandleDevice, Command->deviceAddress, Command->command, Command->command_size);

    if (rv) {
        rv = -1;
    }

    FUNC_LEAVE_INT(rv);
}

/** @brief @internal DOS: Retrieve the answer to a command

 This function reads the answer after sending a command to the floppy.

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] Command
   Pointer to the opencbm_dos_cmd struct that contains the command
   and, especially, the answer buffer.

 @retval "0" if the command could be read completely
 @retval "> 0" if the command could be read partially; in this case, the value
               of the return value is the number of bytes that were read
 @retval "< 0" if an error occurred

 @remark
   Consider using cbm_dos_cmd_send_and_read_answer() that combines sending
   the command with cbm_dos_cmd_send() and retrieving the answer with the
   help of this function.
*/
static int
cbm_dos_cmd_read_answer(
        CBM_FILE          HandleDevice,
        opencbm_dos_cmd * Command
        )
{
    int rv = -1;

    FUNC_ENTER();

    do {
        uint8_t buffer_extra[2];

        DBG_ASSERT(Command->answer_expected_extra_size < sizeof buffer_extra);

        if (   (Command->answer_expected_size == 0)
            && (Command->answer_expected_extra_size == 0)
            )
        {
            rv = 0;
            break;
        }

        if (Command->answer_expected_size) {
            if (
                    (rv = cbm_dos_channel_read(
                        HandleDevice,
                        Command->deviceAddress,
                        15,
                        Command->answer_expected_size,
                        Command->answer_buffer,
                        Command->answer_buffer_size)
                    ) != Command->answer_expected_size
                )
            {
                break;
            }
        }

        if (Command->answer_expected_extra_size) {
            if (
                    (rv = cbm_dos_channel_read(
                        HandleDevice,
                        Command->deviceAddress,
                        15,
                        Command->answer_expected_extra_size,
                        buffer_extra,
                        sizeof buffer_extra)
                    ) != Command->answer_expected_extra_size
                )
            {
                rv = -1;
                break;
            }
        }

        rv = 0;

    } while (0);

    FUNC_LEAVE_INT(rv);
}

/** @brief @internal DOS: Send a command and retrieve the answer

 This function sends a commands to the floppy and reads its answer afterwards.

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] Command
   Pointer to the opencbm_dos_cmd struct that contains the command
   and, especially, the answer buffer.

 @retval 0 if the command could be sent
 @retval "< 0" if an error occurred

 @remark
   This funcion combines cbm_dos_cmd_send() and cbm_dos_cmd_read_answer()
   into one function.
*/
static int
cbm_dos_cmd_send_and_read_answer(
        CBM_FILE          HandleDevice,
        opencbm_dos_cmd * Command)
{
    int rv = -1;

    FUNC_ENTER();

    do {
        if ((rv = cbm_dos_cmd_send(HandleDevice, Command)) < 0) {
            break;
        }

        if ((rv = cbm_dos_cmd_read_answer(HandleDevice, Command)) != 0) {
            break;
        }

        rv = 0;

    } while (0);

    FUNC_LEAVE_INT(rv);
}

/** @brief DOS: Write to the floppy memory

 This function write to the floppy memory
 with the help of the M-W command

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to write to

 @param[in] Count
   The number of bytes to be written

 @param[in] Buffer
   Pointer to a memory buffer that contains the bytes
   to be written.

 @retval 0 if the write succeeded
 @retval "< 0" if an error occurred

 @remark
   Consider using the higher-level function cbm_dos_memory_write() instead.
*/
int CBMAPIDECL
cbm_dos_cmd_memory_write(
        CBM_FILE        HandleDevice,
        uint8_t         DeviceAddress,
        uint16_t        MemoryAddress,
        uint8_t         Count,
        const uint8_t * Buffer
        )
{
    opencbm_dos_cmd command_memory_write;

    int rv = -1;

    FUNC_ENTER();

    do {
        if ((rv = cbm_dos_init_cmd_memory_write(
                    &command_memory_write,
                    DeviceAddress,
                    MemoryAddress,
                    Count,
                    Buffer)
                < 0))
        {
            break;
        }

        if ((rv = cbm_dos_cmd_send_and_read_answer(HandleDevice, &command_memory_write)) < 0) {
            break;
        }

    } while (0);

    FUNC_LEAVE_INT(rv);
}


/** @brief DOS: Read from the floppy memory

 This function reads from the floppy memory
 with the help of the M-R command

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[inout] AnswerBuffer
   Pointer to a memory buffer that will contain the read bytes
   on exit

 @param[in] AnswerBufferSize
   The size of the memory buffer at Buffer. This value must be
   equal or greater than Count.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to read from.

 @param[in] Count
   The number of bytes to be read

 @retval 0 if the read succeeded
 @retval "< 0" if an error occurred

 @remark
   This function will work on floppy drives, starting with DOS2.
   This covers almost all drives, but not very early 2040, 3040 and 4040
   drives. For these, use cbm_dos_cmd_memory_read_dos1() instead. \n
   \n
   Consider using the higher-level function cbm_dos_memory_read() instead.
*/
int CBMAPIDECL
cbm_dos_cmd_memory_read(
        CBM_FILE  HandleDevice,
        uint8_t * AnswerBuffer,
        size_t    AnswerBufferSize,
        uint8_t   DeviceAddress,
        uint16_t  MemoryAddress,
        uint16_t  Count
        )
{
    opencbm_dos_cmd command_memory_read;

    int rv = -1;

    FUNC_ENTER();

    do {
        if ((rv = cbm_dos_init_cmd_memory_read(
                    &command_memory_read,
                    AnswerBuffer,
                    AnswerBufferSize,
                    DeviceAddress,
                    MemoryAddress,
                    Count))
                < 0)
        {
            break;
        }

        if ((rv = cbm_dos_cmd_send_and_read_answer(HandleDevice, &command_memory_read)) < 0) {
            break;
        }

    } while (0);

    FUNC_LEAVE_INT(rv);
}


/** @brief DOS: Read from the floppy memory, DOS1 variant

 This function reads from the floppy memory
 with the help of the M-R command

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[inout] AnswerBuffer
   Pointer to a memory buffer that will contain the read bytes
   on exit

 @param[in] AnswerBufferSize
   The size of the memory buffer at AnswerBuffer. This value must be
   equal or greater than 1.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to read from.

 @retval 0 if the read succeeded
 @retval "< 0" if an error occurred

 @remark
   This function works on all floppy drives, including DOS1 versions.
   That is, it will also run on very early 2040, 3040 and 4040 drives.

   However, it is restricted to reading one byte at a time only, and thus,
   is very slow if multiple bytes have to be read.\n
   It is almost always a good idea to use cbm_dos_cmd_memory_read() instead. \n
   \n
   Also, consider using the higher-level function cbm_dos_memory_read() instead.
*/
int CBMAPIDECL
cbm_dos_cmd_memory_read_dos1(
        CBM_FILE  HandleDevice,
        uint8_t * AnswerBuffer,
        size_t    AnswerBufferSize,
        uint8_t   DeviceAddress,
        uint16_t  MemoryAddress
        )
{
    opencbm_dos_cmd command_memory_read;

    int rv = -1;

    FUNC_ENTER();

    do {
        if ((rv = cbm_dos_init_cmd_memory_read_dos1(
                    &command_memory_read,
                    AnswerBuffer,
                    AnswerBufferSize,
                    DeviceAddress,
                    MemoryAddress))
                < 0)
        {
            break;
        }

        if ((rv = cbm_dos_cmd_send_and_read_answer(HandleDevice, &command_memory_read)) < 0) {
            break;
        }

    } while (0);

    FUNC_LEAVE_INT(rv);
}

/** @brief DOS: Execute code in the floppy memory

 This function executes code int the floppy memory
 with the help of the M-E command

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to read from.

 @retval 0 if the command succeeded
 @retval "< 0" if an error occurred

*/
int CBMAPIDECL
cbm_dos_cmd_memory_execute(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint16_t  MemoryAddress
        )
{
    opencbm_dos_cmd command_memory_execute;

    int rv = -1;

    FUNC_ENTER();

    do {
        if ((rv = cbm_dos_init_cmd_memory_execute(
                    &command_memory_execute,
                    DeviceAddress,
                    MemoryAddress)
                < 0))
        {
            break;
        }

        if ((rv = cbm_dos_cmd_send_and_read_answer(HandleDevice, &command_memory_execute)) < 0) {
            break;
        }

    } while (0);

    FUNC_LEAVE_INT(rv);
}

/** @brief DOS: Read a block from floppy disk and store it in the DOS buffer

 This function reads a block from the floppy with the U1 command.
 The contents of the block are stored in a DOS buffer of the corresponding
 channel.

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] Channel
   The number of the channel in which buffer the block should be read. The
   channel number is the secondary address that was used when
   opening the channel.

 @param[in] Unit
   The unit from which to read. That is, for dual-floppy drives,
   this specifies if the block is read from unit 0 or unit 1.
   Often, this is also called "drive".

 @param[in] Track
   The number of the track that should be read.

 @param[in] Sector
   The number of the sector that should be read.

 @retval 0 if the read succeeded
 @retval "< 0" if an error occurred

 @remark
   This function works on all floppy drives, except DOS1 versions.
   That is, it will not run on very early 2040, 3040 and 4040 drives.
*/
int CBMAPIDECL
cbm_dos_cmd_u1_block_read(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   Channel,
        uint8_t   Unit,
        uint8_t   Track,
        uint8_t   Sector
        )
{
    opencbm_dos_cmd command_u1_block_read;

    int rv = -1;

    FUNC_ENTER();

    do {
        if ((rv = cbm_dos_init_cmd_u1_block_read(
                    &command_u1_block_read,
                    DeviceAddress,
                    Channel,
                    Unit,
                    Track,
                    Sector)
                < 0))
        {
            break;
        }

        if ((rv = cbm_dos_cmd_send_and_read_answer(HandleDevice, &command_u1_block_read)) < 0) {
            break;
        }

    } while (0);

    FUNC_LEAVE_INT(rv);
}

/** @brief DOS: Write a block from DOS buffer to the floppy disk

 This function writes a block to the floppy disk with the U2 command.
 The contents of the block are taken from the DOS buffer of the corresponding
 channel.

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] Channel
   The number of the channel which buffer the block should be written to.
   The channel number is the secondary address that was used when opening the
   channel.

 @param[in] Unit
   The unit to which to write to. That is, for dual-floppy drives,
   this specifies if the block is written to unit 0 or unit 1.
   Often, this is also called "drive".

 @param[in] Track
   The number of the track that should be written.

 @param[in] Sector
   The number of the sector that should be written.

 @retval 0 if the write succeeded
 @retval "< 0" if a write occurred

 @remark
   This function works on all floppy drives, except DOS1 versions.
   That is, it will not run on very early 2040, 3040 and 4040 drives.
*/
int CBMAPIDECL
cbm_dos_cmd_u2_block_write(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   Channel,
        uint8_t   Unit,
        uint8_t   Track,
        uint8_t   Sector
        )
{
    opencbm_dos_cmd command_u2_block_write;

    int rv = -1;

    FUNC_ENTER();

    do {
        if ((rv = cbm_dos_init_cmd_u2_block_write(
                    &command_u2_block_write,
                    DeviceAddress,
                    Channel,
                    Unit,
                    Track,
                    Sector)
                < 0))
        {
            break;
        }

        if ((rv = cbm_dos_cmd_send_and_read_answer(HandleDevice, &command_u2_block_write)) < 0) {
            break;
        }

    } while (0);

    FUNC_LEAVE_INT(rv);
}

/** @brief DOS: Open a channel for direct block access

 This function opens a channel on the floppy disk for use
 with the block-read or block-write, U1 or U2 commands.

 It opens an arbitrary channel.

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] Channel
   The number of the channel which buffer the block should be written to.
   The channel number is the secondary address that was used when opening the
   channel.

 @retval 0 if the write succeeded
 @retval "< 0" if a write occurreda

 @remark
   If you need a specific channel, use cbm_dos_open_channel_specific() instead.
*/
int CBMAPIDECL
cbm_dos_open_channel_generic(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   Channel
        )
{
    int rv = -1;

    FUNC_ENTER();

    rv = cbm_open(HandleDevice, DeviceAddress, Channel, "#", 1);

    FUNC_LEAVE_INT(rv);
}

/** @brief DOS: Open a channel on a specific buffer for direct block access

 This function opens a channel on the floppy disk for use
 with the block-read or block-write, U1 or U2 commands.

 It opens a channel with a specified buffer.

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] Channel
   The number of the channel which buffer the block should be written to.
   The channel number is the secondary address that was used when opening the
   channel.

 @param[in] BufferNumber
   The number of the buffer to open.

 @retval 0 if the write succeeded
 @retval "< 0" if a write occurreda

 @remark
   If you do not need a specific channel, use cbm_dos_open_channel_generic() instead.
*/
int CBMAPIDECL
cbm_dos_open_channel_specific(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   Channel,
        uint8_t   BufferNumber
        )
{
    int rv = -1;
    char channel_string[5];
    int channel_string_len;

    FUNC_ENTER();

    channel_string_len = snprintf(channel_string, sizeof channel_string, "#%u", BufferNumber);

    DBG_ASSERT(channel_string_len < sizeof channel_string);

    if (channel_string_len < sizeof channel_string) {
        rv = cbm_open(HandleDevice, DeviceAddress, Channel, channel_string, channel_string_len);
    }

    FUNC_LEAVE_INT(rv);
}

/** @brief DOS: Set the maximum number of retries for cbm_dos_memory_read()

 This function sets the maximum number of retries that cbm_dos_memory_read()
 is allowed to perform.

 This might be necessary if the cable setup is not fully reliable. There are
 configurations where the IEC protocol is not completely stable, but the
 fast protocols work, anyway. In this case, it is a good idea to allow
 cbm_dos_memory_read() to handle some errors and recover from them.

 The default is no retries.

 @param[in] Number
   The maximum number of retries that are allowed. Set to 0 to disallow
   retries.

 @return
   The maximum number of retries before calling this function.
   If the returned value is 0, no retries were allowed.

 @remark
   - Do not switch on retries if it is not necessary. In most cases, it
     is better to fail instead of hiding the fact that the setup is not reliable.
     In some cases, however, it might be good to be able to do memory operations,
     anyway. This might be good for downloading the firmware ROMs, or for a disk monitor.

*/
int CBMAPIDECL
cbm_dos_memory_read_set_max_retries(
        unsigned int Number
        )
{
    int retries_old = cbm_dos_memory_read_max_retries;

    FUNC_ENTER();

    cbm_dos_memory_read_max_retries = Number;

    FUNC_LEAVE_INT(retries_old);
}

/** @brief DOS: Set the maximum number of retries for cbm_dos_memory_write()

 This function sets the maximum number of retries that cbm_dos_memory_write()
 is allowed to perform.

 This might be necessary if the cable setup is not fully reliable. There are
 configurations where the IEC protocol is not completely stable, but the
 fast protocols work, anyway. In this case, it is a good idea to allow
 cbm_dos_memory_write() to handle some errors and recover from them.

 The default is no retries.

 @param[in] Number
   The maximum number of retries that are allowed. Set to 0 to disallow
   retries.

 @return
   The maximum number of retries before calling this function.
   If the returned value is 0, no retries were allowed.

 @remark
   - Do not switch on retries if it is not necessary. In most cases, it
     is better to fail instead of hiding the fact that the setup is not reliable.
     In some cases, however, it might be good to be able to do memory operations,
     anyway. This might be good for downloading the firmware ROMs, or for a disk monitor.

*/
int CBMAPIDECL
cbm_dos_memory_write_set_max_retries(
        unsigned int Number
        )
{
    int retries_old = cbm_dos_memory_write_max_retries;

    FUNC_ENTER();

    cbm_dos_memory_write_max_retries = Number;

    FUNC_LEAVE_INT(retries_old);
}


/** @brief DOS: Write to the floppy memory

 This function writes an arbitrary region to the floppy memory.

 This is a higher-level function that circumvents
 restrictions of the underlying M-W command

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to write to.

 @param[in] Count
   The number of bytes to be written

 @param[in] Buffer
   Pointer to a memory buffer that contain the bytes to be
   written

 @param[in] Callback
   Pointer to an opencbm_dos_memory_read_write_callback function
   that is signalling progress on execution.

   If the function returns a value != 0, the memory read is aborted.

   The pointer can be NULL if the caller is not interested in
   progress updates.

 @param[in] Callback_Context
   Pointer to a context that will be given to the
   opencbm_dos_memory_read_write_callback function verbatim, that
   is, the pointer and the contents are not used at all.

 @retval 0 if the write succeeded
 @retval "< 0" if an error occurred

 @remark
   This is a higher-level function than cbm_dos_cmd_memory_write() or
   It can write up to 64 KB of memory at once. \n
   \n
   Of course, writing to working areas (page 0, 1 and 2) can be problematic
   as it might disturb the transmission or the operation of the floppy drive.
   \n
   This function works on all floppy drives.
*/
int CBMAPIDECL
cbm_dos_memory_write(
        CBM_FILE                               HandleDevice,
        uint8_t                                DeviceAddress,
        uint16_t                               MemoryAddress,
        uint16_t                               Count,
        const uint8_t *                        Buffer,
        opencbm_dos_memory_read_write_callback Callback,
        void *                                 Callback_Context
        )
{
    int rv = -1;
    int offset_start;

    int memory_write_max = CBM_DOS_MAX_MEMORY_WRITE;

    uint16_t count_missing;

    uint16_t callback_next = 0x0;

    int retrycounter;

    FUNC_ENTER();

    for (offset_start = 0; offset_start < Count; offset_start += count_missing) {

        /* how many bytes are left? */
        count_missing = Count - offset_start;

        /* if we need more byte than we can read at once, limit the number of bytes
         * to read to the maximum
         */
        if (count_missing > memory_write_max) {
            count_missing = memory_write_max;
        }

        if (offset_start >= callback_next) {
            if (Callback
                    && Callback(
                        Callback_Context,
                        MemoryAddress + offset_start,
                        MemoryAddress + offset_start + count_missing - 1,
                        count_missing,
                        (100 * offset_start) / Count
                        )
               )
            {
                rv = -1;
                break;
            }

            callback_next += 0x100;
        }

        for (retrycounter = cbm_dos_memory_write_max_retries; retrycounter >= 0; --retrycounter) {
            rv = cbm_dos_cmd_memory_write(HandleDevice,
                    DeviceAddress, MemoryAddress + offset_start, count_missing,
                    Buffer + offset_start);

            if (rv == 0) {
                break;
            }
        }

        if (rv < 0) {
            break;
        }
    }

    if (rv == 0 && Callback) {
        Callback(
                Callback_Context,
                MemoryAddress + Count,
                MemoryAddress + Count,
                0,
                100
                );
    }

    FUNC_LEAVE_INT(rv);
}

/** @brief DOS: Read from the floppy memory

 This function reads an arbitrary region from the floppy memory.

 This is a higher-level function that circumvents
 restrictions of the underlying M-R command

 @param[in] HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 @param[inout] Buffer
   Pointer to a memory buffer that will contain the read bytes
   on exit

 @param[in] BufferSize
   The size of the memory buffer at Buffer. This value must be
   equal or greater than Count.

 @param[in] DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 @param[in] MemoryAddress
   The address of the floppy RAM to read from.

 @param[in] Count
   The number of bytes to be read

 @param[in] Callback
   Pointer to an opencbm_dos_memory_read_write_callback function
   that is signalling progress on execution.

   If the function returns a value != 0, the memory read is aborted.

   The pointer can be NULL if the caller is not interested in
   progress updates.

 @param[in] Callback_Context
   Pointer to a context that will be given to the
   opencbm_dos_memory_read_write_callback function verbatim, that
   is, the pointer and the contents are not used at all.

 @retval 0 if the read succeeded
 @retval "< 0" if an error occurred

 @remark
   This is a higher-level function than cbm_dos_cmd_memory_read() or
   cbm_dos_cmd_memory_read_dos1(). It can read up to 64 KB of memory
   at once. \n
   \n
   This function works on all floppy drives.
*/
int CBMAPIDECL
cbm_dos_memory_read(
        CBM_FILE                               HandleDevice,
        uint8_t *                              Buffer,
        size_t                                 BufferSize,
        uint8_t                                DeviceAddress,
        uint16_t                               MemoryAddress,
        uint16_t                               Count,
        opencbm_dos_memory_read_write_callback Callback,
        void *                                 Callback_Context
        )
{
    int rv = -1;
    int offset_start;
    int offset_end;

    int memory_read_max = CBM_DOS_MAX_MEMORY_READ;

    uint16_t memory_page;
    uint16_t count_missing;

    size_t buffer_write_offset = 0;

    int page2workaround_allowed = 1;

    int retrycounter;

    FUNC_ENTER();

    if (cbm_dos_dos1_compatibility) {
        memory_read_max = 1;
    }


    /* The 154x/157x/1581 drives (and possibly others) cannot cross a page
     * boundary with a M-R command. Thus, we make sure that we do not
     * have to cross a page anytime.
     *
     * For this, we handle each page of the transfer on its own. That's why
     * we need to split the MemoryAddress into memory_page and an offset into it.
     */
    memory_page = MemoryAddress & 0xFF00u;
    offset_start = MemoryAddress & 0xFFu;
    offset_end = Count + offset_start;

    for (/* offset_start */; offset_start < offset_end; offset_start += count_missing) {

        /* how many bytes are left? */
        count_missing = offset_end - offset_start;

        /* if we need more byte than we can read at once, limit the number of bytes
         * to read to the maximum
         */
        if (count_missing > memory_read_max) {
            count_missing = memory_read_max;
        }

        if ((offset_start & 0xFFu) + count_missing > 0x100) {
            count_missing = 0x100 - (offset_start & 0xFFu);
        }

        if (Callback
                && Callback(
                    Callback_Context,
                    memory_page + offset_start,
                    memory_page + offset_start + count_missing - 1,
                    count_missing,
                    (100 * buffer_write_offset) / Count
                    )
           )
        {
            rv = -1;
            break;
        }

        for (retrycounter = cbm_dos_memory_read_max_retries; retrycounter >= 0; --retrycounter) {
            if (cbm_dos_dos1_compatibility) {
                rv = cbm_dos_cmd_memory_read_dos1(HandleDevice, Buffer + buffer_write_offset, BufferSize - buffer_write_offset,
                        DeviceAddress, memory_page + offset_start);
            }
            else {
                rv = cbm_dos_cmd_memory_read(HandleDevice, Buffer + buffer_write_offset, BufferSize - buffer_write_offset,
                        DeviceAddress, memory_page + offset_start, count_missing);
            }
            if (rv == 0) {
                break;
            }
        }

        if (rv < 0) {
            break;
        }
        else if (rv > 0) {
            /*
             * The read could not be completed.
             *
             * If we are reading page 2 of the floppy drive's memory,
             * this error might be due to the special handling of the $02D4
             * address in the 154x/157x ($02CF in the 1581). It returns a CR
             * and aborts the transfer.
             *
             * If this is the case, restart the transmission for
             * the rest as a work-around.
             */
            if (((memory_page + offset_start + rv) & 0xFF00u) == 0x200 && page2workaround_allowed) {
                /* do not retry the workaround a second time */
                page2workaround_allowed = 0;
                count_missing = rv - 1;
            }
        }

        buffer_write_offset += count_missing;
    }

    if (rv == 0 && Callback) {
        Callback(
                Callback_Context,
                MemoryAddress + Count,
                MemoryAddress + Count,
                0,
                100
                );
    }

    FUNC_LEAVE_INT(rv);
}


int CBMAPIDECL
cbm_dos_channel_read(
        CBM_FILE  HandleDevice,
        uint8_t   DeviceAddress,
        uint8_t   ChannelNumber,
        uint16_t  MaxCount,
        uint8_t * Buffer,
        size_t    BufferSize
        )
{
    int rv = -1;
    int talk_is_done = 0;

    FUNC_ENTER();

    DBG_ASSERT(MaxCount <= BufferSize);

    do {
        if ((rv = cbm_talk(HandleDevice, DeviceAddress, ChannelNumber)) < 0) {
            /* talk failed, we are done */
            rv = -1;
            break;
        }

        talk_is_done = 1;

        if (MaxCount) {
            rv = cbm_raw_read(HandleDevice, Buffer, MaxCount);
        }

    } while (0);

    if (talk_is_done) {
        cbm_untalk(HandleDevice);
    }

    FUNC_LEAVE_INT(rv);
}


/** @} */
