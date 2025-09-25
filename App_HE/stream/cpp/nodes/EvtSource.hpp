#pragma once

extern "C"
{
    #include "config.h"
}

#include "StreamNode.hpp"
#include "GenericNodes.hpp"

using namespace arm_cmsis_stream;

class EvtSource:public StreamNode
{
public:
    static std::array<uint16_t,1> selectors;

    EvtSource():StreamNode(){};

    void processEvent(int dstPort,Event &&evt) final
    {
        
        // Display the event received from another node
        DEBUG_PRINT("Source received event on port %d : evt id %d\n",dstPort ,evt.event_id);
        if (evt.contains<TensorPtr<float>>())
        {
            auto t = evt.get<TensorPtr<float>>();
            t.lock([](CG_MUTEX_ERROR_TYPE error,bool isShared, Tensor<float> &tensor) {
                if (CG_MUTEX_HAS_ERROR(error))
                {
                    ERROR_PRINT("Error locking tensor mutex\n");
                    return;
                }
                DEBUG_PRINT("TENSOR with %d dimensions: ", tensor.nb_dims);
                for (uint8_t i = 0; i < tensor.nb_dims; ++i)
                {
                    DEBUG_PRINT("%d ", tensor.dims[i]);
                }
                DEBUG_PRINT("\n");
            });
        }

        if (evt.contains<TensorPtr<const float>>())
        {
            auto t = evt.get<TensorPtr<const float>>();
            t.lock_shared([](CG_MUTEX_ERROR_TYPE error, const Tensor<const float> &tensor) {
                if (CG_MUTEX_HAS_ERROR(error))
                {
                    ERROR_PRINT("Error locking tensor mutex\n");
                    return;
                }
                DEBUG_PRINT("CONST TENSOR with %d dimensions: ", tensor.nb_dims);
                for (uint8_t i = 0; i < tensor.nb_dims; ++i)
                {
                    DEBUG_PRINT("%d ", tensor.dims[i]);
                }
                DEBUG_PRINT("\n");
            });
        }


        ev0.sendAsync(kNormalPriority,selectors[0],uint16_t(0)); // Send the event to the subscribed nodes
    }

    void subscribe(int outputPort,StreamNode &dst,int dstPort)
    {
        ev0.subscribe(dst,dstPort);
    }

protected:
   EventOutput ev0;
};