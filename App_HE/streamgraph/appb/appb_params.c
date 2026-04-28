#include <stdlib.h>
#include <stdint.h>
#include "appb_params.h"


struct AppbParams appbParams = {
    // Initialize parameters for each node as needed
    // hw is common to all node and does not name a specific node
    .hw_ = {
        .audio_src = NULL, // This will be set in main.cpp after initializing the audio source driver
        .audioSrcEvent = 0   // This will be set in main.cpp after initializing
    },
    .audioSource = {
        .value = 0 // Example value
    }
};  