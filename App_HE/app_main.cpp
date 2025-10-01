
#include "cg_enums.h"
#include "custom.hpp"
#include <cstdio>

#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "cstream_node.h"


extern "C"
{
#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
#include "config.h"
#include "scheduler.h"

    extern int app_main(void);
}

#include "cg_queue.hpp"

#include "nodes/AppDisplay.hpp"
#include "nodes/VStreamVideoSource.hpp"

#include "cg_queue.hpp"

#define NB_MAX_EVENTS 20
#define NB_MAX_BUFS 20

using namespace arm_cmsis_stream;

extern "C"
{
    extern osThreadId_t tid_interrupts;
    extern osThreadId_t tid_stream;

    extern osThreadId_t cg_eventThread;
}

osMemoryPoolId_t cg_eventPool = nullptr;
osMemoryPoolId_t cg_bufPool = nullptr;
osMemoryPoolId_t cg_mutexPool = nullptr;

osThreadId_t tid_stream = nullptr;
osThreadId_t cg_eventThread = nullptr;
osThreadId_t tid_interrupts = nullptr;

// osMutexId_t bin_mutex;

/* Camera frame buffer (RAW8 or RGB565) */
uint8_t CAM_Frame[CAMERA_BUFFER_SIZE] CAMERA_FRAME_BUF_ATTRIBUTE;

/* Display frame buffer */
uint8_t LCD_Frame[DISPLAY_BUFFER_SIZE] DISPLAY_FRAME_BUF_ATTRIBUTE;

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


void VideoSrc_Event_Callback(uint32_t event)
{
    if (event & VSTREAM_EVENT_DATA)
    {
        /* LCD frame is available */
        if (tid_interrupts != NULL)
            osThreadFlagsSet(tid_interrupts, VIDEO_SRC_EVT);
    }
}


AppDisplay *disp = nullptr;
VStreamVideoSource *video_src = nullptr;

void VideoSink_Event_Callback(uint32_t event)
{
    if (event & VSTREAM_EVENT_DATA)
    {
        
        /* Video frame is available in camera frame buffer */
        // if (tid_interrupts != NULL)
        //     osThreadFlagsSet(tid_interrupts, VIDEO_SRC_EVT);
    }
}

void interrupt_thread(void *arg)
{
    DEBUG_PRINT("Interrupt thread started\n");


    for (;;)
    {
        // Wait for interrupt event
        uint32_t res = osThreadFlagsWait(VIDEO_SRC_EVT, osFlagsWaitAny, osWaitForever);
        if (video_src && (res & VIDEO_SRC_EVT))
        {
            Message msg{
                LocalDestination{video_src, 0},
                Event(kDo, kHighPriority)};
            DEBUG_PRINT("Push event for video src\n");
            bool ok = EventQueue::cg_eventQueue->push(std::move(msg));
            if (!ok)
            {
                ERROR_PRINT("Event queue overflow for video src\n");
            }
        }

    }

    // Cleanup and exit the thread if needed
    DEBUG_PRINT("Interrupt thread exit\n");
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
    CStreamNode *c_video_src = nullptr;
    CStreamNode *c_disp = nullptr;
    // init_camera();// Introduces heavy flickering on UI although
    //  camera is not started and just initialized !
    // configure_display_and_2d();

    const osThreadAttr_t dispAttr = {
        .stack_size = 4096,
    };

    const osThreadAttr_t eventAttr = {
        .stack_size = 4096,
        .priority = osPriorityHigh};

    const osThreadAttr_t interruptAttr = {
        .stack_size = 4024,
        .priority = osPriorityHigh};

    const osThreadAttr_t audioAttr = {.stack_size = 4096,
                                      .priority = osPriorityRealtime};
    osKernelInitialize();

    // tid_display = osThreadNew(display_thread, NULL, &dispAttr);
    tid_interrupts = osThreadNew(interrupt_thread, NULL, &interruptAttr);

    cg_eventThread = osThreadNew(event_thread, NULL, &eventAttr);
    tid_stream = osThreadNew(stream_thread, NULL, &audioAttr);

    int err = init_memory_pools();
    if (err != 0)
    {
        goto err_main;
    }

    // bin_mutex = osMutexNew(NULL);

    arm_cmsis_stream::EventQueue::cg_eventQueue = new (std::nothrow) MyQueue(osPriorityLow, osPriorityNormal, osPriorityHigh);
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

#if defined(VIDEOSOURCE_ID)
    c_video_src = get_scheduler_node(VIDEOSOURCE_ID);
    if (c_video_src == nullptr)
    {
        ERROR_PRINT("No video source node found\n");
        osThreadExit();
    }
    video_src = reinterpret_cast<VStreamVideoSource *>(c_video_src->obj);

#else
    video_src = nullptr;
#endif

#if defined(DISPLAY_ID)
    c_disp = get_scheduler_node(DISPLAY_ID);

    if (c_disp == nullptr)
    {
        ERROR_PRINT("No display node found\n");
        osThreadExit();
    }

    disp = reinterpret_cast<AppDisplay *>(c_disp->obj);

#else
    disp = nullptr;
#endif

    DEBUG_PRINT("Scheduler initialized successfully\n");

    osKernelStart();

err_main:
    return 0;
}
