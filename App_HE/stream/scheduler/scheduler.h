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
#define NB_IDENTIFIED_NODES 12
#define AUDIOSOURCE_ID 0
#define FFTLEFT_ID 1
#define FFTRIGHT_ID 2
#define SPECTROGRAMLEFT_ID 3
#define SPECTROGRAMRIGHT_ID 4
#define SRC_F32_ID 5
#define STEREOTOMONO_ID 6
#define TOCOMPLEXLEFT_ID 7
#define TOCOMPLEXRIGHT_ID 8
#define WINLEFT_ID 9
#define WINRIGHT_ID 10
#define DISPLAY_ID 11


extern CStreamNode* get_scheduler_node(int32_t nodeID);


extern int init_scheduler();
extern void free_scheduler();

extern uint32_t scheduler(int *error);

#ifdef   __cplusplus
}
#endif

#endif

