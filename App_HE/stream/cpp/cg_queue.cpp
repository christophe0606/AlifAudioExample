/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Library
 * Title:        cg_queue.cpp
 * Description:  Example implementation of a queue for handling events using C++ mutexes
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
#include "custom.hpp"
#include "cg_queue.hpp"
#include <variant>

extern osThreadId_t cg_eventThread;

using namespace arm_cmsis_stream;

MyQueue::MyQueue() : arm_cmsis_stream::EventQueue()
{
    queue = new (std::nothrow) Message[MY_QUEUE_MAX_ELEMS];
    read = 0;
    write = 0;
    nb_elems = 0;
}

MyQueue::~MyQueue()
{
    delete[] queue;
}

bool MyQueue::push(arm_cmsis_stream::Message &&event)
{
    bool ok = false;
    CG_MUTEX_ERROR_TYPE error;
    CG_ENTER_CRITICAL_SECTION(queue_mutex, error);
    if (!CG_MUTEX_HAS_ERROR(error))
    {
        if (nb_elems < MY_QUEUE_MAX_ELEMS)
        {
            queue[write++] = std::move(event);
            if (write == MY_QUEUE_MAX_ELEMS)
            {
                write = 0; // Wrap around
            }

            nb_elems++;
            ok = true;
        }
    }
    CG_EXIT_CRITICAL_SECTION(queue_mutex, error);
    if (cg_eventThread != nullptr)
    {
        osThreadFlagsSet(cg_eventThread, MY_QUEUE_NEW_EVENT_FLAG);
    }
    
    return ok;
}

bool MyQueue::isEmpty()
{
    bool r = true;
    CG_MUTEX_ERROR_TYPE error;
    CG_ENTER_CRITICAL_SECTION(queue_mutex, error);
    r = (nb_elems == 0);
    CG_EXIT_CRITICAL_SECTION(queue_mutex, error);
    return r;
}

void MyQueue::clear()
{
    CG_MUTEX_ERROR_TYPE error;
    CG_ENTER_CRITICAL_SECTION(queue_mutex, error);
    if (!CG_MUTEX_HAS_ERROR(error))
    {
        while (nb_elems != 0)
        {
            Message msg = std::move(queue[read++]);
            if (read == MY_QUEUE_MAX_ELEMS)
            {
                read = 0; // Wrap around
            }
            nb_elems--;
            msg = Message(); // Reset the message
        } 
    }
    CG_EXIT_CRITICAL_SECTION(queue_mutex, error);
}

void MyQueue::end() noexcept
{
    mustEnd_.store(true);
    if (cg_eventThread != nullptr)
    {
        osThreadFlagsSet(cg_eventThread, MY_QUEUE_NEW_EVENT_FLAG);
    }
};

void MyQueue::execute()
{
    CG_MUTEX_ERROR_TYPE error;
    while (!this->mustEnd())
    {
        while ((!this->mustEnd()) && (!isEmpty()))
        {
            Message msg;
            bool messageWasReceived = false;
            CG_ENTER_CRITICAL_SECTION(queue_mutex, error);

            if (!CG_MUTEX_HAS_ERROR(error))
            {
                if (nb_elems != 0)
                {
                    msg = std::move(queue[read++]);
                    if (read == MY_QUEUE_MAX_ELEMS)
                    {
                        read = 0; // Wrap around
                    }

                    nb_elems--;
                    messageWasReceived = true;
                }
            }
            CG_EXIT_CRITICAL_SECTION(queue_mutex, error);

            // Process event with no lock held
            if (messageWasReceived)
            {
                if (std::holds_alternative<LocalDestination>(msg.destination))
                {
                    LocalDestination &local = std::get<LocalDestination>(msg.destination);
                    local.dst->processEvent(local.dstPort, std::move(msg.event));
                }
                else if (std::holds_alternative<DistantDestination>(msg.destination))
                {
                    DistantDestination &dist = std::get<DistantDestination>(msg.destination);
                    this->callHandler(dist.src_node_id, std::move(msg.event));
                }
            }
        }
        if (this->mustEnd())
        {
            return;
        }
        // If new event was pushed and missed with the
        // empty test
        osThreadFlagsWait(MY_QUEUE_NEW_EVENT_FLAG, osFlagsWaitAny, osWaitForever);
    }
}
