#include "RTE_Components.h"
#include CMSIS_device_header

#include "arm_math_types.h"
#include "image.h"
#include <stdio.h>
#include "spectrogram.h"
#include "disp.h"

#define ROTATE_180

#define NB_BIN 32
static float32_t bin[NB_BIN];
static aipl_image_t image;

void updateBins() {
  for (int i = 0; i < NB_BIN; i++) {
    bin[i] = 0.0f;
  }

  bin[NB_BIN >> 1] = 0.6f;
  bin[(NB_BIN >> 1) + 1] = 0.7f;
  bin[(NB_BIN >> 1) - 1] = 0.1f;
}

void fillRectangle(aipl_image_t *image, int32_t x, int32_t y, int32_t w,
                   int32_t h, uint32_t color) {

  uint32_t strideA = 1;
  uint32_t strideB = image->pitch;
  uint32_t width = image->width;
  uint32_t height = image->height;
  if ((x + w) >= image->width)
    w = image->width - x - 1;
  if ((y + h) >= image->height)
    h = image->height - y - 1;

  if (x < 0) {
    w = w + x;
    x = 0;
  }

  if (y < 0) {
    h = h + y;
    y = 0;
  }

  if (w <= 0)
    return;

  if (h <= 0)
    return;

#if defined(ROTATE_180)
  y = height - y - h;
  x = width - x - w;
#endif

  if (image->format == AIPL_COLOR_ARGB8888) {
    uint32_t *p = (uint32_t *)image->data;
    for (uint32_t j = 0; j < h; j++) {
      for (uint32_t i = 0; i < w; i++) {
        p[(x + i) * strideA + (y + j) * strideB] = color; // ARGB
      }
    }
  } else {
    printf("Unsupported image format %d\n", image->format);
  }
}

void drawBins(aipl_image_t *image) {
  if (image->format == AIPL_COLOR_ARGB8888) {
    for (uint32_t i = 0; i < NB_BIN; i++) {
      uint32_t binHeight = (uint32_t)(bin[i] * (float32_t)image->height);
      if (binHeight > image->height) {
        binHeight = image->height;
      }
      fillRectangle(image, i * (image->width / NB_BIN),
                    image->height - binHeight, (image->width / NB_BIN) - 1,
                    binHeight, 0xFF00FF00); // Green color
    }
  } else {
    printf("Unsupported image format %d\n", image->format);
  }
}

void fillImage(aipl_image_t *image, uint32_t color) {
  if (image->format == AIPL_COLOR_ARGB8888) {
    uint32_t *p = (uint32_t *)image->data;
    for (uint32_t y = 0; y < image->height; y++) {
      for (uint32_t x = 0; x < image->width; x++) {
        p[x + y * (image->pitch)] = color; // ARGB
      }
    }
  } else {
    printf("Unsupported image format %d\n", image->format);
  }
}

int init_spectrogram()
{

  aipl_error_t aipl_ret = AIPL_ERR_OK;

  aipl_ret = aipl_image_create(&image, MY_DISP_HOR_RES, MY_DISP_HOR_RES,
                               MY_DISP_VER_RES, AIPL_COLOR_ARGB8888);

  if (aipl_ret != AIPL_ERR_OK) {
    printf("Error: image aipl_image_create() failed %d\n", aipl_ret);
    return -1;
}

  return 0;
}

void free_spectrogram()
{
   aipl_image_destroy(&image);
}

void update_data()
{
   updateBins();
}

void display_data()
{
    fillImage(&image, 0xFFFFFFFF);
    drawBins(&image);

    aipl_dave2d_prepare();
    aipl_image_draw(0, 0, &image);
    aipl_dave2d_render();
}
