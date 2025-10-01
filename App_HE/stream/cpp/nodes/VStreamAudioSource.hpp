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

#define VSTREAM_STEREO_SOURCE_BLOCK_COUNT (2)
#define VSTREAM_AUDIO_SOURCE_BLOCK_EVT (0x1)

extern vStreamDriver_t Driver_vStreamAudioIn;
#define vStream_AudioIn (&Driver_vStreamAudioIn)

extern "C" {
    extern osThreadId_t tid_stream;
}

template <typename OUT, int outputSize>
class VStreamAudioSource;

template <int outputSamples>
class VStreamAudioSource<sq15, outputSamples>
    : public GenericSource<sq15, outputSamples>
{
  public:
    static void AudioSourceDrv_Event_Callback(uint32_t event)
    {
        (void)event;
        if (event & VSTREAM_EVENT_OVERFLOW)
        {
        }

        if (tid_stream != NULL)
           osThreadFlagsSet(tid_stream, VSTREAM_AUDIO_SOURCE_BLOCK_EVT);
    }

    VStreamAudioSource(FIFOBase<sq15> &dst)
        : GenericSource<sq15, outputSamples>(dst)
    {

        stereoBuffer = new (std::align_val_t(64)) sq15[VSTREAM_STEREO_SOURCE_BLOCK_COUNT * outputSamples];
        
        /* Initialize audio in stream and set the receive buffer */
        if (vStream_AudioIn->Initialize(AudioSourceDrv_Event_Callback) != VSTREAM_OK)
        {
            ERROR_PRINT("vStream_AudioIn Initialize error\n");
        }
        
        if (vStream_AudioIn->SetBuf(stereoBuffer,
                                VSTREAM_STEREO_SOURCE_BLOCK_COUNT * sizeof(sq15) * outputSamples,
                                sizeof(sq15) * outputSamples) != VSTREAM_OK)
        {
            ERROR_PRINT("vStream_AudioIn SetBuf error\n");
        }

        
    };

   
    ~VStreamAudioSource()
    {
        /* Stop audio receiver */
        vStream_AudioIn->Stop();
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
        osThreadFlagsWait(VSTREAM_AUDIO_SOURCE_BLOCK_EVT, osFlagsWaitAny, osWaitForever);
        
        sq15 *buf = (sq15 *)vStream_AudioIn->GetBlock();
        sq15 *out = this->getWriteBuffer();
        if (buf)
        {
            memcpy(out, buf, outputSamples * sizeof(sq15));
        }
        else 
        {
            memset(out, 0, outputSamples * sizeof(sq15));
        }
        vStream_AudioIn->ReleaseBlock();

        return (CG_SUCCESS);
    };

  protected:
    sq15 *stereoBuffer;
};