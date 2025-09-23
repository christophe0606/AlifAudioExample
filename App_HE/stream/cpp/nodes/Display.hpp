#pragma once

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "arm_math_types.h"
#include "config.h"
#include <cstring>
#include <variant>

#include "aipl_dave2d.h"
#include "cmsis_os2.h"
#include "disp.h"

#define NB_SPECTROGRAM 2
#define SEP 10
#define VER_PADDING 10
#define HOR_PADDING 10
#define BIN_PADDING 0

using namespace arm_cmsis_stream;

class Display : public StreamNode
{
  public:
    // The constructor for the sink is only using
    // the input FIFO (coming from the generated scheduler).
    // This FIFO is passed to the GenericSink contructor.
    // Implementation of this Sink constructor is doing nothing
    Display()
        : StreamNode()
    {
        has_changed = false;
        memset(left_bins, 0, sizeof(left_bins));
        memset(right_bins, 0, sizeof(right_bins));
        bin_mutex = osMutexNew(NULL);
    };

    void drawBins(d2_device *handle, int channel_nb)
    {

        const int width = MY_DISP_VER_RES - 2 * VER_PADDING;
        const float height =
            1.0f * (MY_DISP_HOR_RES - 2 * HOR_PADDING - (NB_SPECTROGRAM - 1) * SEP) /
            NB_SPECTROGRAM;
        const int pos = HOR_PADDING + channel_nb * (height + SEP);

        if (osMutexAcquire(bin_mutex, osWaitForever) == osOK)
        {

            float32_t *bin = channel_nb == 0 ? left_bins : right_bins;
            const int BIN_WIDTH = width / (NB_BINS);

            d2_selectrendermode(handle, d2_rm_solid);
            d2_setfillmode(handle, d2_fm_texture);

            for (int i = 0; i < NB_BINS; i++)
            {
                int h = (int)(bin[i] * height);
                if (h > height)
                    h = height;
                if (h == 0)
                    continue;
                h += pos;

                // d2_setfillmode(handle, d2_fm_color);
                // d2_setcolor(handle, 0,0x00FF0000);

                d2_settexturemapping(
                    handle, D2_FIX4(pos),
                    D2_FIX4(VER_PADDING + (i)*BIN_WIDTH), // screen position for (u0,v0)
                    0 << 16, 0 << 16,                     // u0, v0 (start)
                    0, 0, (256 << 16) / height, 0);

                d2_renderquad(
                    handle, D2_FIX4(pos), D2_FIX4(VER_PADDING + (i)*BIN_WIDTH),
                    D2_FIX4(h), D2_FIX4(VER_PADDING + (i)*BIN_WIDTH), D2_FIX4(h),
                    D2_FIX4(VER_PADDING + (i + 1) * BIN_WIDTH - BIN_PADDING),
                    D2_FIX4(pos),
                    D2_FIX4(VER_PADDING + (i + 1) * BIN_WIDTH - BIN_PADDING), 0);
            }

            d2_selectrendermode(handle, d2_rm_outline);
            d2_setfillmode(handle, d2_fm_color);
            d2_setcolor(handle, 0, 0x00000000);
            d2_outlinewidth(handle, D2_FIX4(2));

            d2_renderbox(handle, D2_FIX4(pos), D2_FIX4(VER_PADDING),
                         D2_FIX4((int)height), D2_FIX4(width));

            osMutexRelease(bin_mutex);
        }
    }

    void processEvent(int dstPort, Event &&evt) final
    {
        if (evt.event_id == kValue)
        {
            if (evt.contains<TensorPtr<const float>>())
            {
                auto t = evt.get<TensorPtr<const float>>();
                t.lock_shared([this, dstPort](CG_MUTEX_ERROR_TYPE error, const Tensor<const float> &tensor)
                              {
                                  if (CG_MUTEX_HAS_ERROR(error))
                                  {
                                      printf("Error locking tensor mutex\n");
                                      return;
                                  }
                                  if (tensor.nb_dims != 1 || tensor.dims[0] != NB_BINS)
                                  {
                                      return;
                                  }
                                  if (std::holds_alternative<UniquePtr<const float>>(tensor.data))
                                  {
                                      const auto &data = std::get<UniquePtr<const float>>(tensor.data);
                                      if (osMutexAcquire(bin_mutex, osWaitForever) == osOK)
                                      {
                                           if (dstPort == 0)
                                           {
                                               memcpy(left_bins, data.get(), sizeof(left_bins));
                                               has_changed = true;
                                           }
                                           else if (dstPort == 1)
                                           {
                                               memcpy(right_bins, data.get(), sizeof(right_bins));
                                               has_changed = true;
                                           }
                                           osMutexRelease(bin_mutex);
                                     }
                                  } });
            }
        }
    }

  protected:
    osMutexId_t bin_mutex;
    float32_t left_bins[NB_BINS];
    float32_t right_bins[NB_BINS];
    bool has_changed;
};