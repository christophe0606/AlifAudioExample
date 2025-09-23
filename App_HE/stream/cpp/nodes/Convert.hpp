#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "dsp/support_functions.h"

using namespace arm_cmsis_stream;


template<typename IN,int inputSize,
         typename OUT,int outputSize>
class Convert;

template<int inputSamples>
class Convert<q15_t,inputSamples,float32_t,inputSamples>: 
public GenericNode<q15_t,inputSamples,float32_t,inputSamples>
{
public:
    Convert(FIFOBase<q15_t> &src,FIFOBase<float32_t> &dst):
    GenericNode<q15_t,inputSamples,float32_t,inputSamples>(src,dst){};

    
    int prepareForRunning() final
    {
        if ((this->willOverflow()) || (this->willUnderflow()))
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };

    int run() final{
        float32_t *o=this->getWriteBuffer();
        q15_t *in=this->getReadBuffer();
        arm_q15_to_float(in, o, inputSamples);

        return(CG_SUCCESS);
    };


};