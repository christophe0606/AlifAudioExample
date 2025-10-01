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
        if (vStream_VideoOut->SetBuf(displayFrame, DISPLAY_IMAGE_SIZE, DISPLAY_IMAGE_SIZE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to set buffer for video output\n");
        }

        
       
        
       
    }

    virtual ~VStreamVideoSink()
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


    virtual void drawFrame() = 0;

    // The node was asked to render a new frame
    void
    renderNewFrame()
    {

        if (inRender.load())
        {
            DEBUG_PRINT("Already in render\n");
            return;
        }

        inRender.store(true);

       
        vStreamStatus_t status;
        do
        {
            status = vStream_VideoOut->GetStatus();
        } while (status.active == 1U);
        
        this->drawFrame();
        SCB_CleanInvalidateDCache_by_Addr(renderingFrame, DISPLAY_IMAGE_SIZE);
        switchBuffers();

       
        inRender.store(false);

        if (vStream_VideoOut->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to start LCD output\n");
        }
    }

    cg_status init() override
    {
        SCB_CleanInvalidateDCache_by_Addr(renderingFrame, DISPLAY_IMAGE_SIZE);

        switchBuffers();
        if (vStream_VideoOut->Start(VSTREAM_MODE_SINGLE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to start LCD output\n");
        }
        return CG_SUCCESS;
    }

    void *displayBuffer()
    {
        return (displayFrame.load());
    }

    void switchBuffers()
    {
        frameIndex = (frameIndex == kBuffer0) ? kBuffer1 : kBuffer0;
        switch (frameIndex)
        {
        case kBuffer0:
            renderingFrame = (uint16_t *)LCD_Frame;
            displayFrame.store((uint16_t *)(LCD_Frame + DISPLAY_IMAGE_SIZE));
            break;
        case kBuffer1:
            renderingFrame = (uint16_t *)(LCD_Frame + DISPLAY_IMAGE_SIZE);
            displayFrame.store((uint16_t *)LCD_Frame);
            break;
        }

        setDisplayBuffer();
        vStream_VideoOut->GetBlock();
        vStream_VideoOut->ReleaseBlock();
    }

  protected:
    std::atomic<bool> inRender{false};

    void setDisplayBuffer()
    {
        if (vStream_VideoOut->SetBuf(displayFrame, DISPLAY_IMAGE_SIZE, DISPLAY_IMAGE_SIZE) != VSTREAM_OK)
        {
            ERROR_PRINT("Failed to set buffer for video output\n");
        }
    }

    enum kRenderingBuffer
    {
        kBuffer0 = 0,
        kBuffer1 = 1
    };

    kRenderingBuffer frameIndex{kBuffer0};
    uint16_t *renderingFrame{(uint16_t *)LCD_Frame};
    std::atomic<uint16_t *> displayFrame{(uint16_t *)(LCD_Frame + DISPLAY_IMAGE_SIZE)};
};