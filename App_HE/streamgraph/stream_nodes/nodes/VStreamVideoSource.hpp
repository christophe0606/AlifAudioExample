#pragma once

#include "RTE_Components.h"
#include "config.h"
#include "stream_runtime_config.hpp"

#include CMSIS_device_header

#include "EventQueue.hpp"
#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "app_config.hpp"

extern "C"
{
#include "cmsis_os2.h"
#include "cmsis_vstream.h"
#include "camera_config.h"
}

using namespace arm_cmsis_stream;

const osThreadAttr_t videoSrcAttr = {
    .stack_size = 4096,
    .priority = osPriorityHigh};

extern vStreamDriver_t Driver_vStreamVideoIn;
#define vStream_VideoIn (&Driver_vStreamVideoIn)

#define VSTREAM_VIDEO_SOURCE_BLOCK_EVT (0x1)

class VStreamVideoSource : public StreamNode, public ContextSwitch
{
  public:
    VStreamVideoSource(EventQueue *queue,const struct hardwareParams &settings)
        : StreamNode(),ev0(queue),settings_(settings),eventQueue(queue)
    {
    }

    ~VStreamVideoSource()
    {
        if (settings_.video_src->Stop() != VSTREAM_OK)
        {
            CMSISSTREAM_LOG_ERR("Failed to stop video input\n");
        }

    };

     int pause() final
	{
		// Implementation of pause
		if (started_.load() == false) {
			// If it was never started, nothing to do
			return 0;
		}
        int32_t rc = settings_.video_src->Stop();
		if (rc != VSTREAM_OK) {
			CMSISSTREAM_LOG_ERR("Video stop failed: %i", rc);
		}
		started_.store(false);
		paused_.store(true);
		return 0;
	}

    int resume() final
	{
        paused_.store(false);
        if (settings_.video_src->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK)
        {
                //CMSISSTREAM_LOG_ERR("Failed to start video capture\n");
        }
		Event evt(kDo, kNormalPriority);
        evt.setTTL(40);

        eventQueue->push(LocalDestination{this, 0}, std::move(evt));
		return 0;
	}

    void subscribe(int outputPort, StreamNode &dst, int dstPort)
    {
        ev0.subscribe(dst, dstPort);
    }

    static void release_video_frame(void *frame)
    {
        CMSISSTREAM_LOG_DBG("Release camera frame\n");
        if (vStream_VideoIn->ReleaseBlock() != VSTREAM_OK)
        {
            CMSISSTREAM_LOG_ERR("Failed to release video input frame\n");
        }
        else
        {
            vStreamStatus_t status;
            do
            {
                status = vStream_VideoIn->GetStatus();
            } while (status.active == 1U);

            if (vStream_VideoIn->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK)
            {
                //CMSISSTREAM_LOG_ERR("Failed to start video capture\n");
            }
        }
    }

    void processEvent(int dstPort, Event &&evt) final
    {
        if (paused_.load()) {
            return;
        }
        if (evt.event_id == kDo)
        {
            CMSISSTREAM_LOG_DBG("kDo for video source\n");
            uint8_t *inFrame = (uint8_t *)settings_.video_src->GetBlock();
            if (inFrame != nullptr)
            {
                //SCB_InvalidateDCache_by_Addr(inFrame, CAMERA_FRAME_SIZE);

                CMSISSTREAM_LOG_DBG("Send frame\n");
                UniquePtr<uint16_t> rgb_buf((uint16_t *)inFrame, release_video_frame);
                TensorPtr<uint16_t> t = TensorPtr<uint16_t>::create_with((uint16_t)2,
                                                                         cg_tensor_dims_t{CAMERA_FRAME_HEIGHT, CAMERA_FRAME_WIDTH},
                                                                         std::move(rgb_buf));

                ev0.sendSync(kHighPriority, kValue, std::move(t)); // Send the event to the subscribed nodes
                Event evt(kDo, kNormalPriority);
                evt.setTTL(40);

                eventQueue->push(LocalDestination{this, 0}, std::move(evt));
		
            }
            else
            {
                CMSISSTREAM_LOG_ERR("No camera frame available\n");
            }
        }
    }

protected:
    std::atomic<bool> started_ = false;
    std::atomic<bool> paused_ = true;
    const struct hardwareParams &settings_;
    EventQueue *eventQueue;
    EventOutput ev0;
};