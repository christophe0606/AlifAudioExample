#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "dsp/transform_functions.h"
#include <cstring>

using namespace arm_cmsis_stream;


template<typename IN,int inputSize,
         typename OUT,int outputSize>
class CFFT;

template<int inputSamples>
class CFFT<float32_t,inputSamples,float32_t,inputSamples>: 
public GenericNode<float32_t,inputSamples,float32_t,inputSamples>
{
public:
    CFFT(FIFOBase<float32_t> &src,FIFOBase<float32_t> &dst):
    GenericNode<float32_t,inputSamples,float32_t,inputSamples>(src,dst){
        if constexpr (inputSamples==256)
            arm_cfft_init_256_f32(&varInstCfftF32);
        else if constexpr (inputSamples==512)
            arm_cfft_init_512_f32(&varInstCfftF32);
        else if constexpr (inputSamples==1024)
            arm_cfft_init_1024_f32(&varInstCfftF32);
        else if constexpr (inputSamples==2048)
            arm_cfft_init_2048_f32(&varInstCfftF32);
        else if constexpr (inputSamples==4096)
            arm_cfft_init_4096_f32(&varInstCfftF32);
        else
            static_assert("Unsupported FFT size");
    };

    
    int prepareForRunning() final
    {
        if ((this->willOverflow()) || this->willUnderflow())
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };

    int run() final{
        float32_t *o=this->getWriteBuffer();
        float32_t *in=this->getReadBuffer();

        memcpy(o,in,sizeof(float32_t)*inputSamples);
        arm_cfft_f32(&varInstCfftF32, o, 0, 1);

        return(CG_SUCCESS);
    };

protected:
arm_cfft_instance_f32 varInstCfftF32;

};