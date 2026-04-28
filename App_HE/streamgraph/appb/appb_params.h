#ifndef APPB_PARAMS_H
#define APPB_PARAMS_H

#include "node_settings_datatype.h"

#ifdef   __cplusplus
extern "C"
{
#endif


struct AppbParams
{
    // Name of struct is the name of the node as defined
    // in Python graph.
    struct hardwareParams hw_;
    struct emptySourceParams audioSource;
};

extern struct AppbParams appbParams;

#ifdef   __cplusplus
}
#endif

#endif