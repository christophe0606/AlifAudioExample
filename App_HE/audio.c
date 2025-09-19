#include "audio.h"

#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_vstream.h"
#include "cmsis_os2.h" /* CMSIS-RTOS2 API */

#include <stdio.h>

#include "spectrogram.h"

#include "dsp/basic_math_functions.h"
#include "dsp/transform_functions.h"
#include "dsp/support_functions.h"
#include "dsp/complex_math_functions.h"

/* Define stereo (audio in) and mono (audio for inference) buffers */
#define STEREO_BLOCK_COUNT   (2)
#define STEREO_BLOCK_SAMPLES (16000)
#define STEREO_BLOCK_SIZE    (STEREO_BLOCK_SAMPLES * 2)
#define MONO_BLOCK_COUNT     (2)
#define MONO_BLOCK_SAMPLES   (8000)
#define MONO_BLOCK_SIZE      (MONO_BLOCK_SAMPLES * 2)

int16_t stereoBuffer[STEREO_BLOCK_SAMPLES * STEREO_BLOCK_COUNT];
int16_t left[MONO_BLOCK_SAMPLES];
int16_t right[MONO_BLOCK_SAMPLES];
arm_cfft_instance_q15 varInstCfftQ15;

#define FFT_SIZE 1024
float32_t spectrum[2*FFT_SIZE];
float32_t mag[FFT_SIZE>>1];
float32_t bins[NB_BIN];


extern vStreamDriver_t          Driver_vStreamAudioIn;
#define vStream_AudioIn       (&Driver_vStreamAudioIn)

osThreadId_t tid_audio_capture = NULL;

void AudioDrv_Event_Callback (uint32_t event) {
  (void)event;

  osThreadFlagsSet(tid_audio_capture, 0x0001);
}


void compute_spectrogram(int16_t *buf)
{
    

   for(int i=0; i<MONO_BLOCK_SAMPLES; i++)
   {
      left[i] = buf[2*i];
      right[i] = buf[2*i + 1];
   }

   // Convert to mono with saturation
   arm_add_q15((q15_t *)left, 
   (q15_t *)right, 
   (q15_t *)left, 
   MONO_BLOCK_SAMPLES);

   // Ugly decimation
   for(int i=0; i<MONO_BLOCK_SAMPLES/8; i++)
   {
      if (8*i < MONO_BLOCK_SAMPLES)
      {
          right[2*i] = left[8*i];
          right[2*i+1] = 0;
      }
      else 
      {
        right[2*i] = 0;
        right[2*i+1] = 0;
      }
   }

   arm_cfft_q15(&varInstCfftQ15, (q15_t *)right, 0, 1);

   arm_q15_to_float((const q15_t *)right, 
   spectrum, 
   2*FFT_SIZE);

   arm_cmplx_mag_f32(spectrum, mag, FFT_SIZE>>1);

   float di = 1.0*NB_BIN / ((float)FFT_SIZE/2);
   float scale = 1.0f*FFT_SIZE/2/NB_BIN;
   float k = 0;
   memset(bins, 0, sizeof(bins));
   
   for(int i=0; i<FFT_SIZE/2; i++)
   {
      if (k < NB_BIN)
         bins[(int)k] += mag[i];
      k += di;
   }

   for(int i=0; i<NB_BIN; i++)
   {
      bins[i] *= scale;
   }

    new_data(bins);

}


void app_audio_thread(void *arg) {
  int16_t *buf;

  arm_status status = ARM_MATH_SUCCESS;

  status=arm_cfft_init_1024_q15(&varInstCfftQ15);

  /* Initialize audio in stream and set the receive buffer */
  vStream_AudioIn->Initialize(AudioDrv_Event_Callback);
  vStream_AudioIn->SetBuf(stereoBuffer, STEREO_BLOCK_COUNT * STEREO_BLOCK_SIZE, STEREO_BLOCK_SIZE);

  /* Start audio receiver */
  vStream_AudioIn->Start(VSTREAM_MODE_CONTINUOUS);

  printf("Audio thread started\n");
  for (;;) {
    /* Wait for flag from audio callback */
      osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);

      /* Process block of currently received audio samples */
      buf = (int16_t *)vStream_AudioIn->GetBlock();
      compute_spectrogram(buf);

      /* Release buffer block to vStream driver */
      vStream_AudioIn->ReleaseBlock();
  }
}