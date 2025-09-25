
#include "cg_enums.h"
#include "cg_queue.hpp"
#include "custom.hpp"
#include <cstdio>

extern "C"
{
#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
#include "cstream_node.h"
#include "disp.h"
#include "main.h"
#include "scheduler.h"
#include "config.h"
}

#include "nodes/Display.hpp"

#include "cg_queue.hpp"

#define NB_MAX_EVENTS 20
#define NB_MAX_BUFS 20

using namespace arm_cmsis_stream;

extern "C"
{
    extern osThreadId_t tid_display;
    extern osThreadId_t tid_stream;

    extern osThreadId_t cg_eventThread;
}

osMemoryPoolId_t cg_eventPool = nullptr;
osMemoryPoolId_t cg_bufPool = nullptr;
osMemoryPoolId_t cg_mutexPool = nullptr;

osThreadId_t tid_display = nullptr;
osThreadId_t tid_stream = nullptr;

osThreadId_t cg_eventThread = nullptr;

osMutexId_t bin_mutex;

int init_memory_pools()
{

    DEBUG_PRINT("Create event pool\n");
    cg_eventPool = osMemoryPoolNew(NB_MAX_EVENTS, sizeof(ListValue) + 16, NULL);
    if (cg_eventPool == NULL)
    {
        ERROR_PRINT("Can't create CMSIS Event Pool of size %d\n", NB_MAX_EVENTS * (sizeof(ListValue) + 16));
        return 1;
    }

    DEBUG_PRINT("Create buf pool\n");
    cg_bufPool = osMemoryPoolNew(NB_MAX_BUFS, sizeof(Tensor<double>) + 16, NULL);
    if (cg_bufPool == NULL)
    {
        ERROR_PRINT("Can't create CMSIS buf Pool of size %d\n", NB_MAX_BUFS * (sizeof(Tensor<double>) + 16));
        osMemoryPoolDelete(cg_eventPool);
        return 1;
    }

    DEBUG_PRINT("Create mutex pool\n");
    cg_mutexPool = osMemoryPoolNew(NB_MAX_BUFS, sizeof(CG_MUTEX) + 16, NULL);
    if (cg_mutexPool == NULL)
    {
        ERROR_PRINT("Can't create CMSIS mutex Pool of size %d\n", NB_MAX_BUFS * (sizeof(CG_MUTEX) + 16));
        osMemoryPoolDelete(cg_eventPool);
        osMemoryPoolDelete(cg_bufPool);
        return 1;
    }

    return 0;
}

void display_thread(void *arg)
{
    DEBUG_PRINT("Display thread started\n");
    
    CStreamNode *disp = get_scheduler_node(DISPLAY_ID);
    if (disp)
    {
        Display *display = reinterpret_cast<Display *>(disp->obj);

        for (;;)
        {
            bool has_changed = false;
            has_changed = display->render();
            if (has_changed)
            {
                lcd_content_was_changed = 1;
            }
            osThreadFlagsWait(LCD_REFRESH_FLAG, osFlagsWaitAny, osWaitForever);
        }
    }
    else
    {
        ERROR_PRINT("No display node found\n");
        goto endMain;
    }

endMain:
    DEBUG_PRINT("Display thread exit\n");
    osThreadExit();
}

void event_thread(void *argument)
{

    DEBUG_PRINT("Event thread started\n");

    arm_cmsis_stream::EventQueue::cg_eventQueue->execute();

    // Delete the event queue
    delete arm_cmsis_stream::EventQueue::cg_eventQueue;
    arm_cmsis_stream::EventQueue::cg_eventQueue = nullptr;

err_evt:
    DEBUG_PRINT("Event thread exit\n");
    osMemoryPoolDelete(cg_eventPool);
    osMemoryPoolDelete(cg_bufPool);
    osMemoryPoolDelete(cg_mutexPool);

    osThreadExit();
}

void stream_thread(void *arg)
{
    uint32_t nb_iter;
    int error;
    DEBUG_PRINT("Stream thread started\n");

    nb_iter = scheduler(&error);
    if (error != 0)
    {
        ERROR_PRINT("Scheduler error %d\n", error);
    }
    DEBUG_PRINT("Scheduler done after %d iterations\n", nb_iter);

err_stream:
    DEBUG_PRINT("End stream thread\n");
    free_scheduler();
    osThreadExit();
}

int app_main(void)
{

    configure_display_and_2d();

    const osThreadAttr_t mainAttr = {
        .stack_size = 4096,
    };

    const osThreadAttr_t audioAttr = {.stack_size = 4096,
                                      .priority = osPriorityRealtime};
    osKernelInitialize();

    tid_display = osThreadNew(display_thread, NULL, &mainAttr);
    cg_eventThread = osThreadNew(event_thread, NULL, &mainAttr);
    tid_stream = osThreadNew(stream_thread, NULL, &audioAttr);


    int err = init_memory_pools();
    if (err != 0)
    {
        goto err_main;
    }

    bin_mutex = osMutexNew(NULL);

    arm_cmsis_stream::EventQueue::cg_eventQueue = new (std::nothrow) MyQueue();
    if (arm_cmsis_stream::EventQueue::cg_eventQueue == nullptr)
    {
        ERROR_PRINT("Can't create CMSIS Event Queue\n");
        goto err_main;
    }

    DEBUG_PRINT("Create nodes\n");
    // Init nodes and starts audio stream
    err = init_scheduler();
    if (err == CG_MEMORY_ALLOCATION_FAILURE)
    {
        ERROR_PRINT("Error: Memory allocation failure during scheduler initialization.\n");
        goto err_main;
    }

    DEBUG_PRINT("Scheduler initialized successfully\n");

    osKernelStart();

err_main:
    return 0;
}
