#ifndef config_h
#define config_h

#define NB_BINS 128

#include <stdio.h>

#if 0
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)

#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#else 

#define DEBUG_PRINT(fmt, ...)
#define ERROR_PRINT(fmt, ...)
#endif 

#if 0
#define CG_BEFORE_NODE_EXECUTION(ID)                            \
    {                                                           \
        DEBUG_PRINT("Executing node ID=%lu\n", (unsigned long)(ID)); \
    }
#endif 

#endif