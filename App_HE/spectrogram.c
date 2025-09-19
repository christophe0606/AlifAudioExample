#include "RTE_Components.h"
#include "dave_driver.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */

#include "arm_math_types.h"
#include "disp.h"
#include "image.h"
#include "spectrogram.h"
#include <stdio.h>

#include "aipl_dave2d.h"

#define ROTATE_180

static osMutexId_t g_mutex;

static float32_t bin[NB_BIN];

// Gradient
uint32_t grad_1x256[256];

void new_data(float *new_bins) {
  if (osMutexAcquire(g_mutex, osWaitForever) == osOK) {
    memcpy(bin, new_bins, sizeof(bin));
    osMutexRelease(g_mutex);
  }
}

void updateBins() {
  for (int i = 0; i < NB_BIN; i++) {
    bin[i] = 0.0f;
  }

  bin[NB_BIN >> 1] = 0.6f;
  bin[(NB_BIN >> 1) + 1] = 0.9f;
  bin[(NB_BIN >> 1) - 1] = 0.1f;
}

void drawBins(d2_device *handle) {

  //if (osMutexAcquire(g_mutex, osWaitForever) == osOK) {

    const int BIN_WIDTH = MY_DISP_VER_RES / (NB_BIN + 1);
    for (int i = 0; i < NB_BIN; i++) {
      int h = (int)(bin[i] * MY_DISP_HOR_RES);
      if (h > MY_DISP_HOR_RES)
        h = MY_DISP_HOR_RES;
      if (h == 0)
        continue;

      // d2_setfillmode(handle, d2_fm_color);
      // d2_setcolor(handle, 0,0x00FF0000);

      d2_settexturemapping(
          handle, D2_FIX4(0),
          D2_FIX4((i)*BIN_WIDTH), // screen position for (u0,v0)
          0 << 16, 0 << 16,       // u0, v0 (start)
          0, 0, (256 << 16) / MY_DISP_HOR_RES, 0);

      d2_renderquad(handle, D2_FIX4(0), D2_FIX4((i)*BIN_WIDTH), D2_FIX4(h),
                    D2_FIX4((i)*BIN_WIDTH), D2_FIX4(h),
                    D2_FIX4((i + 1) * BIN_WIDTH-2), D2_FIX4(0),
                    D2_FIX4((i + 1) * BIN_WIDTH-2), 0);
    }
    //osMutexRelease(g_mutex);
  //}
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

void update_data() { 
  
}

void display_data() {
  // fillImage(&image, 0xFFFFFFFF);

  aipl_dave2d_prepare();
  // aipl_image_draw(0, 0, &image);

  d2_device *handle = aipl_dave2d_handle();

  // d2_setblendmode(handle, d2_bm_alpha, d2_bm_one_minus_alpha);
  // d2_setalphablendmode(handle, d2_bm_one, d2_bm_one_minus_alpha);

  d2_clear(handle, 0xFFFFFFFF);
  // d2_setfillmode(handle, d2_fm_color);
  d2_selectrendermode(handle, d2_rm_solid);
  // d2_setcolor(handle, 0, 0xFFFF0000);

  d2_settexture(handle, grad_1x256, /*pitch*/ 1, /*width*/ 1, /*height*/ 256,
                d2_mode_argb8888);

  // 2) Texture mode: bilinear filter (default includes filter); add wrap flags
  // if you ever need repeating.
  d2_settexturemode(handle, d2_tm_filter);

  // 3) Use textures as the fill source
  d2_setfillmode(handle, d2_fm_texture);

  drawBins(handle);

  aipl_dave2d_render();
}
