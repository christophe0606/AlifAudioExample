#include "audio.h"

#include "RTE_Components.h"
#include "dsp/fast_math_functions.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
#include "cmsis_vstream.h"

#include <stdio.h>

#include "spectrogram.h"

#include "dsp/basic_math_functions.h"
#include "dsp/complex_math_functions.h"
#include "dsp/support_functions.h"
#include "dsp/transform_functions.h"
#include "dsp/window_functions.h"

/* Define stereo (audio in) and mono (audio for inference) buffers */
#define FFT_SIZE 512

#define STEREO_BLOCK_COUNT (2)
#define STEREO_BLOCK_SAMPLES (FFT_SIZE * 2)          // left and right channel
#define STEREO_BLOCK_SIZE (STEREO_BLOCK_SAMPLES * 2) // size in bytes

#define MONO_BLOCK_SAMPLES (FFT_SIZE)            // complex numbers
#define MONO_BLOCK_SIZE (MONO_BLOCK_SAMPLES * 2) // size in bytes

static int16_t stereoBuffer[STEREO_BLOCK_SAMPLES * STEREO_BLOCK_COUNT];
static int16_t tmpQ15[MONO_BLOCK_SAMPLES];
static float tmp[MONO_BLOCK_SAMPLES];

static arm_cfft_instance_f32 varInstCfftF32;

static float32_t spectrum[2 * FFT_SIZE];
static float32_t hanning[FFT_SIZE];

static float32_t mag[FFT_SIZE >> 1];
float32_t bins[NB_BIN];

extern vStreamDriver_t Driver_vStreamAudioIn;
#define vStream_AudioIn (&Driver_vStreamAudioIn)

osThreadId_t tid_audio_capture = NULL;

void AudioDrv_Event_Callback(uint32_t event) {
  (void)event;

  osThreadFlagsSet(tid_audio_capture, 0x0001);
}

void compute_spectrogram(int16_t *stereo_buf, int left_channel) {

  for (int i = 0; i < MONO_BLOCK_SAMPLES; i++) {
    if (left_channel)
      tmpQ15[i] = stereo_buf[2 * i];
    else
      tmpQ15[i] = stereo_buf[2 * i + 1];
  }

  arm_q15_to_float((const q15_t *)tmpQ15, tmp, FFT_SIZE);
  arm_mult_f32(tmp, hanning, tmp, FFT_SIZE);

  for (int i = 0; i < FFT_SIZE; i++) {
    spectrum[2 * i] = tmp[i];
    spectrum[2 * i + 1] = 0.0f;
  }

  arm_cfft_f32(&varInstCfftF32, spectrum, 0, 1);

  arm_cmplx_mag_f32(spectrum, mag, FFT_SIZE >> 1);

  arm_scale_f32(spectrum,
                1.0f / (FFT_SIZE  >> 5),
                spectrum, FFT_SIZE >> 1);
/*
  arm_scale_f32(spectrum,
                10.0f / 2.302585092994046f / ((FFT_SIZE * FFT_SIZE) >> 4),
                spectrum, FFT_SIZE >> 1);

  arm_offset_f32(spectrum, 1.0e-6f, spectrum, FFT_SIZE >> 1);

  arm_vlog_f32(spectrum, spectrum, FFT_SIZE >> 1);

  arm_scale_f32(spectrum, 10.0f / 2.302585092994046f, spectrum, FFT_SIZE >> 1);

  arm_offset_f32(spectrum, 60.0f, spectrum, FFT_SIZE >> 1);

  arm_scale_f32(spectrum, 1.0f / 60.0f, spectrum, FFT_SIZE >> 1);
*/

  float di = 1.0 * NB_BIN / ((float)FFT_SIZE / 2);
  // float scale = 1.0f * FFT_SIZE / 2 / NB_BIN;
  float k = 0;
  memset(bins, 0, sizeof(bins));

  for (int i = 0; i < FFT_SIZE / 2; i++) {
    if (k < NB_BIN)
      bins[(int)k] += mag[i];
    k += di;
  }

  for (int i = 0; i < NB_BIN; i++) {
    //   bins[i] *= scale;
    if (bins[i] > 1.0f)
      bins[i] = 1.0f;
    if (bins[i] < 0.0f)
      bins[i] = 0.0f;
  }

  new_data(bins,left_channel);
}

void app_audio_thread(void *arg) {
  int16_t *buf;

  arm_status status = ARM_MATH_SUCCESS;

  status = arm_cfft_init_512_f32(&varInstCfftF32);
  arm_hanning_f32(hanning, FFT_SIZE);

  /* Initialize audio in stream and set the receive buffer */
  vStream_AudioIn->Initialize(AudioDrv_Event_Callback);
  vStream_AudioIn->SetBuf(stereoBuffer, STEREO_BLOCK_COUNT * STEREO_BLOCK_SIZE,
                          STEREO_BLOCK_SIZE);

  /* Start audio receiver */
  vStream_AudioIn->Start(VSTREAM_MODE_CONTINUOUS);

  printf("Audio thread started\n");
  for (;;) {
    /* Wait for flag from audio callback */
    osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);

    /* Process block of currently received audio samples */
    buf = (int16_t *)vStream_AudioIn->GetBlock();
    compute_spectrogram(buf,1);
    compute_spectrogram(buf,0);

    /* Release buffer block to vStream driver */
    vStream_AudioIn->ReleaseBlock();
  }
}