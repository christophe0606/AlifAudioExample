#pragma once

#include CMSIS_device_header

#include <new>

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"

#include "cmsis_os2.h"
#include "cmsis_vstream.h"

using namespace arm_cmsis_stream;

#define VSTREAM_STEREO_BLOCK_COUNT (2)
#define VSTREAM_AUDIO_BLOCK_EVT 0x1

extern vStreamDriver_t Driver_vStreamAudioIn;
#define vStream_AudioIn (&Driver_vStreamAudioIn)

extern osThreadId_t tid_stream;

void AudioDrv_Event_Callback(uint32_t event) {
  (void)event;

  osThreadFlagsSet(tid_stream, VSTREAM_AUDIO_BLOCK_EVT);
}

template <typename OUT, int outputSize> class VStreamAudioSource;

template <int outputSamples>
class VStreamAudioSource<q15_t, outputSamples>
    : public GenericSource<q15_t, outputSamples> {
public:
  VStreamAudioSource(FIFOBase<q15_t> &dst)
      : GenericSource<q15_t, outputSamples>(dst) {

    stereoBuffer = new (std::align_val_t(64)) q15_t[outputSamples];
    /* Initialize audio in stream and set the receive buffer */
    vStream_AudioIn->Initialize(AudioDrv_Event_Callback);
    vStream_AudioIn->SetBuf(stereoBuffer,
                            VSTREAM_STEREO_BLOCK_COUNT * outputSamples,
                            outputSamples);

    /* Start audio receiver */
    vStream_AudioIn->Start(VSTREAM_MODE_CONTINUOUS);
  };

  ~VStreamAudioSource() {
    /* Stop audio receiver */
    vStream_AudioIn->Stop();
    delete[] (stereoBuffer);
  };

  int prepareForRunning() final {
    if (this->willOverflow()) {
      return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
    }

    return (0);
  };

  int run() final {
    osThreadFlagsWait(VSTREAM_AUDIO_BLOCK_EVT, osFlagsWaitAny, osWaitForever);
    q15_t *buf = (int16_t *)vStream_AudioIn->GetBlock();
    q15_t *out = this->getWriteBuffer();
    memcpy(out, buf, outputSamples * sizeof(q15_t));
    vStream_AudioIn->ReleaseBlock();

    return (CG_SUCCESS);
  };

protected:
  q15_t *stereoBuffer;
};