#pragma once



#include "cg_enums.h"
#include "custom.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"


using namespace arm_cmsis_stream;


template <typename OUT, int outputSize>
class NullSink;

template <int outputSamples>
class NullSink<sq15, outputSamples>
    : public GenericSink<sq15, outputSamples>
{
  public:
   
    NullSink(FIFOBase<sq15> &dst)
        : GenericSink<sq15, outputSamples>(dst)
    {

       
    };

    ~NullSink()
    {
        
    };

    int prepareForRunning() final
    {
        if (this->willUnderflow())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        sq15 *input = this->getReadBuffer();
        
        return (CG_SUCCESS);
    };

  
};