#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h"          /* CMSIS-RTOS2 API */

#include <stdio.h>

#include "main.h"

void app_audio_thread(void *arg)
{
    printf("Audio thread started\n");
    for (;;) {
        osDelay(1000);
    }
}

void app_main_thread(void *arg)
{
    printf("Main thread started\n");
    for (;;) {
        osDelay(1000);
    }
}

int app_main(void) 
{

    const osThreadAttr_t mainAttr = {
        .stack_size = 4096,
    };

    const osThreadAttr_t audioAttr = {
        .stack_size = 4096,
        .priority = osPriorityRealtime
    };
    osKernelInitialize();
    osThreadNew(app_main_thread, NULL, &mainAttr);
    osThreadNew(app_audio_thread, NULL, &audioAttr);
    osKernelStart();

    return 0;
}
