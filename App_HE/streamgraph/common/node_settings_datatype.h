#ifndef NODE_SETTINGS_DATATYPE_H
#define NODE_SETTINGS_DATATYPE_H

#include <stdint.h>
#include <stdlib.h>


#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_vstream.h"
#include "cmsis_os2.h" /* CMSIS-RTOS2 API */


#ifdef   __cplusplus
extern "C"
{
#endif

struct emptySourceParams
{
    int value;
};

struct classifyParams
{
    int historyLength;
};


struct tfliteNodeParams
{
   uint8_t *modelAddr;
   size_t modelSize;
};

/**
 * @brief Structure to hold hardware connection parameters
 * for nodes that interact with hardware components.
 * The convention is that each parameter structure for a graph
 * starts with a hw_ field of type hardwareParams.
 */
struct hardwareParams
{
   const vStreamDriver_t *audio_src;
   osEventFlagsId_t audioSrcEvent;
};

#ifdef   __cplusplus
}
#endif

#endif