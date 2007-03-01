/* Name: event_log.c
 * Project: xu1541
 * Author: Till Harbaum
 * Tabsize: 4
 * Copyright: (c) 2007 by Till Harbaum <till@harbaum.org>
 * License: GPL
 * This Revision: $Id: event_log.c,v 1.1 2007-03-01 12:59:08 harbaum Exp $
 *
 * $Log: event_log.c,v $
 * Revision 1.1  2007-03-01 12:59:08  harbaum
 * Added event log
 *
 */

#include <string.h>  // for memset
#include "event_log.h"

#ifdef ENABLE_EVENT_LOG

static unsigned char log[EVENT_LOG_LEN];
static unsigned char cur;

void event_log_init(void) {
  memset(log, EVENT_NONE, EVENT_LOG_LEN);
  cur = 0;
}

/* just store in buffer and wrap if necessary */
void event_log_add(unsigned char event) {
  log[cur++] = event;

  if(cur == EVENT_LOG_LEN) 
    cur = 0;
}

/* get a byte from the log */
unsigned char event_log_get(unsigned char index) {
  return log[(cur+index) % EVENT_LOG_LEN];
}

#endif
