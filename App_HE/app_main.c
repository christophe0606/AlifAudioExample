#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
#include "main.h"
#include <stdio.h>


#include "disp.h"
#include "spectrogram.h"


void app_audio_thread(void *arg) {
  printf("Audio thread started\n");
  for (;;) {
    osDelay(1000);
  }
}

void app_main_thread(void *arg) {
  printf("Main thread started\n");

  for (;;) {
    update_data();
    display_data();
    // osDelay(40);
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
  osThreadNew(app_main_thread, NULL, &mainAttr);
  osThreadNew(app_audio_thread, NULL, &audioAttr);

  configure_display_and_2d();
  init_spectrogram();

  osKernelStart();

  return 0;
}
