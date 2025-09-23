#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"


using namespace arm_cmsis_stream;


template<typename IN,int inputSize,
         typename OUT1,int outputSize1,
         typename OUT2,int outputSize2>
class StereoToMono;

template<typename T,int inputSamples,int outputSamples>
class StereoToMono<T,inputSamples,T,outputSamples,T,outputSamples>: 
public GenericNode12<T,inputSamples,T,outputSamples,T,outputSamples>
{
    static_assert(inputSamples==2*outputSamples,"StereoToMono: input size must be twice the output size");
public:
    StereoToMono(FIFOBase<T> &src,FIFOBase<T> &left,FIFOBase<T> &right):
    GenericNode12<T,inputSamples,T,outputSamples,T,outputSamples>(src,left,right){};

    
    int prepareForRunning() final
    {
        if ((this->willOverflow1()) || (this->willOverflow2()) || (this->willUnderflow()))
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };

    int run() final{
        T *l=this->getWriteBuffer1();
        T *r=this->getWriteBuffer2();
        T *in=this->getReadBuffer();
        for(int i=0;i<outputSamples;i++)
        {
            l[i]=in[2*i];
            r[i]=in[2*i+1];
        }

        
        return(CG_SUCCESS);
    };


};