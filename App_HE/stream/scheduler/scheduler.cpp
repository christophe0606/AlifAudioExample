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
static uint8_t schedule[17]=
{ 
1,13,2,3,15,11,5,9,4,8,7,14,0,16,12,6,10,
};

/*

Internal ID identification for the nodes

*/
#define AUDIOSINK_INTERNAL_ID 0
#define AUDIOSOURCE_INTERNAL_ID 1
#define DEINTERLEAVE_INTERNAL_ID 2
#define DUP0_INTERNAL_ID 3
#define DUP1_INTERNAL_ID 4
#define FFTLEFT_INTERNAL_ID 5
#define FFTRIGHT_INTERNAL_ID 6
#define INTERLEAVE_INTERNAL_ID 7
#define MIXER_INTERNAL_ID 8
#define SPECTROGRAMLEFT_INTERNAL_ID 9
#define SPECTROGRAMRIGHT_INTERNAL_ID 10
#define TOCOMPLEXLEFT_INTERNAL_ID 11
#define TOCOMPLEXRIGHT_INTERNAL_ID 12
#define TO_F32_INTERNAL_ID 13
#define TO_Q15_INTERNAL_ID 14
#define WINLEFT_INTERNAL_ID 15
#define WINRIGHT_INTERNAL_ID 16
#define DISPLAY_INTERNAL_ID 17
#define VIDEOSOURCE_INTERNAL_ID 18



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
#define FIFOSIZE4 512
#define FIFOSIZE5 512
#define FIFOSIZE6 512
#define FIFOSIZE7 512
#define FIFOSIZE8 512
#define FIFOSIZE9 512
#define FIFOSIZE10 512
#define FIFOSIZE11 512
#define FIFOSIZE12 512
#define FIFOSIZE13 512
#define FIFOSIZE14 512
#define FIFOSIZE15 512
#define FIFOSIZE16 512
#define FIFOSIZE17 512

#define BUFFERSIZE0 4096
CG_BEFORE_BUFFER
uint8_t buf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 4096
CG_BEFORE_BUFFER
uint8_t buf1[BUFFERSIZE1]={0};

#define BUFFERSIZE2 4096
CG_BEFORE_BUFFER
uint8_t buf2[BUFFERSIZE2]={0};

#define BUFFERSIZE3 4096
CG_BEFORE_BUFFER
uint8_t buf3[BUFFERSIZE3]={0};


typedef struct {
FIFO<sq15,FIFOSIZE0,1,0> *fifo0;
FIFO<sf32,FIFOSIZE1,1,0> *fifo1;
FIFO<float,FIFOSIZE2,1,0> *fifo2;
FIFO<float,FIFOSIZE3,1,0> *fifo3;
FIFO<cf32,FIFOSIZE4,1,0> *fifo4;
FIFO<cf32,FIFOSIZE5,1,0> *fifo5;
FIFO<cf32,FIFOSIZE6,1,0> *fifo6;
FIFO<cf32,FIFOSIZE7,1,0> *fifo7;
FIFO<float,FIFOSIZE8,1,0> *fifo8;
FIFO<float,FIFOSIZE9,1,0> *fifo9;
FIFO<sf32,FIFOSIZE10,1,0> *fifo10;
FIFO<sq15,FIFOSIZE11,1,0> *fifo11;
FIFO<float,FIFOSIZE12,1,0> *fifo12;
FIFO<float,FIFOSIZE13,1,0> *fifo13;
FIFO<float,FIFOSIZE14,1,0> *fifo14;
FIFO<float,FIFOSIZE15,1,0> *fifo15;
FIFO<float,FIFOSIZE16,1,0> *fifo16;
FIFO<float,FIFOSIZE17,1,0> *fifo17;
} fifos_t;

typedef struct {
    VStreamAudioSink<sq15,512> *audioSink;
    VStreamAudioSource<sq15,512> *audioSource;
    DeinterleaveStereo<sf32,512,float,512,float,512> *deinterleave;
    Duplicate<float,512,float,512> *dup0;
    Duplicate<float,512,float,512> *dup1;
    CFFT<cf32,512,cf32,512> *fftLeft;
    CFFT<cf32,512,cf32,512> *fftRight;
    InterleaveStereo<float,512,float,512,sf32,512> *interleave;
    Mixer<float,512,float,512,float,512,float,512> *mixer;
    Spectrogram<cf32,512> *spectrogramLeft;
    Spectrogram<cf32,512> *spectrogramRight;
    RealToComplex<float,512,cf32,512> *toComplexLeft;
    RealToComplex<float,512,cf32,512> *toComplexRight;
    Convert<sq15,512,sf32,512> *to_f32;
    Convert<sf32,512,sq15,512> *to_q15;
    Hanning<float,512,float,512> *winLeft;
    Hanning<float,512,float,512> *winRight;
    AppDisplay *display;
    VStreamVideoSource *videoSource;
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
    fifos.fifo0 = new (std::nothrow) FIFO<sq15,FIFOSIZE0,1,0>(buf1);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new (std::nothrow) FIFO<sf32,FIFOSIZE1,1,0>(buf0);
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
    fifos.fifo4 = new (std::nothrow) FIFO<cf32,FIFOSIZE4,1,0>(buf3);
    if (fifos.fifo4==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo5 = new (std::nothrow) FIFO<cf32,FIFOSIZE5,1,0>(buf1);
    if (fifos.fifo5==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo6 = new (std::nothrow) FIFO<cf32,FIFOSIZE6,1,0>(buf2);
    if (fifos.fifo6==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo7 = new (std::nothrow) FIFO<cf32,FIFOSIZE7,1,0>(buf0);
    if (fifos.fifo7==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo8 = new (std::nothrow) FIFO<float,FIFOSIZE8,1,0>(buf2);
    if (fifos.fifo8==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo9 = new (std::nothrow) FIFO<float,FIFOSIZE9,1,0>(buf3);
    if (fifos.fifo9==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo10 = new (std::nothrow) FIFO<sf32,FIFOSIZE10,1,0>(buf0);
    if (fifos.fifo10==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo11 = new (std::nothrow) FIFO<sq15,FIFOSIZE11,1,0>(buf2);
    if (fifos.fifo11==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo12 = new (std::nothrow) FIFO<float,FIFOSIZE12,1,0>(buf2);
    if (fifos.fifo12==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo13 = new (std::nothrow) FIFO<float,FIFOSIZE13,1,0>(buf0);
    if (fifos.fifo13==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo14 = new (std::nothrow) FIFO<float,FIFOSIZE14,1,0>(buf0);
    if (fifos.fifo14==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo15 = new (std::nothrow) FIFO<float,FIFOSIZE15,1,0>(buf1);
    if (fifos.fifo15==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo16 = new (std::nothrow) FIFO<float,FIFOSIZE16,1,0>(buf1);
    if (fifos.fifo16==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo17 = new (std::nothrow) FIFO<float,FIFOSIZE17,1,0>(buf1);
    if (fifos.fifo17==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.audioSink = new (std::nothrow) VStreamAudioSink<sq15,512>(*(fifos.fifo11),5);
    if (nodes.audioSink==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[AUDIOSINK_ID]=createStreamNode(*nodes.audioSink);
    nodes.audioSink->setID(AUDIOSINK_ID);
    initError = nodes.audioSink->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.audioSource = new (std::nothrow) VStreamAudioSource<sq15,512>(*(fifos.fifo0));
    if (nodes.audioSource==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[AUDIOSOURCE_ID]=createStreamNode(*nodes.audioSource);
    nodes.audioSource->setID(AUDIOSOURCE_ID);
    initError = nodes.audioSource->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.deinterleave = new (std::nothrow) DeinterleaveStereo<sf32,512,float,512,float,512>(*(fifos.fifo1),*(fifos.fifo12),*(fifos.fifo15));
    if (nodes.deinterleave==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[DEINTERLEAVE_ID]=createStreamNode(*nodes.deinterleave);
    nodes.deinterleave->setID(DEINTERLEAVE_ID);
    initError = nodes.deinterleave->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.dup0 = new (std::nothrow) Duplicate<float,512,float,512>(*(fifos.fifo12),{fifos.fifo13});
    if (nodes.dup0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    initError = nodes.dup0->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.dup1 = new (std::nothrow) Duplicate<float,512,float,512>(*(fifos.fifo15),{});
    if (nodes.dup1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    initError = nodes.dup1->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.fftLeft = new (std::nothrow) CFFT<cf32,512,cf32,512>(*(fifos.fifo4),*(fifos.fifo6));
    if (nodes.fftLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[FFTLEFT_ID]=createStreamNode(*nodes.fftLeft);
    nodes.fftLeft->setID(FFTLEFT_ID);
    initError = nodes.fftLeft->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.fftRight = new (std::nothrow) CFFT<cf32,512,cf32,512>(*(fifos.fifo5),*(fifos.fifo7));
    if (nodes.fftRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[FFTRIGHT_ID]=createStreamNode(*nodes.fftRight);
    nodes.fftRight->setID(FFTRIGHT_ID);
    initError = nodes.fftRight->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.interleave = new (std::nothrow) InterleaveStereo<float,512,float,512,sf32,512>(*(fifos.fifo8),*(fifos.fifo9),*(fifos.fifo10));
    if (nodes.interleave==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[INTERLEAVE_ID]=createStreamNode(*nodes.interleave);
    nodes.interleave->setID(INTERLEAVE_ID);
    initError = nodes.interleave->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.mixer = new (std::nothrow) Mixer<float,512,float,512,float,512,float,512>(*(fifos.fifo14),*(fifos.fifo17),*(fifos.fifo8),*(fifos.fifo9));
    if (nodes.mixer==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[MIXER_ID]=createStreamNode(*nodes.mixer);
    nodes.mixer->setID(MIXER_ID);
    initError = nodes.mixer->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.spectrogramLeft = new (std::nothrow) Spectrogram<cf32,512>(*(fifos.fifo6));
    if (nodes.spectrogramLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[SPECTROGRAMLEFT_ID]=createStreamNode(*nodes.spectrogramLeft);
    nodes.spectrogramLeft->setID(SPECTROGRAMLEFT_ID);
    initError = nodes.spectrogramLeft->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.spectrogramRight = new (std::nothrow) Spectrogram<cf32,512>(*(fifos.fifo7));
    if (nodes.spectrogramRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[SPECTROGRAMRIGHT_ID]=createStreamNode(*nodes.spectrogramRight);
    nodes.spectrogramRight->setID(SPECTROGRAMRIGHT_ID);
    initError = nodes.spectrogramRight->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.toComplexLeft = new (std::nothrow) RealToComplex<float,512,cf32,512>(*(fifos.fifo2),*(fifos.fifo4));
    if (nodes.toComplexLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[TOCOMPLEXLEFT_ID]=createStreamNode(*nodes.toComplexLeft);
    nodes.toComplexLeft->setID(TOCOMPLEXLEFT_ID);
    initError = nodes.toComplexLeft->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.toComplexRight = new (std::nothrow) RealToComplex<float,512,cf32,512>(*(fifos.fifo3),*(fifos.fifo5));
    if (nodes.toComplexRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[TOCOMPLEXRIGHT_ID]=createStreamNode(*nodes.toComplexRight);
    nodes.toComplexRight->setID(TOCOMPLEXRIGHT_ID);
    initError = nodes.toComplexRight->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.to_f32 = new (std::nothrow) Convert<sq15,512,sf32,512>(*(fifos.fifo0),*(fifos.fifo1));
    if (nodes.to_f32==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[TO_F32_ID]=createStreamNode(*nodes.to_f32);
    nodes.to_f32->setID(TO_F32_ID);
    initError = nodes.to_f32->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.to_q15 = new (std::nothrow) Convert<sf32,512,sq15,512>(*(fifos.fifo10),*(fifos.fifo11));
    if (nodes.to_q15==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[TO_Q15_ID]=createStreamNode(*nodes.to_q15);
    nodes.to_q15->setID(TO_Q15_ID);
    initError = nodes.to_q15->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.winLeft = new (std::nothrow) Hanning<float,512,float,512>(*(fifos.fifo13),*(fifos.fifo2));
    if (nodes.winLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[WINLEFT_ID]=createStreamNode(*nodes.winLeft);
    nodes.winLeft->setID(WINLEFT_ID);
    initError = nodes.winLeft->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.winRight = new (std::nothrow) Hanning<float,512,float,512>(*(fifos.fifo16),*(fifos.fifo3));
    if (nodes.winRight==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[WINRIGHT_ID]=createStreamNode(*nodes.winRight);
    nodes.winRight->setID(WINRIGHT_ID);
    initError = nodes.winRight->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.display = new (std::nothrow) AppDisplay;
    if (nodes.display==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[DISPLAY_ID]=createStreamNode(*nodes.display);
    nodes.display->setID(DISPLAY_ID);
    initError = nodes.display->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }

    nodes.videoSource = new (std::nothrow) VStreamVideoSource;
    if (nodes.videoSource==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[VIDEOSOURCE_ID]=createStreamNode(*nodes.videoSource);
    nodes.videoSource->setID(VIDEOSOURCE_ID);
    initError = nodes.videoSource->init();
    if (initError != CG_SUCCESS)
    {
        return(initError);
    }


/* Subscribe nodes for the event system*/
    nodes.spectrogramLeft->subscribe(0,*nodes.display,0);
    nodes.spectrogramRight->subscribe(0,*nodes.display,1);
    nodes.videoSource->subscribe(0,*nodes.display,2);


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
    if (fifos.fifo8!=NULL)
    {
       delete fifos.fifo8;
    }
    if (fifos.fifo9!=NULL)
    {
       delete fifos.fifo9;
    }
    if (fifos.fifo10!=NULL)
    {
       delete fifos.fifo10;
    }
    if (fifos.fifo11!=NULL)
    {
       delete fifos.fifo11;
    }
    if (fifos.fifo12!=NULL)
    {
       delete fifos.fifo12;
    }
    if (fifos.fifo13!=NULL)
    {
       delete fifos.fifo13;
    }
    if (fifos.fifo14!=NULL)
    {
       delete fifos.fifo14;
    }
    if (fifos.fifo15!=NULL)
    {
       delete fifos.fifo15;
    }
    if (fifos.fifo16!=NULL)
    {
       delete fifos.fifo16;
    }
    if (fifos.fifo17!=NULL)
    {
       delete fifos.fifo17;
    }

    if (nodes.audioSink!=NULL)
    {
        delete nodes.audioSink;
    }
    if (nodes.audioSource!=NULL)
    {
        delete nodes.audioSource;
    }
    if (nodes.deinterleave!=NULL)
    {
        delete nodes.deinterleave;
    }
    if (nodes.dup0!=NULL)
    {
        delete nodes.dup0;
    }
    if (nodes.dup1!=NULL)
    {
        delete nodes.dup1;
    }
    if (nodes.fftLeft!=NULL)
    {
        delete nodes.fftLeft;
    }
    if (nodes.fftRight!=NULL)
    {
        delete nodes.fftRight;
    }
    if (nodes.interleave!=NULL)
    {
        delete nodes.interleave;
    }
    if (nodes.mixer!=NULL)
    {
        delete nodes.mixer;
    }
    if (nodes.spectrogramLeft!=NULL)
    {
        delete nodes.spectrogramLeft;
    }
    if (nodes.spectrogramRight!=NULL)
    {
        delete nodes.spectrogramRight;
    }
    if (nodes.toComplexLeft!=NULL)
    {
        delete nodes.toComplexLeft;
    }
    if (nodes.toComplexRight!=NULL)
    {
        delete nodes.toComplexRight;
    }
    if (nodes.to_f32!=NULL)
    {
        delete nodes.to_f32;
    }
    if (nodes.to_q15!=NULL)
    {
        delete nodes.to_q15;
    }
    if (nodes.winLeft!=NULL)
    {
        delete nodes.winLeft;
    }
    if (nodes.winRight!=NULL)
    {
        delete nodes.winRight;
    }
    if (nodes.display!=NULL)
    {
        delete nodes.display;
    }
    if (nodes.videoSource!=NULL)
    {
        delete nodes.videoSource;
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
        for(; id < 17; id++)
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
                    
                   cgStaticError = nodes.audioSource->run();
                }
                break;

                case 2:
                {
                    
                   cgStaticError = nodes.deinterleave->run();
                }
                break;

                case 3:
                {
                    
                   cgStaticError = nodes.dup0->run();
                }
                break;

                case 4:
                {
                    
                   cgStaticError = nodes.dup1->run();
                }
                break;

                case 5:
                {
                    
                   cgStaticError = nodes.fftLeft->run();
                }
                break;

                case 6:
                {
                    
                   cgStaticError = nodes.fftRight->run();
                }
                break;

                case 7:
                {
                    
                   cgStaticError = nodes.interleave->run();
                }
                break;

                case 8:
                {
                    
                   cgStaticError = nodes.mixer->run();
                }
                break;

                case 9:
                {
                    
                   cgStaticError = nodes.spectrogramLeft->run();
                }
                break;

                case 10:
                {
                    
                   cgStaticError = nodes.spectrogramRight->run();
                }
                break;

                case 11:
                {
                    
                   cgStaticError = nodes.toComplexLeft->run();
                }
                break;

                case 12:
                {
                    
                   cgStaticError = nodes.toComplexRight->run();
                }
                break;

                case 13:
                {
                    
                   cgStaticError = nodes.to_f32->run();
                }
                break;

                case 14:
                {
                    
                   cgStaticError = nodes.to_q15->run();
                }
                break;

                case 15:
                {
                    
                   cgStaticError = nodes.winLeft->run();
                }
                break;

                case 16:
                {
                    
                   cgStaticError = nodes.winRight->run();
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
