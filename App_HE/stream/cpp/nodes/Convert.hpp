#pragma once

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"
#include "dsp/support_functions.h"


using namespace arm_cmsis_stream;

template <typename IN, int inputSize,
          typename OUT, int outputSize>
class Convert;

template <int inputSamples>
class Convert<sq15, inputSamples, sf32, inputSamples> : public GenericNode<sq15, inputSamples, sf32, inputSamples>
{
  public:
    Convert(FIFOBase<sq15> &src, FIFOBase<sf32> &dst)
        : GenericNode<sq15, inputSamples, sf32, inputSamples>(src, dst) {};

    int prepareForRunning() final
    {
        if ((this->willOverflow()) || (this->willUnderflow()))
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        sf32 *o = this->getWriteBuffer();
        sq15 *in = this->getReadBuffer();
        arm_q15_to_float((q15_t *)in, (float32_t *)o, 2 * inputSamples);

        return (CG_SUCCESS);
    };
};