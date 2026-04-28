#ifndef NODE_SETTINGS_DATATYPE_H
#define NODE_SETTINGS_DATATYPE_H

#include <stdint.h>
#include <stdlib.h>

#ifdef   __cplusplus
extern "C"
{
#endif

#if 0
typedef struct
{
    volatile uint32_t currentIndex;
    volatile uint32_t transmitIndex;
    uint32_t bufferSize;
    uint8_t *audioBuffer;
} audioOutputGlobalState_t;
#endif 

struct emptySourceParams
{
    int value;
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
   const void *i2s_mic;
};

#ifdef   __cplusplus
}
#endif

#endif