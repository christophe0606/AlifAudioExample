#pragma once



#include "cg_enums.h"
#include "app_config.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"


using namespace arm_cmsis_stream;


template <typename IN, int inputSamples>
class EmptySource: public GenericSource<IN, inputSamples>
{
  public:
   
    EmptySource(FIFOBase<IN> &src)
        : GenericSource<IN, inputSamples>(src)
    {

       
    };

    ~EmptySource()
    {
        
    };


    int run() final
    {
        IN *input = this->getWriteBuffer();
        
        return (CG_SUCCESS);
    };

  
};