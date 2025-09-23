
#include <cstdio>
#include "cg_enums.h"
#include "custom.hpp"
#include "cg_queue.hpp"
#include "rendering.h"



extern "C" {
#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
#include "main.h"
#include "disp.h"
#include "cstream_node.h"
#include "scheduler.h"
}

#include "cg_queue.hpp"


#define NB_MAX_EVENTS 5 
#define NB_MAX_BUFS 5 

using namespace arm_cmsis_stream;

osMemoryPoolId_t cg_eventPool = nullptr;
osMemoryPoolId_t cg_bufPool = nullptr;
osMemoryPoolId_t cg_mutexPool = nullptr;

osThreadId_t tid_display = NULL;
osThreadId_t tid_event = NULL;
osThreadId_t tid_stream = NULL;

osThreadId_t cg_eventThread = nullptr;




int  init_stream()
{
    arm_cmsis_stream::EventQueue::cg_eventQueue = new (std::nothrow) MyQueue();
    if (arm_cmsis_stream::EventQueue::cg_eventQueue == nullptr)
    {
        printf("Can't create CMSIS Event Queue\n");
        return 1;
    }

    printf("Create event pool\n");
    cg_eventPool = osMemoryPoolNew(NB_MAX_EVENTS, sizeof(ListValue) + 16, NULL);
    if (cg_eventPool == NULL)
    {
        printf("Can't create CMSIS Event Pool of size %d\n", NB_MAX_EVENTS * (sizeof(ListValue) + 16));
        delete arm_cmsis_stream::EventQueue::cg_eventQueue;
        arm_cmsis_stream::EventQueue::cg_eventQueue = nullptr;
        return 1;
    }

    printf("Create buf pool\n");
    cg_bufPool = osMemoryPoolNew(NB_MAX_BUFS, sizeof(Tensor<double>) + 16, NULL);
    if (cg_bufPool == NULL)
    {
        printf("Can't create CMSIS buf Pool of size %d\n", NB_MAX_BUFS * (sizeof(Tensor<double>) + 16));
        delete arm_cmsis_stream::EventQueue::cg_eventQueue;
        arm_cmsis_stream::EventQueue::cg_eventQueue = nullptr;
        osMemoryPoolDelete(cg_eventPool);
        return 1;
    }

    printf("Create mutex pool\n");
    cg_mutexPool = osMemoryPoolNew(NB_MAX_BUFS, sizeof(CG_MUTEX) + 16, NULL);
    if (cg_mutexPool == NULL)
    {
        printf("Can't create CMSIS mutex Pool of size %d\n", NB_MAX_BUFS * (sizeof(CG_MUTEX) + 16));
        delete arm_cmsis_stream::EventQueue::cg_eventQueue;
        arm_cmsis_stream::EventQueue::cg_eventQueue = nullptr;
        osMemoryPoolDelete(cg_eventPool);
        osMemoryPoolDelete(cg_bufPool);
        return 1;
    }

    printf("Create nodes\n");
    int error = init_scheduler();
    if (error == CG_MEMORY_ALLOCATION_FAILURE)
    {
        printf("Error: Memory allocation failure during scheduler initialization.\n");
        delete arm_cmsis_stream::EventQueue::cg_eventQueue;
        arm_cmsis_stream::EventQueue::cg_eventQueue = nullptr;
        osMemoryPoolDelete(cg_eventPool);
        osMemoryPoolDelete(cg_bufPool);
        osMemoryPoolDelete(cg_mutexPool);
        return 1;
    }

    return 0;


}

void display_thread(void *arg) {
  configure_display_and_2d();
  init_rendering();
  

  printf("Main thread started\n");


  for (;;) {
    render_data();
  }

endMain:
  osThreadExit();
}

void event_thread(void *argument)
{
    arm_cmsis_stream::EventQueue::cg_eventQueue->execute();
    

    // Delete the event queue
    delete arm_cmsis_stream::EventQueue::cg_eventQueue;
    arm_cmsis_stream::EventQueue::cg_eventQueue = nullptr;

    osMemoryPoolDelete(cg_eventPool);
    osMemoryPoolDelete(cg_bufPool);
    osMemoryPoolDelete(cg_mutexPool);

    osThreadExit();
}

void stream_thread(void *arg) 
{

   int error;
   uint32_t nb_iter = scheduler(&error);
   if (error != 0) 
   {
     printf("Scheduler error %d\n", error);
   }
   printf("Scheduler done after %d iterations\n", nb_iter);

   free_scheduler();
   osThreadExit();
}

int app_main(void) {

  const osThreadAttr_t mainAttr = {
      .stack_size = 4096,
  };

  const osThreadAttr_t audioAttr = {.stack_size = 4096,
                                    .priority = osPriorityRealtime};
  osKernelInitialize();

  int err = init_stream();
  if (err!= 0) {
    goto err;
  }
  tid_display = osThreadNew(display_thread, NULL, &mainAttr);
  tid_event = osThreadNew(event_thread, NULL, &mainAttr);
  tid_stream = osThreadNew(stream_thread, NULL, &audioAttr);


  osKernelStart();

err: return 0;
}
