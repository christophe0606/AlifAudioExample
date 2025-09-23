#pragma once

#include "config.h"

#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "dsp/basic_math_functions.h"
#include "dsp/complex_math_functions.h"
#include <cstring>

using namespace arm_cmsis_stream;

template <typename IN, int inputSize>
class Spectrogram;

template <int inputSamples>
class Spectrogram<float32_t, inputSamples>
    : public GenericSink<float32_t, inputSamples>
{
  public:
    Spectrogram(FIFOBase<float32_t> &src)
        : GenericSink<float32_t, inputSamples>(src)
    {
        mag = new float32_t[inputSamples / 2];
    };

    ~Spectrogram()
    {
        delete[] mag;
    }

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
        float32_t *in = this->getReadBuffer();

        arm_cmplx_mag_f32(in, mag, inputSamples >> 1);

        arm_scale_f32(mag, 1.0f / (inputSamples >> 5), mag, inputSamples >> 1);

        float di = 1.0 * NB_BINS / ((float)inputSamples / 2);
        // float scale = 1.0f * FFT_SIZE / 2 / NB_BIN;
        float k = 0;
        memset(bins, 0, sizeof(bins));

        for (int i = 0; i < inputSamples / 2; i++)
        {
            if (k < NB_BINS)
                bins[(int)k] += mag[i];
            k += di;
        }

        for (int i = 0; i < NB_BINS; i++)
        {
            //   bins[i] *= scale;
            if (bins[i] > 1.0f)
                bins[i] = 1.0f;
            if (bins[i] < 0.0f)
                bins[i] = 0.0f;
        }

        UniquePtr<const float> tensorData(bins);

        TensorPtr<const float> t = TensorPtr<const float>::create_with((uint8_t)1,
                                                                           cg_tensor_dims_t{NB_BINS},
                                                                           std::move(tensorData));

        ev0.sendAsync(kNormalPriority, kValue, std::move(t)); // Send the event to the subscribed nodes

        return (CG_SUCCESS);
    };

    void subscribe(int outputPort, StreamNode &dst, int dstPort)
    {
        ev0.subscribe(dst, dstPort);
    }

  protected:
    float32_t *mag;
    float32_t bins[NB_BINS];
    EventOutput ev0;
};