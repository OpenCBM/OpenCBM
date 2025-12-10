/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 2025 Spiro Trikaliotis
 */

#include "arch.h"

#include <time.h>

/*! \brief Sleep for a specified time in microseconds

 This function sleeps, that is, it suspends execution of the program for a
 given time period.

 \param Microseconds
   The time (in microseconds, us) to sleep

 \remark
   As Sleep() of Windows does not have a lower granularity than 1 ms,
   this function rounds up the microseconds to the next millisecond on
   Windows systems.
*/
void arch_sleep_us(unsigned int Microseconds)
{
    struct timespec time_to_sleep = { 0, Microseconds * 1000ul };

    nanosleep(&time_to_sleep, NULL);
}

/*! \brief Sleep for a specified time in milliseconds

 This function sleeps, that is, it suspends execution of the program for a
 given time period.

 \param Milliseconds
   The time (in milliseconds, ms) to sleep
*/
void arch_sleep_ms(unsigned int Milliseconds)
{
    struct timespec time_to_sleep = { 0, Milliseconds * 1000000ul };

    nanosleep(&time_to_sleep, NULL);
}

/*! \brief Sleep for a specified time in seconds

 This function sleeps, that is, it suspends execution of the program for a
 given time period.

 \param Sec
   The time (in seconds, s) to sleep
*/
void arch_sleep_s(unsigned int Seconds)
{
    struct timespec time_to_sleep = { Seconds, 0 };

    nanosleep(&time_to_sleep, NULL);
}
