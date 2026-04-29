#pragma once 

extern "C"
{
#include "RTE_Components.h"
#include CMSIS_device_header
#include "cmsis_vstream.h"
#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
//#include "Driver_Touch_Screen.h"

}

extern vStreamDriver_t* init_audio_source(osEventFlagsId_t &audioSrcEvent);
extern vStreamDriver_t *init_video_source(const osEventFlagsId_t &videoSrcEvent);
//extern ARM_DRIVER_TOUCH_SCREEN *init_touch_screen();
