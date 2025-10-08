/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef _SCHEDULER_H_ 
#define _SCHEDULER_H_

#ifdef   __cplusplus
extern "C"
{
#endif


/* Node identifiers */
#define NB_IDENTIFIED_NODES 8
#define AUDIOSINK_ID 0
#define DEINTERLEAVE_ID 1
#define INTERLEAVE_ID 2
#define MIC_ID 3
#define SRCL_ID 4
#define SRCR_ID 5
#define TOF32_ID 6
#define TOQ15_ID 7

/* Selectors global identifiers */
#define SEL_VOLUME_ID 100 

extern CStreamNode* get_scheduler_node(int32_t nodeID);


extern int init_scheduler();
extern void free_scheduler();

extern uint32_t scheduler(int *error);

#ifdef   __cplusplus
}
#endif

#endif

