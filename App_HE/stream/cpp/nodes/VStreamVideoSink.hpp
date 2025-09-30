#pragma once

#include "RTE_Components.h"
#include "config.h"

#include CMSIS_device_header

#include <new>

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"

extern "C"
{
#include "cmsis_os2.h"
#include "cmsis_vstream.h"
#include "config.h"
}

using namespace arm_cmsis_stream;

extern vStreamDriver_t Driver_vStreamVideoOut;
#define vStream_VideoOut (&Driver_vStreamVideoOut)

class VStreamVideoSink : public StreamNode
{
  public:
    VStreamVideoSink()
        : StreamNode()
    {

        vStream_VideoOut->Initialize(VideoSink_Event_Callback);

        /* Set Input Video buffer */
        if (vStream_VideoOut->SetBuf(LCD_Frame, sizeof(LCD_Frame), DISPLAY_IMAGE_SIZE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to set buffer for video output\n");
        }

        renderFrame();
    }

    ~VStreamVideoSink()
    {
        if (vStream_VideoOut->Stop() != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to stop video output\n");
        }

        if (vStream_VideoOut->Uninitialize() != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to uninitialize video output\n");
        }
    };

    static void release_video_frame(void *frame)
    {
        if (vStream_VideoOut->ReleaseBlock() != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to release video input frame\n");
        }
        else
        {
            if (vStream_VideoOut->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK)
            {
                ERROR_PRINT("Failed to start video capture\n");
            }
        }
    }

    void renderFrame()
    {
        uint16_t *inFrame = (uint16_t *)vStream_VideoOut->GetBlock();
        if (inFrame != nullptr)
        {
            for (int w = 0; w < DISPLAY_FRAME_WIDTH; w++)
            {
                for (int h = 0; h < DISPLAY_FRAME_HEIGHT; h++)
                {
                    *inFrame++ = 0x07E0; // Add green tint
                }
            }
            release_video_frame(inFrame);
        }
    }

    void processEvent(int dstPort, Event &&evt) final
    {
        // Refresh LCD
        if (evt.event_id == kDo)
        {
            renderFrame();
        }

        // New camera frame or spectrogram
        if (evt.event_id == kValue)
        {
            // New camera frame
            if (dstPort == 2)
            {
                if (evt.wellFormed<TensorPtr<uint16_t>>())
                {
                    evt.apply(&VStreamVideoSink::processCameraFrame, *this);
                }
            }

            // New left spectrogram
            if (dstPort == 0)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply(&VStreamVideoSink::processLeftSpectrogram, *this);
                }
            }

            // New right spectrogram
            if (dstPort == 1)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply(&VStreamVideoSink::processRightSpectrogram, *this);
                }
            }
        }
    }

  protected:
    void processCameraFrame(TensorPtr<uint16_t> &&frame)
    {
    }

    void processLeftSpectrogram(TensorPtr<float> &&frame)
    {
    }

    void processRightSpectrogram(TensorPtr<float> &&frame)
    {
    }
};