#pragma once

#include "cg_enums.h"
#include "custom.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "config.h"
#include "dave_base.h"
#include "dave_driver.h"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <variant>

extern "C"
{
#include "aipl_dave2d.h"
#include "cmsis_os2.h"
#include "config.h"
#include "disp.h"

}

#define NB_SPECTROGRAM 2
#define SEP 10
#define VER_PADDING 10
#define HOR_PADDING 10
#define BIN_PADDING 0

using namespace arm_cmsis_stream;

extern osMutexId_t bin_mutex;

static_assert(std::atomic<uint32_t>::is_always_lock_free, "expected lock-free 32-bit atomics");

static __ALIGNED(16) uint32_t grad_1x256[256];

class DisplayGPU : public StreamNode
{
  public:
    // The constructor for the sink is only using
    // the input FIFO (coming from the generated scheduler).
    // This FIFO is passed to the GenericSink contructor.
    // Implementation of this Sink constructor is doing nothing
    DisplayGPU()
        : StreamNode()
    {
        memset(left_bins, 0, sizeof(left_bins));
        memset(right_bins, 0, sizeof(right_bins));
        for (int i = 0; i < 256; i++)
        {
            if (i < 128)
            {
                grad_1x256[i] = 0xFF00FF00;
            }
            else if (i < 192)
            {
                grad_1x256[i] = 0xFFFFFF00;
            }
            else
            {
                grad_1x256[i] = 0xFFFF0000;
            }
        }
        d2_device *handle = aipl_dave2d_handle();
        d2_settexture(handle, grad_1x256, /*pitch*/ 1, /*width*/ 1, /*height*/ 256,
                      d2_mode_argb8888);
    };

    void processEvent(int dstPort, Event &&evt) final
    {
        if (evt.event_id == kValue)
        {
            // DEBUG_PRINT("Value event received on port %d, frame = %d\n", dstPort, frame_count++);
            if (evt.contains<TensorPtr<float>>())
            {
                TensorPtr<float> t = evt.get<TensorPtr<float>>();
                t.lock_shared([this, dstPort](CG_MUTEX_ERROR_TYPE error, const Tensor<float> &tensor)
                              {
                                  if (CG_MUTEX_HAS_ERROR(error))
                                  {
                                      ERROR_PRINT("Error locking tensor mutex\n");
                                      return;
                                  }
                                  if (tensor.nb_dims != 1 || tensor.dims[0] != NB_BINS)
                                  {
                                      return;
                                  }

                                  if (std::holds_alternative<UniquePtr<float>>(tensor.data))
                                  {
                                      const auto &data = std::get<UniquePtr<float>>(tensor.data);
                                      osStatus_t status = osMutexAcquire(bin_mutex, osWaitForever);
                                      if (status == osOK)
                                      {
                                           //DEBUG_PRINT("Display event on port %d, frame = %d\n", dstPort, frame_count++);
                                           if (dstPort == 0)
                                           {
                                               memcpy(left_bins, data.get(), sizeof(left_bins));
                                               has_changed.store(1);
                                           }
                                           else if (dstPort == 1)
                                           {
                                               memcpy(right_bins, data.get(), sizeof(right_bins));
                                               has_changed.store(1);
                                           }
                                           osMutexRelease(bin_mutex);
                                     }
                                  } });
            }
        }
    }

    bool render()
    {
        DEBUG_PRINT("Try render\n");
        if (has_changed.load() == 1)
        {
            DEBUG_PRINT("Render display frame %d\n", frame_count);
            has_changed.store(0);
            d2_device *handle = aipl_dave2d_handle();
            /* Prepare frame buffer */

            d2_framebuffer(handle, disp_inactive_buffer(), RTE_PANEL_HACTIVE_TIME,
                           RTE_PANEL_HACTIVE_TIME, RTE_PANEL_VACTIVE_LINE,
                           d2_mode_rgb565);

            d2_startframe(handle);

            // d2_setblendmode(handle, d2_bm_alpha, d2_bm_one_minus_alpha);
            // d2_setalphablendmode(handle, d2_bm_one, d2_bm_one_minus_alpha);

            d2_clear(handle, 0xFFFFFFFF);

            d2_settexturemode(handle, d2_tm_filter);

            drawBins(handle, 0);
            drawBins(handle, 1);

            d2_endframe(handle);
            d2_startframe(handle);
            d2_endframe(handle);

            return true;
        }
        return false;
    }

  protected:
    void drawBins(d2_device *handle, int channel_nb)
    {

        const int width = MY_DISP_VER_RES - 2 * VER_PADDING;
        const float height =
            1.0f * (MY_DISP_HOR_RES - 2 * HOR_PADDING - (NB_SPECTROGRAM - 1) * SEP) /
            NB_SPECTROGRAM;
        const int pos = HOR_PADDING + channel_nb * (height + SEP);
        osStatus_t status;
        status = osMutexAcquire(bin_mutex, osWaitForever);

        if (status == osOK)
        {

            float32_t *bin = channel_nb == 0 ? left_bins : right_bins;
            const float BIN_WIDTH = 1.0f * width / (NB_BINS);

            d2_selectrendermode(handle, d2_rm_solid);
            // d2_setfillmode(handle, d2_fm_color);
            // d2_setcolor(handle, 0,0x00FF0000);

            d2_setfillmode(handle, d2_fm_texture);

            d2_settexturemapping(
                handle, D2_FIX4(pos),
                D2_FIX4(VER_PADDING), // screen position for (u0,v0)
                0 << 16, 0 << 16,     // u0, v0 (start)
                0, 0, (d2_s32)((256 << 16) / height), 0);

            for (int i = 0; i < NB_BINS; i++)
            {
                int h = (int)(bin[i] * height);
                if (h >= height)
                    h = height - 1;
                if (h == 0)
                    continue;
                h += pos;

                d2_renderquad(
                    handle, D2_FIX4(pos), D2_FIX4((int)(VER_PADDING + (i)*BIN_WIDTH)),
                    D2_FIX4(h), D2_FIX4((int)(VER_PADDING + (i)*BIN_WIDTH)), D2_FIX4(h),
                    D2_FIX4((int)(VER_PADDING + (i + 1) * BIN_WIDTH - BIN_PADDING)),
                    D2_FIX4(pos),
                    D2_FIX4((int)(VER_PADDING + (i + 1) * BIN_WIDTH - BIN_PADDING)), 0);
            }

            osMutexRelease(bin_mutex);
        }
        else
        {
            ERROR_PRINT("Error acquiring bin mutex %d\n", status);
        }
        d2_selectrendermode(handle, d2_rm_outline);
        d2_setfillmode(handle, d2_fm_color);
        d2_setcolor(handle, 0, 0x00000000);
        d2_outlinewidth(handle, D2_FIX4(2));

        d2_renderbox(handle, D2_FIX4(pos), D2_FIX4(VER_PADDING),
                     D2_FIX4((int)height), D2_FIX4(width));
    }

    float32_t left_bins[NB_BINS];
    float32_t right_bins[NB_BINS];
    std::atomic<uint32_t> has_changed{1};
    uint32_t frame_count = 0;
};