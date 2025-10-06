#pragma once

#include <cstdint>
#include CMSIS_device_header

#include <new>

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include "custom.hpp"

#include "cmsis_os2.h"

using namespace arm_cmsis_stream;

#define VSTREAM_STEREO_SINK_BLOCK_COUNT (3)
#define VSTREAM_AUDIO_SINK_BLOCK_EVT (0x2)
#define VSTREAM_AUDIO_SINK_UNDERFLOW_EVT (0x4)

extern "C"
{
    extern osThreadId_t tid_stream;
#include "Driver_I2C.h"

#include "Driver_SAI.h"
#include "WM8904_driver.h"

    extern ARM_DRIVER_WM8904 WM8904;
    extern ARM_DRIVER_SAI Driver_SAI2;
#define AudioSinkDriver (&Driver_SAI2)
}

extern ARM_DRIVER_I2C ARM_Driver_I2C_(RTE_WM8904_CODEC_I2C_INSTANCE);
#define WM8904_I2C (&ARM_Driver_I2C_(RTE_WM8904_CODEC_I2C_INSTANCE))

template <typename OUT, int outputSize>
class VStreamAudioSink;

template <int outputSamples>
class VStreamAudioSink<sq15, outputSamples>
    : public GenericSink<sq15, outputSamples>
{
  public:
    static void ARM_SAI_SignalEvent(uint32_t event)
    {
        // Handle SAI events
        if (event & ARM_SAI_EVENT_SEND_COMPLETE)
        {

            if (tid_stream != NULL)
                osThreadFlagsSet(tid_stream, VSTREAM_AUDIO_SINK_BLOCK_EVT);
        }
        if (event & ARM_SAI_EVENT_TX_UNDERFLOW)
        {
            ERROR_PRINT("SAI TX underflow"); // Handle SAI Tx underflow
            osThreadFlagsSet(tid_stream, VSTREAM_AUDIO_SINK_UNDERFLOW_EVT);
        }
    }

    VStreamAudioSink(FIFOBase<sq15> &dst,int volume = 5)
        : GenericSink<sq15, outputSamples>(dst)
    {

        stereoBuffer = new (std::align_val_t(64)) sq15[VSTREAM_STEREO_SINK_BLOCK_COUNT * outputSamples];

        if (AudioSinkDriver->Initialize(ARM_SAI_SignalEvent) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to initialize SAI driver\n");
        }

        if (AudioSinkDriver->PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to power on SAI driver\n");
        }

        if (AudioSinkDriver->Control(ARM_SAI_CONFIGURE_TX |
                                         ARM_SAI_MODE_MASTER |
                                         ARM_SAI_ASYNCHRONOUS |
                                         ARM_SAI_PROTOCOL_I2S |
                                         ARM_SAI_DATA_SIZE(16),
                                     32, 16000) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to configure SAI driver\n");
        }

        if (WM8904.Initialize() != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to initialize WM8904 codec\n");
        }


        if (WM8904.PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to power on WM8904 codec\n");
        }

        if (WM8904.SetVolume(volume) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to set volume on WM8904 codec\n");
        }

    };

    ~VStreamAudioSink()
    {
        if (AudioSinkDriver->Control(ARM_SAI_CONTROL_TX, 0, 0) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to abort SAI send\n");
        }
        if (AudioSinkDriver->PowerControl(ARM_POWER_OFF) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to power off SAI driver\n");
        }
        if (AudioSinkDriver->Uninitialize() != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to uninitialize SAI driver\n");
        }
        if (WM8904.Uninitialize() != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to uninitialize WM8904 codec\n");
        }
        delete[] (stereoBuffer);
    };

    int prepareForRunning() final
    {
        if (this->willUnderflow())
        {
            return (CG_SKIP_EXECUTION_ID_CODE); // Skip execution
        }

        return (0);
    };

    int run() final
    {
        if (!started)
        {
            started = true;
            int32_t status = status = AudioSinkDriver->Control(ARM_SAI_CONTROL_TX, 1, 0);
            if (status)
            {
                return CG_INIT_FAILURE;
            }

            if (AudioSinkDriver->Send(getTransmitBlock(), outputSamples) != ARM_DRIVER_OK)
            {
                ERROR_PRINT("Failed to send audio data 1\n");
                return (CG_BUFFER_UNDERFLOW);
            }
        }
        uint32_t flags = osThreadFlagsWait(VSTREAM_AUDIO_SINK_BLOCK_EVT | VSTREAM_AUDIO_SINK_UNDERFLOW_EVT, osFlagsWaitAny, osWaitForever);
        if (flags & VSTREAM_AUDIO_SINK_UNDERFLOW_EVT)
        {
            return (CG_BUFFER_UNDERFLOW);
        }

        transmitIndex = (transmitIndex + 1) % VSTREAM_STEREO_SINK_BLOCK_COUNT;
        if (AudioSinkDriver->Send(getTransmitBlock(), outputSamples) != ARM_DRIVER_OK)
        {
            ERROR_PRINT("Failed to send audio data 2\n");
        }

        sq15 *buf = getCurrentBlock();
        sq15 *input = this->getReadBuffer();
        if (buf)
            memcpy(buf, input, outputSamples * sizeof(sq15));
        else
            return (CG_BUFFER_UNDERFLOW);

        return (CG_SUCCESS);
    };

  protected:
    sq15 *getTransmitBlock()
    {
        return &stereoBuffer[transmitIndex * outputSamples];
    }

    sq15 *getCurrentBlock()
    {
        uint32_t oldIndex = currentIndex;
        currentIndex = (currentIndex + 1) % VSTREAM_STEREO_SINK_BLOCK_COUNT;

        return &stereoBuffer[oldIndex * outputSamples];
    }

    bool started{false};
    sq15 *stereoBuffer;
    uint32_t currentIndex{2};
    uint32_t transmitIndex{0};
};