/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef SCHEDULER_H_ 
#define SCHEDULER_H_


#include <stdint.h>

#ifdef   __cplusplus
extern "C"
{
#endif

#include "cstream_node.h"


/* Node identifiers */
#define NB_IDENTIFIED_NODES 19
#define AUDIOSINK_ID 0
#define AUDIOSOURCE_ID 1
#define DEINTERLEAVE_ID 2
#define FFTLEFT_ID 3
#define FFTRIGHT_ID 4
#define INTERLEAVE_ID 5
#define MIXER_ID 6
#define SPECTROGRAMLEFT_ID 7
#define SPECTROGRAMRIGHT_ID 8
#define SRCLEFT_ID 9
#define SRCRIGHT_ID 10
#define TOCOMPLEXLEFT_ID 11
#define TOCOMPLEXRIGHT_ID 12
#define TO_F32_ID 13
#define TO_Q15_ID 14
#define WINLEFT_ID 15
#define WINRIGHT_ID 16
#define DISPLAY_ID 17
#define VIDEOSOURCE_ID 18

#define SCHED_LEN 19


extern CStreamNode* get_scheduler_node(int32_t nodeID);

extern int init_scheduler(void *evtQueue_);
extern void free_scheduler();
extern uint32_t scheduler(int *error);
extern void reset_fifos_scheduler(int all);

#ifdef   __cplusplus
}
#endif

#endif

