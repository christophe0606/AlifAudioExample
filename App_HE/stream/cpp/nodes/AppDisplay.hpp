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
#include "Driver_CDC200.h"
#include "cmsis_os2.h"
#include "cmsis_vstream.h"
#include "config.h"
}

#include "nodes/VStreamVideoSink.hpp"

using namespace arm_cmsis_stream;

class AppDisplay : public VStreamVideoSink
{
  public:
    AppDisplay()
        : VStreamVideoSink()
    {
    }

    cg_status init() final override
    {
        drawFrame();

        return VStreamVideoSink::init();
    }

    virtual ~AppDisplay() {};

    static constexpr int PADDING_LEFT = 10;
    static constexpr int PADDING_RIGHT = 10;
    static constexpr int PADDING_TOP = 10;
    static constexpr int PADDING_BOTTOM = 10;
    static constexpr int HORIZONTAL_SEPARATION = 10;

    void fillRectangle(int x, int y, int width, int height, uint16_t color)
    {
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
                renderingFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
    }

    void strokeRectangle(int x, int y, int width, int height, uint16_t color)
    {
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
                renderingFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
        if (drawBottom)
        {
            for (int j = 0; j < width; j++)
            {
                int px = x + j;
                int py = y + height - 1;
                renderingFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
        if (drawLeft)
        {
            for (int i = 0; i < height; i++)
            {
                int px = x;
                int py = y + i;
                renderingFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
        if (drawRight)
        {
            for (int i = 0; i < height; i++)
            {
                int px = x + width - 1;
                int py = y + i;
                renderingFrame[py * DISPLAY_FRAME_WIDTH + px] = color;
            }
        }
    }

    void drawFrame() final override
    {

        memset(renderingFrame, 0xFF, DISPLAY_IMAGE_SIZE);
        const int boxWidth = (DISPLAY_FRAME_WIDTH - PADDING_LEFT - PADDING_RIGHT - HORIZONTAL_SEPARATION) / 2;
        const int boxHeight = DISPLAY_FRAME_HEIGHT - PADDING_TOP - PADDING_BOTTOM;

        strokeRectangle(PADDING_LEFT, PADDING_TOP, boxWidth, boxHeight, 0x00);
        strokeRectangle(PADDING_LEFT + boxWidth + HORIZONTAL_SEPARATION, PADDING_TOP, boxWidth, boxHeight, 0x00);
        // fillRectangle(0,0,CAMERA_FRAME_WIDTH,CAMERA_FRAME_HEIGHT,0x03F << 5);
        if (hasCameraFrame)
        {
#if 1
            currentCameraFrame.lock_shared([this](CG_MUTEX_ERROR_TYPE error, const Tensor<uint16_t> &tensor)
                                           {
            if (!CG_MUTEX_HAS_ERROR(error))
            {
               
            
             if (std::holds_alternative<UniquePtr<uint16_t>>(tensor.data))
                {
                    const int wpad = (DISPLAY_FRAME_WIDTH - tensor.dims[1]) / 2;
                    const int hpad = (DISPLAY_FRAME_HEIGHT - tensor.dims[0]) / 2;
                    const UniquePtr<uint16_t> &buf = std::get<UniquePtr<uint16_t>>(tensor.data);
                    for (int h = 0; h < tensor.dims[0]; h++)
                    {
                        for (int w = 0; w < tensor.dims[1]; w++)
                        {
                            renderingFrame[wpad+w + (h+hpad) * DISPLAY_FRAME_WIDTH] = buf.get()[(w) + (h) * tensor.dims[1]];
                        }
                    }
            } } });

#endif
        }
    }

    void processEvent(int dstPort, Event &&evt) final override
    {

        // New camera frame or spectrogram
        if (evt.event_id == kValue)
        {
            // New camera frame
            if (dstPort == 2)
            {
                if (evt.wellFormed<TensorPtr<uint16_t>>())
                {
                    evt.apply<TensorPtr<uint16_t>>(&AppDisplay::processCameraFrame, *this);
                    return;
                }
            }

            // New left spectrogram
            if (dstPort == 0)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&AppDisplay::processLeftSpectrogram, *this);
                    return;
                }
            }

            // New right spectrogram
            if (dstPort == 1)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&AppDisplay::processRightSpectrogram, *this);
                    return;
                }
            }
        }
    }

  protected:
    void processCameraFrame(TensorPtr<uint16_t> &&frame)
    {
        hasCameraFrame = true;
        currentCameraFrame = std::move(frame);

        // Render new frame each time a camera frame is received
        this->renderNewFrame();

        // Release the frame so that camera get new one
        currentCameraFrame = std::move(TensorPtr<uint16_t>()); // Release the frame so that camera get new one
        hasCameraFrame = false;
    }

    void processLeftSpectrogram(TensorPtr<float> &&frame)
    {
    }

    void processRightSpectrogram(TensorPtr<float> &&frame)
    {
    }

    bool hasCameraFrame = false;
    TensorPtr<uint16_t> currentCameraFrame;
};