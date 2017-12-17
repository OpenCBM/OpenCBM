/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#ifndef __WATCHDOG_H_
#define __WATCHDOG_H_

// Structure for the inventory

typedef struct
{
	void (*Setup)   (void);
	void (*Reset)   (void);
	void (*Suspend) (void);
	void (*Stop)    (void);
} Watchdog_Operations;

#endif /* __WATCHDOG_H_ */
