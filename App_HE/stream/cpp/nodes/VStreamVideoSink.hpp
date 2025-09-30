#pragma once

#include "RTE_Components.h"
#include "config.h"
#include "m-profile/armv7m_cachel1.h"

#include CMSIS_device_header

#include <atomic>

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

        vStreamStatus_t status;

        vStream_VideoOut->Initialize(VideoSink_Event_Callback);

        /* Set Input Video buffer */
        if (vStream_VideoOut->SetBuf(LCD_Frame, sizeof(LCD_Frame), DISPLAY_IMAGE_SIZE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to set buffer for video output\n");
        }

        /* Wait for video output frame to be released */
        do
        {
            status = vStream_VideoOut->GetStatus();
        } while (status.active == 1U);

        currentFrame = (uint16_t *)vStream_VideoOut->GetBlock();
        drawFrame();
        vStream_VideoOut->ReleaseBlock();
        currentFrame = (uint16_t *)vStream_VideoOut->GetBlock();
        drawFrame();
        if (vStream_VideoOut->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to start LCD output\n");
        }
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

    static constexpr int PADDING_LEFT = 10;
    static constexpr int PADDING_RIGHT = 10;
    static constexpr int PADDING_TOP = 10;
    static constexpr int PADDING_BOTTOM = 10;
    static constexpr int VERTICAL_SEPARATION = 10;

    void drawFrame()
    {

        if (currentFrame == nullptr)
            return;
        memset(currentFrame, 0xFF, DISPLAY_IMAGE_SIZE);
        for (int i = 0; i < DISPLAY_FRAME_WIDTH; i++)
        {
            for (int k = 0; k < 5; k++)
            {
                currentFrame[(50 + k) * DISPLAY_FRAME_WIDTH + i] = 0x0;
            }
        }
        /*
        for (int w = 0; w < DISPLAY_FRAME_WIDTH; w++)
        {
            for (int h = 0; h < DISPLAY_FRAME_HEIGHT; h++)
            {
                // *inFrame++ = 0x1F << 11; // Add red tint
            }
        }
            */
    }

    // The node was asked to render a new frame
    void renderNewFrame()
    {
        if (wasRendered.load())
            return; // Previous frame not yet displayed

        if (currentFrame == nullptr)
            return;

        drawFrame();

        // A new frame is pending to be displayed
        wasRendered.store(true);
    }

    void nextFrameBuffer()
    {
        SCB_CleanInvalidateDCache_by_Addr(currentFrame, DISPLAY_IMAGE_SIZE);
        if (vStream_VideoOut->ReleaseBlock() != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to release video input frame\n");
        }
        currentFrame = (uint16_t *)vStream_VideoOut->GetBlock();
        wasRendered.store(false);
    }

    void processEvent(int dstPort, Event &&evt) final
    {
        // Refresh LCD
        if (evt.event_id == kDo)
        {
            renderNewFrame();
            return;
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
                    return;
                }
            }

            // New left spectrogram
            if (dstPort == 0)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply(&VStreamVideoSink::processLeftSpectrogram, *this);
                    return;
                }
            }

            // New right spectrogram
            if (dstPort == 1)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply(&VStreamVideoSink::processRightSpectrogram, *this);
                    return;
                }
            }
        }
    }

    std::atomic<bool> wasRendered{true};

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

    uint16_t *currentFrame = nullptr;
};