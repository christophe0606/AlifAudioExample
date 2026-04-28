#include <stdlib.h>
#include <stdint.h>
#include "appa_params.h"


struct AppaParams appaParams = {
    // Initialize parameters for each node as needed
    // hw is common to all node and does not name a specific node
    .hw_ = {
        .i2s_mic = NULL // To be set to the I2S device
    },
    .audioSource = {
        .value = 0 // Example value
    }
};  