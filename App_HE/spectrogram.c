#include "RTE_Components.h"
#include "dave_driver.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */

#include "arm_math_types.h"
#include "disp.h"
#include "spectrogram.h"
#include <stdio.h>


#include "aipl_dave2d.h"

static osMutexId_t g_mutex;

static float32_t left_bins[NB_BIN];
static float32_t right_bins[NB_BIN];

// Gradient
static uint32_t grad_1x256[256];

void new_data(float *new_bins, int left_channel) {
  if (osMutexAcquire(g_mutex, osWaitForever) == osOK) {
    if (left_channel) {
      memcpy(left_bins, new_bins, sizeof(left_bins));
    } else {
      memcpy(right_bins, new_bins, sizeof(right_bins));
    }
    osMutexRelease(g_mutex);
  }
}

#define NB_SPECTROGRAM 2
#define SEP 10
#define VER_PADDING 10
#define HOR_PADDING 10
#define BIN_PADDING 0

void drawBins(d2_device *handle, float *bin, int channel_nb) {

  const int width = MY_DISP_VER_RES - 2 * VER_PADDING;
  const float height =
      1.0f * (MY_DISP_HOR_RES - 2 * HOR_PADDING - (NB_SPECTROGRAM - 1) * SEP) /
      NB_SPECTROGRAM;
  const int pos = HOR_PADDING + channel_nb * (height + SEP);

  if (osMutexAcquire(g_mutex, osWaitForever) == osOK) {

    const int BIN_WIDTH = width / (NB_BIN);

    d2_selectrendermode(handle, d2_rm_solid);
    d2_setfillmode(handle, d2_fm_texture);

    for (int i = 0; i < NB_BIN; i++) {
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

    osMutexRelease(g_mutex);
  }
}

int init_spectrogram() {

  g_mutex = osMutexNew(NULL);

  for (int i = 0; i < 256; i++) {
    if (i < 128) {
      grad_1x256[i] = 0xFF00FF00;
    } else if (i < 192) {
      grad_1x256[i] = 0xFFFFFF00;
    } else {
      grad_1x256[i] = 0xFFFF0000;
    }
  }
  return 0;
}

void free_spectrogram() {}

void display_data() {

  osThreadFlagsWait(LCD_REFRESH_FLAG, osFlagsWaitAny, osWaitForever);

  d2_device *handle = aipl_dave2d_handle();
  /* Prepare frame buffer */

  d2_framebuffer(handle, disp_inactive_buffer(), RTE_PANEL_HACTIVE_TIME,
                 RTE_PANEL_HACTIVE_TIME, RTE_PANEL_VACTIVE_LINE,
                 d2_mode_rgb565);

  d2_startframe(handle);

  // d2_setblendmode(handle, d2_bm_alpha, d2_bm_one_minus_alpha);
  // d2_setalphablendmode(handle, d2_bm_one, d2_bm_one_minus_alpha);

  d2_clear(handle, 0xFFFFFFFF);

  d2_settexture(handle, grad_1x256, /*pitch*/ 1, /*width*/ 1, /*height*/ 256,
                d2_mode_argb8888);

  d2_settexturemode(handle, d2_tm_filter);

  drawBins(handle, left_bins, 1);
  drawBins(handle, right_bins, 0);

  d2_endframe(handle);
  d2_startframe(handle);
  d2_endframe(handle);

  disp_next_frame();

  
}
