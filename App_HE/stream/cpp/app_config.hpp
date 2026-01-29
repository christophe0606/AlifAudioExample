/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Library
 * Title:        app_config.hpp
 * Description:  Example configuration for CMSIS-Stream with event handling in bare metal
 *               and multi-threaded environments.
 *
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * --------------------------------------------------------------------
 *
 * Copyright (C) 2021-2025 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CUSTOM_H_
#define CUSTOM_H_

extern "C"
{
#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_os2.h"
#include "config.h"
#include "arm_math_types.h"
}

struct cf32 {
    float real;
    float imag;
};

struct sf32 {
    float left;
    float right;
};

struct cq15 {
    q15_t real;
    q15_t imag;
};

struct sq15 {
    q15_t left;
    q15_t right;
};



#include "rtos_events.hpp"

// Because memory optimization is enabled
#define CG_BEFORE_BUFFER __ALIGNED(16)

#define CG_BEFORE_SCHEDULE \
  uint32_t errorFlags = 0;

#define CG_BEFORE_NODE_EXECUTION(ID)                                                                             \
{                                                                                                                \
    errorFlags = osThreadFlagsWait(AUDIO_SINK_UNDERFLOW_EVENT | AUDIO_SOURCE_OVERFLOW_EVENT, osFlagsWaitAny, 0); \
    if (errorFlags & osFlagsErrorResource)                                                                       \
    {                                                                                                            \
        errorFlags = 0;                                                                                          \
    }                                                                                                            \
    if (errorFlags & AUDIO_SOURCE_OVERFLOW_EVENT)                                                                \
    {                                                                                                            \
        cgStaticError = CG_BUFFER_OVERFLOW;                                                                      \
        goto errorHandling;                                                                                      \
    }                                                                                                            \
}




#endif