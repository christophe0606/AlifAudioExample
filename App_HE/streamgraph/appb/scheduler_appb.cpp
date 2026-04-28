/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/


#include <cstdint>
#include "app_config.hpp"
#include "stream_platform_config.hpp"
#include "cg_enums.h"
#include "StreamNode.hpp"
#include "cstream_node.h"
#include "IdentifiedNode.hpp"
#include "EventQueue.hpp"
#include "GenericNodes.hpp"
#include "AppNodes_appb.hpp"
#include "scheduler_appb.h"
#include "appb_extern_templates.hpp"

#if !defined(CHECKERROR)
#define CHECKERROR       if (cgStaticError < 0) \
       {\
         goto errorHandling;\
       }

#endif


#if !defined(CG_BEFORE_ITERATION)
#define CG_BEFORE_ITERATION
#endif 

#if !defined(CG_AFTER_ITERATION)
#define CG_AFTER_ITERATION
#endif 

#if !defined(CG_BEFORE_SCHEDULE)
#define CG_BEFORE_SCHEDULE
#endif

#if !defined(CG_AFTER_SCHEDULE)
#define CG_AFTER_SCHEDULE
#endif

#if !defined(CG_BEFORE_BUFFER)
#define CG_BEFORE_BUFFER
#endif

#if !defined(CG_BEFORE_FIFO_BUFFERS)
#define CG_BEFORE_FIFO_BUFFERS
#endif

#if !defined(CG_BEFORE_FIFO_INIT)
#define CG_BEFORE_FIFO_INIT
#endif

#if !defined(CG_BEFORE_NODE_INIT)
#define CG_BEFORE_NODE_INIT
#endif

#if !defined(CG_AFTER_INCLUDES)
#define CG_AFTER_INCLUDES
#endif

#if !defined(CG_BEFORE_SCHEDULER_FUNCTION)
#define CG_BEFORE_SCHEDULER_FUNCTION
#endif

#if !defined(CG_BEFORE_NODE_EXECUTION)
#define CG_BEFORE_NODE_EXECUTION(ID)
#endif

#if !defined(CG_AFTER_NODE_EXECUTION)
#define CG_AFTER_NODE_EXECUTION(ID)
#endif





CG_AFTER_INCLUDES


using namespace arm_cmsis_stream;

/*

Description of the scheduling. 

*/
static uint8_t schedule[2]=
{ 
0,1,
};

/*

Internal ID identification for the nodes

*/
#define AUDIOSOURCE_INTERNAL_ID 0
#define SINK_INTERNAL_ID 1



/***********

Node identification

************/
static CStreamNode identifiedNodes[STREAM_APPB_NB_IDENTIFIED_NODES]={0};

CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 320

#define BUFFERSIZE0 1280
CG_BEFORE_BUFFER
uint8_t stream_appb_buf0[BUFFERSIZE0]={0};


typedef struct {
FIFO<sq15,FIFOSIZE0,1,0> *fifo0;
} fifos_t;

typedef struct {
    VStreamAudioSource<sq15,320> *audioSource;
    NullSink<sq15,320> *sink;
} nodes_t;


static fifos_t fifos={0};

static nodes_t nodes={0};

CStreamNode* get_scheduler_appb_node(int32_t nodeID)
{
    if (nodeID >= STREAM_APPB_NB_IDENTIFIED_NODES)
    {
        return(nullptr);
    }
    if (nodeID < 0)
    {
        return(nullptr);
    }
    return(&identifiedNodes[nodeID]);
}

int init_scheduler_appb(void *evtQueue_,AppbParams *params)
{
    EventQueue *evtQueue = reinterpret_cast<EventQueue *>(evtQueue_);
    (void)evtQueue;

    CG_BEFORE_FIFO_INIT;
    fifos.fifo0 = new (std::nothrow) FIFO<sq15,FIFOSIZE0,1,0>(stream_appb_buf0);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.audioSource = new (std::nothrow) VStreamAudioSource<sq15,320>(*(fifos.fifo0),params->hw_);
    if (nodes.audioSource==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPB_AUDIOSOURCE_ID]=createStreamNode(*nodes.audioSource);
    nodes.audioSource->setID(STREAM_APPB_AUDIOSOURCE_ID);

    nodes.sink = new (std::nothrow) NullSink<sq15,320>(*(fifos.fifo0));
    if (nodes.sink==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APPB_SINK_ID]=createStreamNode(*nodes.sink);
    nodes.sink->setID(STREAM_APPB_SINK_ID);


/* Subscribe nodes for the event system*/

    initError = CG_SUCCESS;
    initError = nodes.audioSource->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.sink->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
   


    return(CG_SUCCESS);

}

void free_scheduler_appb()
{
    if (fifos.fifo0!=NULL)
    {
       delete fifos.fifo0;
    }

    if (nodes.audioSource!=NULL)
    {
        delete nodes.audioSource;
    }
    if (nodes.sink!=NULL)
    {
        delete nodes.sink;
    }
}

void reset_fifos_scheduler_appb(int all)
{
    if (fifos.fifo0!=NULL)
    {
       fifos.fifo0->reset();
    }
   // Buffers are set to zero too
   if (all)
   {
       std::fill_n(stream_appb_buf0, BUFFERSIZE0, (uint8_t)0);
   }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t scheduler_appb(int *error)
{
    int cgStaticError=0;
    uint32_t nbSchedule=0;






    /* Run several schedule iterations */
    CG_BEFORE_SCHEDULE;
    while(cgStaticError==0)
    {
        /* Run a schedule iteration */
        CG_BEFORE_ITERATION;
        unsigned long id=0;
        for(; id < 2; id++)
        {
            CG_BEFORE_NODE_EXECUTION(schedule[id]);
            switch(schedule[id])
            {
                case 0:
                {
                    
                   cgStaticError = nodes.audioSource->run();
                }
                break;

                case 1:
                {
                    
                   cgStaticError = nodes.sink->run();
                }
                break;

                default:
                break;
            }
            CG_AFTER_NODE_EXECUTION(schedule[id]);
                        CHECKERROR;
        }
       CG_AFTER_ITERATION;
       nbSchedule++;
    }
errorHandling:
    CG_AFTER_SCHEDULE;
    *error=cgStaticError;
    return(nbSchedule);
    
}
