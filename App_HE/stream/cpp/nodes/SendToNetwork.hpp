#pragma once

extern "C"
{
#include "config.h"
}

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"
#include <cstring>


using namespace arm_cmsis_stream;

template <typename IN, int inputSamples>
class SendToNetwork : public GenericSink<IN, inputSamples>
{
  public:
    SendToNetwork(FIFOBase<IN> &src)
        : GenericSink<IN, inputSamples>(src) {
          };

    int prepareForRunning() final
    {
        if (this->willUnderflow())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() override final
    {
        IN *in = this->getReadBuffer();
        if (ready)
        {

            UniquePtr<IN> tensorData(inputSamples);
            memcpy(tensorData.get(), in, inputSamples * sizeof(IN));

            TensorPtr<IN> t = TensorPtr<IN>::create_with((uint8_t)1,
                                                         cg_tensor_dims_t{inputSamples},
                                                         std::move(tensorData));
            bool status = ev0.sendAsync(kNormalPriority, kValue, std::move(t)); // Send the event to the subscribed nodes

            if (!status)
            {
                ERROR_PRINT("Failed to send event to network\n");
            }
            else 
            {
                ready = false;
            }
        }

        return (CG_SUCCESS);
    };

    void processEvent(int dstPort, Event &&evt) final override
    {
        if (dstPort == 0)
        {
            if (evt.event_id == kDo)
            {
                ready = true;
            }
        }
    }

    void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
    {
        ev0.subscribe(dst, dstPort);
    }

  protected:
    bool ready{true};
    EventOutput ev0;
};