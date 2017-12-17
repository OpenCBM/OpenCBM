/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2011-2017 Arnd Menge
*/

#include "watchdog.h"

// Forward declarations

static void wdg_setup  (void);
static void wdg_reset  (void);
static void wdg_suspend(void);
static void wdg_stop   (void);

// Register operations in inventory

Watchdog_Operations WatchdogOps =
{
	wdg_setup,
	wdg_reset,
	wdg_suspend,
	wdg_stop
};

// Implementation

static void wdg_setup(void)
{
	// No watchdog so far.
}

static void wdg_reset(void)
{
	// No watchdog so far.
}

static void wdg_suspend(void)
{
	// No watchdog so far.
}

static void wdg_stop(void)
{
	// No watchdog so far.
}
