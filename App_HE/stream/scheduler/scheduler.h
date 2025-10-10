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
#define NB_IDENTIFIED_NODES 10
#define AUDIOSOURCE_ID 0
#define AUDIOWIN_ID 1
#define DEINTERLEAVE_ID 2
#define MFCC_ID 3
#define MFCCWIN_ID 4
#define NULLRIGHT_ID 5
#define SEND_ID 6
#define TO_F32_ID 7
#define CLASSIFY_ID 8
#define KWS_ID 9


extern CStreamNode* get_scheduler_node(int32_t nodeID);


extern int init_scheduler();
extern void free_scheduler();

extern uint32_t scheduler(int *error);

#ifdef   __cplusplus
}
#endif

#endif

