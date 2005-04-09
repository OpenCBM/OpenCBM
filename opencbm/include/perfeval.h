/*! **************************************************************
** \file include/perfeval.h \n
** \author Spiro Trikaliotis \n
** \version $Id: perfeval.h,v 1.1.2.1 2005-04-09 15:10:55 strik Exp $ \n
** \n
** \brief Functions and macros for performance evaluation purposes
**
****************************************************************/

#ifndef PERFEVAL_H
#define PERFEVAL_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef _CPLUSPLUS */

#include "debug.h"

#ifdef PERFEVAL

/*! One entry of the performance evaluation. This is the
structure as it is written into the memory and/or the file */

typedef struct
PERFORMANCE_EVAL_ENTRY
{
    /*! the RDTSC timestamp */
    __int64 Timestamp;

    /*! The processor on which this event occurred. New in v2 */
    ULONG Processor;

    /*! The thread which is executing this event. New in v2 */
    PETHREAD PeThread;

    /*! The event which is logged */
    ULONG Event;
 
    /*! Additional data for the event */
    ULONG Data;

} PERFORMANCE_EVAL_ENTRY, *PPERFORMANCE_EVAL_ENTRY;

extern VOID PerfInit(VOID);
extern VOID PerfEvent(IN ULONG Event, IN ULONG Data);
extern VOID PerfSave(VOID);

/*! Call PerfInit() if performance evaluation is selected */
#define PERF_INIT() PerfInit()

/*! Call PerfEvent() if performance evaluation is selected */
#define PERF_EVENT(_Event_, _Data_) PerfEvent(_Event_, _Data_)

/*! Only define this non-empty if you want very verbose performance data */
#if 0
  #define PERF_EVENT_VERBOSE(_Event_, _Data_) PERF_EVENT(_Event_, _Data_)
#else
  #define PERF_EVENT_VERBOSE(_Event_, _Data_)
#endif

/*! Call PerfSave() if performance evaluation is selected */
#define PERF_SAVE() PerfSave()

#else /* #ifdef PERFEVAL */

/*! no performance evaluation, do nothing */
#define PERF_INIT()

/*! no performance evaluation, do nothing */
#define PERF_EVENT(_Event_, _Data_)

/*! no performance evaluation, do nothing */
#define PERF_SAVE()

#endif /* #ifdef PERFEVAL */

#ifdef __cplusplus
}
#endif /* #ifdef _CPLUSPLUS */

#endif // #ifndef PERFEVAL_H
