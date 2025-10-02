#pragma once

#include CMSIS_device_header

#include <new>

#include "cg_enums.h"
#include "custom.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"

#include "cmsis_os2.h"
#include "cmsis_vstream.h"

using namespace arm_cmsis_stream;

#define VSTREAM_STEREO_SINK_BLOCK_COUNT (2)
#define VSTREAM_AUDIO_SINK_BLOCK_EVT (0x2)

extern vStreamDriver_t Driver_vStreamAudioOut;
#define vStream_AudioOut (&Driver_vStreamAudioOut)

extern "C"{
extern osThreadId_t tid_stream;
}

template <typename OUT, int outputSize>
class VStreamAudioSink;

template <int outputSamples>
class VStreamAudioSink<sq15, outputSamples>
    : public GenericSink<sq15, outputSamples>
{
  public:
    static void AudioSinkDrv_Event_Callback(uint32_t event)
    {
        (void)event;

        if (event & VSTREAM_EVENT_DATA)
        {
           if (tid_stream)
              osThreadFlagsSet(tid_stream, VSTREAM_AUDIO_SINK_BLOCK_EVT);
        }
    }

    VStreamAudioSink(FIFOBase<sq15> &dst)
        : GenericSink<sq15, outputSamples>(dst)
    {

        stereoBuffer = new (std::align_val_t(64)) sq15[VSTREAM_STEREO_SINK_BLOCK_COUNT * outputSamples];
        /* Initialize audio in stream and set the receive buffer */
        vStream_AudioOut->Initialize(AudioSinkDrv_Event_Callback);
        vStream_AudioOut->SetBuf(stereoBuffer,
                                 VSTREAM_STEREO_SINK_BLOCK_COUNT * sizeof(sq15) * outputSamples,
                                 sizeof(sq15) * outputSamples);

        vStream_AudioOut->GetBlock();
        vStream_AudioOut->ReleaseBlock();
    };

    ~VStreamAudioSink()
    {
        /* Stop audio receiver */
        vStream_AudioOut->Stop();
        delete[] (stereoBuffer);
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
        if (!started)
        {
            started = true;
            if (vStream_AudioOut->Start(VSTREAM_MODE_CONTINUOUS) != VSTREAM_OK)
            {
                 ERROR_PRINT("vStream_AudioOut Start error\n");
                 return(CG_INIT_FAILURE);
            }
        }
        osThreadFlagsWait(VSTREAM_AUDIO_SINK_BLOCK_EVT, osFlagsWaitAny, osWaitForever);
        sq15 *buf = (sq15 *)vStream_AudioOut->GetBlock();
        sq15 *input = this->getReadBuffer();
        if (buf)
            memcpy(buf, input, outputSamples * sizeof(sq15));
        vStream_AudioOut->ReleaseBlock();

        return (CG_SUCCESS);
    };

  protected:
    bool started{false};
    sq15 *stereoBuffer;
};