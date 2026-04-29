#pragma once

#include "stream_platform_config.hpp"
#include CMSIS_device_header

#include <new>

#include "cg_enums.h"
#include "app_config.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"

#include "cmsis_os2.h"
#include "cmsis_vstream.h"
#include "rtos_events.hpp"
#include "datatypes.hpp"


using namespace arm_cmsis_stream;


extern vStreamDriver_t Driver_vStreamAudioIn;
#define vStream_AudioIn (&Driver_vStreamAudioIn)

extern "C" {
    extern osThreadId_t tid_stream;
}

template <typename OUT, int outputSize>
class VStreamAudioSource;

template <int outputSamples>
class VStreamAudioSource<sq15, outputSamples>
    : public GenericSource<sq15, outputSamples>,public ContextSwitch
{
    static_assert(AUDIO_BLOCK == outputSamples,
		      "The audio source output size must match AUDIO_BLOCK");

  public:
    
    VStreamAudioSource(FIFOBase<sq15> &dst,const struct hardwareParams &settings)
        : GenericSource<sq15, outputSamples>(dst),settings_(settings)
    {};

   
    ~VStreamAudioSource()
    {
    };

    int pause() final
	{
		// Implementation of pause
		if (started_.load() == false) {
			// If it was never started, nothing to do
			return 0;
		}
        int32_t rc = settings_.audio_src->Stop();
		if (rc != VSTREAM_OK) {
			CMSISSTREAM_LOG_ERR("I2S_TRIGGER_STOP failed: %i", rc);
		}
		started_.store(false);
		return 0;
	}

    int resume() final
	{
		// Implementation of resume
		return 0;
	}


    int run() final
	{
		size_t size;
		if (!started_.load()) {
			CMSISSTREAM_LOG_DBG("Starting audio\n");
			
			if (vStream_AudioIn->Start(VSTREAM_MODE_CONTINUOUS) != VSTREAM_OK)
            {
                 CMSISSTREAM_LOG_ERR("vStream_AudioIn Start error\n");
                 return(CG_INIT_FAILURE);
            }

			started_.store(true);
		}

        uint32_t res = osEventFlagsWait(settings_.audioSrcEvent, AUDIO_SOURCE_FRAME_EVENT|AUDIO_SOURCE_OVERFLOW_EVENT, osFlagsWaitAny, osWaitForever);
        if ((res & AUDIO_SOURCE_OVERFLOW_EVENT) != 0) {
            CMSISSTREAM_LOG_ERR("Audio source overflow detected\n");
            return(CG_BUFFER_OVERFLOW);
        }

        sq15 *buf = (sq15 *)settings_.audio_src->GetBlock();
		sq15 *out = this->getWriteBuffer();
		memset(out, 0, outputSamples * sizeof(sq15));
		

		if (buf == nullptr) {
			CMSISSTREAM_LOG_ERR("vStream getBlock failed");
            settings_.audio_src->Stop();
			return (CG_BUFFER_UNDERFLOW);
		}

        memcpy(out, buf, outputSamples * sizeof(sq15));
        settings_.audio_src->ReleaseBlock();
		return (CG_SUCCESS);
	};

    

  protected:
    std::atomic<bool> started_ = false;
	const struct hardwareParams &settings_;

};