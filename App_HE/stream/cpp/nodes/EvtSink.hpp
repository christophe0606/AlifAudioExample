#pragma once

#include "StreamNode.hpp"
#include "GenericNodes.hpp"

extern "C"
{
    #include "config.h"
}
using namespace arm_cmsis_stream;

class EvtSink: public StreamNode
{
public:
    static std::array<uint16_t,2> selectors;

    // The constructor for the sink is only using
    // the input FIFO (coming from the generated scheduler).
    // This FIFO is passed to the GenericSink contructor.
    // Implementation of this Sink constructor is doing nothing
    EvtSink():StreamNode(),eventCount(0){};

    void processEvent(int dstPort,Event &&evt) final
    {
        eventCount++;
        
        
        // Display the event received from another node
        DEBUG_PRINT("Sink received event on port %d : evt id %d val = %d\n",dstPort ,evt.event_id ,eventCount);
    }

protected:
   int eventCount;

};