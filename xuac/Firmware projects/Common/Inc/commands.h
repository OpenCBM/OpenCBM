/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __COMMANDS_H_
#define __COMMANDS_H_

#include <stdint.h> // for uint8_t, uint16_t, uint32_t

// Exported operations.
void HandleBulkCmd(uint8_t cmd, uint8_t subcmd, uint32_t cmd_data,
                   uint8_t *device_status, uint16_t *request_status,
                   uint32_t *answer_data);

#endif /* __COMMANDS_H_ */
