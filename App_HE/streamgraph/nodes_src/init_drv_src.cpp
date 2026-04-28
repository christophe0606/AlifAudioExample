extern "C"
{
#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
}
#include "camera_config.h"
#include "stream_platform_config.hpp"
#include "init_drv_src.hpp"

#include "cmsis_vstream.h"
#include "datatypes.hpp"
#include "app_config.hpp"

/* Camera frame buffer (RAW8 or RGB565) */
uint8_t CAM_Frame[CAMERA_BUFFER_SIZE] CAMERA_FRAME_BUF_ATTRIBUTE;

/* Display frame buffer */
uint8_t LCD_Frame[DISPLAY_BUFFER_SIZE] DISPLAY_FRAME_BUF_ATTRIBUTE;


#define VSTREAM_STEREO_SOURCE_BLOCK_COUNT (2)

extern vStreamDriver_t Driver_vStreamAudioIn;
#define vStream_AudioIn (&Driver_vStreamAudioIn)

static sq15 stereoBuffer[VSTREAM_STEREO_SOURCE_BLOCK_COUNT * AUDIO_BLOCK];

static osEventFlagsId_t cg_audioSrcEvent;





static void AudioSourceDrv_Event_Callback(uint32_t event)
    {
        (void)event;
        if (event & VSTREAM_EVENT_OVERFLOW)
        {
            osEventFlagsSet(cg_audioSrcEvent, AUDIO_SOURCE_OVERFLOW_EVENT);
        }

        osEventFlagsSet(cg_audioSrcEvent, AUDIO_SOURCE_FRAME_EVENT);
    }

vStreamDriver_t *init_audio_source(osEventFlagsId_t &audioSrcEvent)
{
    cg_audioSrcEvent = osEventFlagsNew(NULL);
    audioSrcEvent = cg_audioSrcEvent;


    if (vStream_AudioIn->Initialize(AudioSourceDrv_Event_Callback) != VSTREAM_OK)
    {
            CMSISSTREAM_LOG_ERR("vStream_AudioIn Initialize error\n");
            return nullptr;
    }

    if (vStream_AudioIn->SetBuf(stereoBuffer,
                                VSTREAM_STEREO_SOURCE_BLOCK_COUNT * sizeof(sq15) * AUDIO_BLOCK,
                            sizeof(sq15) * AUDIO_BLOCK) != VSTREAM_OK)
    {
            CMSISSTREAM_LOG_ERR("vStream_AudioIn SetBuf error\n");
            vStream_AudioIn->Uninitialize();
            return nullptr;
    }

    return vStream_AudioIn;
}