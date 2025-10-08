/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/


#include <cstdint>
#include "custom.hpp"
#include "cg_enums.h"
#include "StreamNode.hpp"
#include "cstream_node.h"
#include "IdentifiedNode.hpp"
#include "EventQueue.hpp"
#include "GenericNodes.hpp"
#include "AppNodes.hpp"
#include "scheduler.h"

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
static uint8_t schedule[8]=
{ 
3,6,1,4,5,2,7,0,
};

/*

Internal ID identification for the nodes

*/
#define AUDIOSINK_INTERNAL_ID 0
#define DEINTERLEAVE_INTERNAL_ID 1
#define INTERLEAVE_INTERNAL_ID 2
#define MIC_INTERNAL_ID 3
#define SRCL_INTERNAL_ID 4
#define SRCR_INTERNAL_ID 5
#define TOF32_INTERNAL_ID 6
#define TOQ15_INTERNAL_ID 7

/* Initialize the selectors global IDs in each class */
template<>
std::array<uint16_t,1> VStreamAudioSink<sq15,1536>::selectors = {SEL_VOLUME_ID};


/***********

Node identification

************/
static CStreamNode identifiedNodes[NB_IDENTIFIED_NODES]={0};

CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 512
#define FIFOSIZE1 512
#define FIFOSIZE2 512
#define FIFOSIZE3 512
#define FIFOSIZE4 1536
#define FIFOSIZE5 1536
#define FIFOSIZE6 1536
#define FIFOSIZE7 1536

#define BUFFERSIZE0 12288
CG_BEFORE_BUFFER
uint8_t buf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 6144
CG_BEFORE_BUFFER
uint8_t buf1[BUFFERSIZE1]={0};

#define BUFFERSIZE2 6144
CG_BEFORE_BUFFER
uint8_t buf2[BUFFERSIZE2]={0};


typedef struct {
FIFO<sq15,FIFOSIZE0,1,0> *fifo0;
FIFO<sf32,FIFOSIZE1,1,0> *fifo1;
FIFO<float,FIFOSIZE2,1,0> *fifo2;
FIFO<float,FIFOSIZE3,1,0> *fifo3;
FIFO<float,FIFOSIZE4,1,0> *fifo4;
FIFO<float,FIFOSIZE5,1,0> *fifo5;
FIFO<sf32,FIFOSIZE6,1,0> *fifo6;
FIFO<sq15,FIFOSIZE7,1,0> *fifo7;
} fifos_t;

typedef struct {
    VStreamAudioSink<sq15,1536> *audioSink;
    DeinterleaveStereo<sf32,512,float,512,float,512> *deinterleave;
    InterleaveStereo<float,1536,float,1536,sf32,1536> *interleave;
    VStreamAudioSource<sq15,512> *mic;
    SRC<float,512,float,1536> *srcL;
    SRC<float,512,float,1536> *srcR;
    Convert<sq15,512,sf32,512> *toF32;
    Convert<sf32,1536,sq15,1536> *toQ15;
} nodes_t;


static fifos_t fifos={0};

static nodes_t nodes={0};

CStreamNode* get_scheduler_node(int32_t nodeID)
{
    if (nodeID >= NB_IDENTIFIED_NODES)
    {
        return(nullptr);
    }
    if (nodeID < 0)
    {
        return(nullptr);
    }
    return(&identifiedNodes[nodeID]);
}

int init_scheduler()
{

    CG_BEFORE_FIFO_INIT;
    fifos.fifo0 = new (std::nothrow) FIFO<sq15,FIFOSIZE0,1,0>(buf0);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new (std::nothrow) FIFO<sf32,FIFOSIZE1,1,0>(buf1);
    if (fifos.fifo1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo2 = new (std::nothrow) FIFO<float,FIFOSIZE2,1,0>(buf2);
    if (fifos.fifo2==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo3 = new (std::nothrow) FIFO<float,FIFOSIZE3,1,0>(buf0);
    if (fifos.fifo3==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo4 = new (std::nothrow) FIFO<float,FIFOSIZE4,1,0>(buf1);
    if (fifos.fifo4==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo5 = new (std::nothrow) FIFO<float,FIFOSIZE5,1,0>(buf2);
    if (fifos.fifo5==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo6 = new (std::nothrow) FIFO<sf32,FIFOSIZE6,1,0>(buf0);
    if (fifos.fifo6==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo7 = new (std::nothrow) FIFO<sq15,FIFOSIZE7,1,0>(buf1);
    if (fifos.fifo7==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.audioSink = new (std::nothrow) VStreamAudioSink<sq15,1536>(*(fifos.fifo7),60,1);
    if (nodes.audioSink==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[AUDIOSINK_ID]=createStreamNode(*nodes.audioSink);
    nodes.audioSink->setID(AUDIOSINK_ID);

    nodes.deinterleave = new (std::nothrow) DeinterleaveStereo<sf32,512,float,512,float,512>(*(fifos.fifo1),*(fifos.fifo2),*(fifos.fifo3));
    if (nodes.deinterleave==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[DEINTERLEAVE_ID]=createStreamNode(*nodes.deinterleave);
    nodes.deinterleave->setID(DEINTERLEAVE_ID);

    nodes.interleave = new (std::nothrow) InterleaveStereo<float,1536,float,1536,sf32,1536>(*(fifos.fifo4),*(fifos.fifo5),*(fifos.fifo6));
    if (nodes.interleave==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[INTERLEAVE_ID]=createStreamNode(*nodes.interleave);
    nodes.interleave->setID(INTERLEAVE_ID);

    nodes.mic = new (std::nothrow) VStreamAudioSource<sq15,512>(*(fifos.fifo0),0);
    if (nodes.mic==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[MIC_ID]=createStreamNode(*nodes.mic);
    nodes.mic->setID(MIC_ID);

    nodes.srcL = new (std::nothrow) SRC<float,512,float,1536>(*(fifos.fifo2),*(fifos.fifo4));
    if (nodes.srcL==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[SRCL_ID]=createStreamNode(*nodes.srcL);
    nodes.srcL->setID(SRCL_ID);

    nodes.srcR = new (std::nothrow) SRC<float,512,float,1536>(*(fifos.fifo3),*(fifos.fifo5));
    if (nodes.srcR==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[SRCR_ID]=createStreamNode(*nodes.srcR);
    nodes.srcR->setID(SRCR_ID);

    nodes.toF32 = new (std::nothrow) Convert<sq15,512,sf32,512>(*(fifos.fifo0),*(fifos.fifo1));
    if (nodes.toF32==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[TOF32_ID]=createStreamNode(*nodes.toF32);
    nodes.toF32->setID(TOF32_ID);

    nodes.toQ15 = new (std::nothrow) Convert<sf32,1536,sq15,1536>(*(fifos.fifo6),*(fifos.fifo7));
    if (nodes.toQ15==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[TOQ15_ID]=createStreamNode(*nodes.toQ15);
    nodes.toQ15->setID(TOQ15_ID);


/* Subscribe nodes for the event system*/

    initError = CG_SUCCESS;
    initError = nodes.audioSink->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.deinterleave->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.interleave->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.mic->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.srcL->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.srcR->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.toF32->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.toQ15->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
   


    return(CG_SUCCESS);

}

void free_scheduler()
{
    if (fifos.fifo0!=NULL)
    {
       delete fifos.fifo0;
    }
    if (fifos.fifo1!=NULL)
    {
       delete fifos.fifo1;
    }
    if (fifos.fifo2!=NULL)
    {
       delete fifos.fifo2;
    }
    if (fifos.fifo3!=NULL)
    {
       delete fifos.fifo3;
    }
    if (fifos.fifo4!=NULL)
    {
       delete fifos.fifo4;
    }
    if (fifos.fifo5!=NULL)
    {
       delete fifos.fifo5;
    }
    if (fifos.fifo6!=NULL)
    {
       delete fifos.fifo6;
    }
    if (fifos.fifo7!=NULL)
    {
       delete fifos.fifo7;
    }

    if (nodes.audioSink!=NULL)
    {
        delete nodes.audioSink;
    }
    if (nodes.deinterleave!=NULL)
    {
        delete nodes.deinterleave;
    }
    if (nodes.interleave!=NULL)
    {
        delete nodes.interleave;
    }
    if (nodes.mic!=NULL)
    {
        delete nodes.mic;
    }
    if (nodes.srcL!=NULL)
    {
        delete nodes.srcL;
    }
    if (nodes.srcR!=NULL)
    {
        delete nodes.srcR;
    }
    if (nodes.toF32!=NULL)
    {
        delete nodes.toF32;
    }
    if (nodes.toQ15!=NULL)
    {
        delete nodes.toQ15;
    }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t scheduler(int *error)
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
        for(; id < 8; id++)
        {
            CG_BEFORE_NODE_EXECUTION(schedule[id]);
            switch(schedule[id])
            {
                case 0:
                {
                    
                   cgStaticError = nodes.audioSink->run();
                }
                break;

                case 1:
                {
                    
                   cgStaticError = nodes.deinterleave->run();
                }
                break;

                case 2:
                {
                    
                   cgStaticError = nodes.interleave->run();
                }
                break;

                case 3:
                {
                    
                   cgStaticError = nodes.mic->run();
                }
                break;

                case 4:
                {
                    
                   cgStaticError = nodes.srcL->run();
                }
                break;

                case 5:
                {
                    
                   cgStaticError = nodes.srcR->run();
                }
                break;

                case 6:
                {
                    
                   cgStaticError = nodes.toF32->run();
                }
                break;

                case 7:
                {
                    
                   cgStaticError = nodes.toQ15->run();
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
