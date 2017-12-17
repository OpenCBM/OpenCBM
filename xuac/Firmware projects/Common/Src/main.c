/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "main.h"
#include "xuac.h" // the inventory

// Implementation

int main(void)
{
	// Initialize XUAC platform.
	xuac->Init();

	while (1)
	{
		// Handle incoming commands.
		xuac->USB_Ops->BulkWorker();
	}
}
