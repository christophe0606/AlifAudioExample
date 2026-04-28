#include <stdlib.h>
#include <stdint.h>
#include "appa_params.h"


struct AppaParams appaParams = {
    // Initialize parameters for each node as needed
    // hw is common to all node and does not name a specific node
    .hw_ = {
        .audio_src = NULL, // This will be set in main.cpp after initializing the audio source driver
        .audioSrcEvent = 0   // This will be set in main.cpp after initializing
    },
    .classify = {
        .historyLength = 10, // Example value
    },
    .kws = {
        .modelAddr = NULL, // To be set to the model address
        .modelSize = 0       // To be set to the model size
    }
};  