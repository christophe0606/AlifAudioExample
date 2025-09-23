#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"


using namespace arm_cmsis_stream;


template<typename IN,int inputSize,
         typename OUT,int outputSize>
class RealToComplex;

template<typename T,int inputSamples,int outputSamples>
class RealToComplex<T,inputSamples,T,outputSamples>: 
public GenericNode<T,inputSamples,T,outputSamples>
{
    static_assert(2*inputSamples==outputSamples,"RealToComplex: output size must be twice the input size");
public:
    RealToComplex(FIFOBase<T> &src,FIFOBase<T> &dst):
    GenericNode<T,inputSamples,T,outputSamples>(src,dst){};

    
    int prepareForRunning() final
    {
        if ((this->willOverflow()) || (this->willUnderflow()))
        {
           return(CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return(0);
    };

    int run() final{
        T *o=this->getWriteBuffer();
        T *in=this->getReadBuffer();
        for(int i=0;i<outputSamples;i++)
        {
           o[2*i]=in[i];
           o[2*i+1]=0;
        }

        
        return(CG_SUCCESS);
    };


};