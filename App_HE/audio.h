#ifndef audio_h
#define audio_h

#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" 

extern void app_audio_thread(void *arg);
extern osThreadId_t tid_audio_capture;

#endif