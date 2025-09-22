#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
#include "main.h"
#include <stdio.h>


#include "disp.h"
#include "spectrogram.h"

#include "audio.h"

osThreadId_t tid_main = NULL;

void app_main_thread(void *arg) {
  configure_display_and_2d();
  

  printf("Main thread started\n");


  for (;;) {
    display_data();
  }

endMain:
  free_spectrogram();
  osThreadExit();
}

int app_main(void) {

  const osThreadAttr_t mainAttr = {
      .stack_size = 4096,
  };

  const osThreadAttr_t audioAttr = {.stack_size = 4096,
                                    .priority = osPriorityRealtime};
  osKernelInitialize();
  tid_main = osThreadNew(app_main_thread, NULL, &mainAttr);
  tid_audio_capture = osThreadNew(app_audio_thread, NULL, &audioAttr);

  init_spectrogram();

  osKernelStart();

  return 0;
}
