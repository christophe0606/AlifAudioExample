#pragma once

#include CMSIS_device_header

#include <new>

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"

#include "dsp/support_functions.h"

#include "cmsis_os2.h"
#include "cmsis_vstream.h"

#include "custom.hpp"

using namespace arm_cmsis_stream;

#define VSTREAM_STEREO_BLOCK_COUNT (2)
#define VSTREAM_AUDIO_BLOCK_EVT (0x1)

extern vStreamDriver_t Driver_vStreamAudioIn;
#define vStream_AudioIn (&Driver_vStreamAudioIn)

extern "C"
{
    extern osThreadId_t tid_stream;
}

void AudioDrv_Event_Callback(uint32_t event)
{
    (void)event;

    osThreadFlagsSet(tid_stream, VSTREAM_AUDIO_BLOCK_EVT);
}

template <typename OUT, int outputSize>
class DebugSource;

template <int outputSamples>
class DebugSource<sq15, outputSamples>
    : public GenericSource<sq15, outputSamples>
{
  public:
    DebugSource(FIFOBase<sq15> &dst,
                int frequency = 440,
                int samplingFreq = 16000,
                bool master = false)
        : GenericSource<sq15, outputSamples>(dst), master_(master)
    {

        stereoBuffer = new (std::align_val_t(64)) sq15[VSTREAM_STEREO_BLOCK_COUNT * outputSamples];
        /* Initialize audio in stream and set the receive buffer */
        if (master_)
        {
            vStream_AudioIn->Initialize(AudioDrv_Event_Callback);
            vStream_AudioIn->SetBuf(stereoBuffer,
                                    VSTREAM_STEREO_BLOCK_COUNT * sizeof(sq15) * outputSamples,
                                    sizeof(sq15) * outputSamples);

            /* Start audio receiver */
            vStream_AudioIn->Start(VSTREAM_MODE_CONTINUOUS);
        }

        deltaPhaseFrequency = 3.141592f * 2 * frequency / samplingFreq;
        // 1 second period amplitude modulation
        deltaPhaseAmp = 3.141592f * 2 * 1 / samplingFreq;
    };

    ~DebugSource()
    {
        /* Stop audio receiver */
        if (master_)
        {
            vStream_AudioIn->Stop();
        }
        delete[] (stereoBuffer);
    };

    int prepareForRunning() final
    {
        if (this->willOverflow())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        sq15 *buf = nullptr;
        if (master_)
        {
            osThreadFlagsWait(VSTREAM_AUDIO_BLOCK_EVT, osFlagsWaitAny, osWaitForever);
            // Input audio block is read but ignore.
            // The debug source generates a known signal.
            buf = (sq15 *)vStream_AudioIn->GetBlock();
        }
        sq15 *out = this->getWriteBuffer();
        if (master_)
        {
            vStream_AudioIn->ReleaseBlock();
        }

        // Now we generate debug data
        for (int i = 0; i < outputSamples; i++)
        {
            // out[i].left = (0.3f * (cosf(phaseAmp) + 1.0f) * sinf(phaseFrequency) * 16384+0.5f);
            signal[i] = 1.0f * sinf(phaseFrequency);

            phaseFrequency += deltaPhaseFrequency;
            if (phaseFrequency >= 2 * 3.141592f)
                phaseFrequency -= 2 * 3.141592f;

            phaseAmp += deltaPhaseAmp;
            if (phaseAmp >= 2 * 3.141592f)
                phaseAmp -= 2 * 3.141592f;
        }

        arm_float_to_q15(signal, (q15_t *)out, 2 * outputSamples);

        return (CG_SUCCESS);
    };

  protected:
    float32_t signal[outputSamples];
    sq15 *stereoBuffer;
    float32_t phaseFrequency = 0.0f;
    float32_t deltaPhaseFrequency = 0.0f;
    float32_t phaseAmp = 0.0f;
    float32_t deltaPhaseAmp = 0.0f;
    bool master_;
};