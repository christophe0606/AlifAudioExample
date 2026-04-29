
/**
 *
 * This file is used to customize the dataflow loop and provide
 * some definition  to the scheduler.cpp
 * Generally this file defines some macros used in the
 * scheduler dataflow loop and some datatypes used in the project.
 *
 */

#pragma once

extern "C"
{
#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
}

#include <cstdlib>

#include "rtos_events.hpp"

#include "datatypes.hpp"

#include "selector_ids.h"

#include "appa_params.h"
#include "appb_params.h"

#include "cmsis_compiler.h"

extern osEventFlagsId_t cg_streamEvent;

// Because memory optimization is enabled in Python scripts, the alignment is NEEDED
//
// To use a memory overlay for the graph FIFOs, the section must be different for each graph
// Python scripts can be customized so that each generated scheduler includes a different
// configuration file where the macro could have different definitions
#define CG_BEFORE_BUFFER __ALIGNED(16) __attribute__((section(".bss.stream_fifo")))

#define CG_BEFORE_NODE_EXECUTION(id)                                                 \
    {                                                                                \
        uint32_t res =                                                               \
            osEventFlagsWait(cg_streamEvent, STREAM_PAUSE_EVENT | STREAM_DONE_EVENT, \
                             osFlagsWaitAny, 0);                                     \
        if (!(res & 0x80000000))                                                     \
        {                                                                            \
            if ((res & STREAM_DONE_EVENT) != 0)                                      \
            {                                                                        \
                cgStaticError = CG_STOP_SCHEDULER;                                   \
                goto errorHandling;                                                  \
            }                                                                        \
            if ((res & STREAM_PAUSE_EVENT) != 0)                                     \
            {                                                                        \
                cgStaticError = CG_PAUSED_SCHEDULER;                                 \
                goto errorHandling;                                                  \
            }                                                                        \
        }                                                                            \
    }

class ContextSwitch
{
  public:
    virtual ~ContextSwitch()
    {
    }
    /*

    Event queue running but posting event disabled.
    Run from data flow thread except for pure event graphs.
    In that case, it is run from event thread.

    */
    virtual int pause() = 0;

    /*

    Run from data  flow thread.
    Posting events is possible but event thread is not yet
    restarted.

    */
    virtual int resume() = 0;
};

#include "stream_runtime_config.hpp"

#define AUDIO_BLOCK 320
#define NB_BINS 128

#define SAMPLE_RATE 16000

#define HAS_AUDIO_SRC
#define HAS_CAMERA_SRC
