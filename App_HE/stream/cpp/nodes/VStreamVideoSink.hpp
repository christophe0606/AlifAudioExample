#pragma once

#include "RTE_Components.h"
#include "config.h"
#include "m-profile/armv7m_cachel1.h"
#include <utility>
#include <variant>

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
        SCB_CleanInvalidateDCache_by_Addr(LCD_Frame, 2 * DISPLAY_IMAGE_SIZE);
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
    static constexpr int HORIZONTAL_SEPARATION = 10;

    void fillRectangle(int x, int y, int width, int height, uint16_t color)
    {
        if (currentFrame == nullptr)
            return;
        if (x < 0)
        {
            width += x;
            x = 0;
        }
        if (y < 0)
        {
            height += y;
            y = 0;
        }
        if (width + x > DISPLAY_FRAME_WIDTH)
        {
            width = DISPLAY_FRAME_WIDTH - x;
        }
        if (height + y > DISPLAY_FRAME_HEIGHT)
        {
            height = DISPLAY_FRAME_HEIGHT - y;
        }
        if (width <= 0)
            return;
        if (height <= 0)
            return;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                int px = x + j;
                int py = y + i;
                currentFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
    }

    void strokeRectangle(int x, int y, int width, int height, uint16_t color)
    {
        if (currentFrame == nullptr)
            return;
        bool drawTop = true;
        bool drawBottom = true;
        bool drawLeft = true;
        bool drawRight = true;
        if (x < 0)
        {
            width += x;
            x = 0;
            drawLeft = false;
        }
        if (y < 0)
        {
            height += y;
            y = 0;
            drawTop = false;
        }
        if (width + x > DISPLAY_FRAME_WIDTH)
        {
            width = DISPLAY_FRAME_WIDTH - x;
            drawRight = false;
        }
        if (height + y > DISPLAY_FRAME_HEIGHT)
        {
            height = DISPLAY_FRAME_HEIGHT - y;
            drawBottom = false;
        }

        if (width <= 0)
            return;
        if (height <= 0)
            return;
        if (drawTop)
        {
            for (int j = 0; j < width; j++)
            {
                int px = x + j;
                int py = y;
                currentFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
        if (drawBottom)
        {
            for (int j = 0; j < width; j++)
            {
                int px = x + j;
                int py = y + height - 1;
                currentFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
        if (drawLeft)
        {
            for (int i = 0; i < height; i++)
            {
                int px = x;
                int py = y + i;
                currentFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
        if (drawRight)
        {
            for (int i = 0; i < height; i++)
            {
                int px = x + width - 1;
                int py = y + i;
                currentFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
    }

    void drawFrame()
    {

        if (currentFrame == nullptr)
            return;
        memset(currentFrame, 0xFF, DISPLAY_IMAGE_SIZE);
        const int boxWidth = (DISPLAY_FRAME_WIDTH - PADDING_LEFT - PADDING_RIGHT - HORIZONTAL_SEPARATION) / 2;
        const int boxHeight = DISPLAY_FRAME_HEIGHT - PADDING_TOP - PADDING_BOTTOM;

        strokeRectangle(PADDING_LEFT, PADDING_TOP, boxWidth, boxHeight, 0x00);
        strokeRectangle(PADDING_LEFT + boxWidth + HORIZONTAL_SEPARATION, PADDING_TOP, boxWidth, boxHeight, 0x00);
        fillRectangle(0,0,CAMERA_FRAME_WIDTH,CAMERA_FRAME_HEIGHT,0x03F << 5);
        if (hasCameraFrame)
        {
#if 0
            currentCameraFrame.lock_shared([this](CG_MUTEX_ERROR_TYPE error, const Tensor<uint16_t> &tensor)
                                           {
            if (!CG_MUTEX_HAS_ERROR(error))
            {
               
            
             if (std::holds_alternative<UniquePtr<uint16_t>>(tensor.data))
                {
                const UniquePtr<uint16_t> &buf = std::get<UniquePtr<uint16_t>>(tensor.data);
                for (int h = 0; h < tensor.dims[0]>>1; h++)
                {
                    for (int w = 0; w < tensor.dims[1]>>1; w++)
                    {
                        currentFrame[w + h * DISPLAY_FRAME_WIDTH] = buf.get()[(w<<1) + (h<<1) * tensor.dims[1]];
                    }
                }
            } }
             hasCameraFrame = false;
        });
           
#endif
            currentCameraFrame = std::move(TensorPtr<uint16_t>()); // Release the frame so that camera get new one
        }
    }

    // The node was asked to render a new frame
    void
    renderNewFrame()
    {
        // Was rendered is still true so new buffer has
        // not yet been taken into account
        if (wasRendered.load())
            return; // Previous frame not yet displayed

        if (currentFrame == nullptr)
            return;

        drawFrame();

        SCB_CleanInvalidateDCache_by_Addr(currentFrame, DISPLAY_IMAGE_SIZE);

        // A new frame is pending to be displayed
        wasRendered.store(true);
    }

    void nextFrameBuffer()
    {
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
                    evt.apply<TensorPtr<uint16_t>>(&VStreamVideoSink::processCameraFrame, *this);
                    return;
                }
            }

            // New left spectrogram
            if (dstPort == 0)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&VStreamVideoSink::processLeftSpectrogram, *this);
                    return;
                }
            }

            // New right spectrogram
            if (dstPort == 1)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&VStreamVideoSink::processRightSpectrogram, *this);
                    return;
                }
            }
        }
    }

    std::atomic<bool> wasRendered{true};

  protected:
    void processCameraFrame(TensorPtr<uint16_t> &&frame)
    {
        hasCameraFrame = true;
        currentCameraFrame = std::move(frame);
    }

    void processLeftSpectrogram(TensorPtr<float> &&frame)
    {
    }

    void processRightSpectrogram(TensorPtr<float> &&frame)
    {
    }

    uint16_t *currentFrame = nullptr;
    bool hasCameraFrame = false;
    TensorPtr<uint16_t> currentCameraFrame;
};