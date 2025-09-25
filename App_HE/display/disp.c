/**
 * @file disp.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "disp.h"
#include <stdbool.h>
#include <stdio.h>

#include "RTE_Components.h"
#include CMSIS_device_header
#include <RTE_Device.h>
#include "Driver_CDC200.h" // Display driver
#include "sys_utils.h"

#include "cmsis_os2.h"

extern osThreadId_t tid_display;

// DAVE
#include "aipl_dave2d.h"
#include "dave_cfg.h"
#include "dave_d0lib.h"

#include "config.h"

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
    // Replace the macro MY_DISP_HOR_RES with the actual screen width.
    #define MY_DISP_HOR_RES    (RTE_PANEL_HACTIVE_TIME)
#endif

#ifndef MY_DISP_VER_RES
    // Replace the macro MY_DISP_HOR_RES with the actual screen height.
    #define MY_DISP_VER_RES    (RTE_PANEL_VACTIVE_LINE)
#endif

/**********************
 *      TYPEDEFS
 **********************/

#pragma pack(1)
#if RTE_CDC200_PIXEL_FORMAT == 0    // ARGB8888
typedef uint32_t Pixel;
#elif RTE_CDC200_PIXEL_FORMAT == 1  // RGB888
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Pixel;
#elif RTE_CDC200_PIXEL_FORMAT == 2  // RGB565
typedef uint16_t Pixel;
#else
#error "CDC200 Unsupported color format"
#endif
#pragma pack()

// DAVE heap
#define D0_HEAP_SIZE 0x00240000
static uint8_t d0_heap[D0_HEAP_SIZE]
    __attribute__((section(".bss.video_mem_heap")));

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void disp_callback(uint32_t event);

/**********************
 *  STATIC VARIABLES
 **********************/

static Pixel lcd_buffer_1[MY_DISP_VER_RES][MY_DISP_HOR_RES]
            __attribute__((section(".bss.lcd_frame_buf"))) = {0};
static Pixel lcd_buffer_2[MY_DISP_VER_RES][MY_DISP_HOR_RES]
            __attribute__((section(".bss.lcd_frame_buf"))) = {0};

enum {
    BUFFER_1 = 0,
    BUFFER_2 = 1,
    NUM_BUFFERS
};

static Pixel* buffers[NUM_BUFFERS] = { (Pixel*)&lcd_buffer_1, (Pixel*)&lcd_buffer_2 };

static uint8_t  current_buffer = BUFFER_1;
static uint32_t frame_durations[NUM_BUFFERS] = { 1, 1 };
static uint32_t switch_times[NUM_BUFFERS] = { 0, 0 };

extern ARM_DRIVER_CDC200 Driver_CDC200;
static ARM_DRIVER_CDC200 *CDCdrv = &Driver_CDC200;

volatile int lcd_content_was_changed=0;


/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
int display_init(void)
{
    /* Initialize CDC driver */
    int ret = CDCdrv->Initialize(disp_callback);
    if(ret != ARM_DRIVER_OK){
        ERROR_PRINT("CDC init failed\n");
        return ret;
    }

    /* Power control CDC */
    ret = CDCdrv->PowerControl(ARM_POWER_FULL);
    if(ret != ARM_DRIVER_OK){
        ERROR_PRINT("CDC Power up failed\n");
        return ret;
    }

    
    ret = CDCdrv->Control(CDC200_SCANLINE0_EVENT, ENABLE);
    if(ret != ARM_DRIVER_OK){
        ERROR_PRINT("CDC controller configuration failed\n");
        return ret;
    }
    

    /* configure CDC controller */
    ret = CDCdrv->Control(CDC200_CONFIGURE_DISPLAY, (uint32_t)disp_active_buffer());
    if(ret != ARM_DRIVER_OK){
        ERROR_PRINT("CDC controller configuration failed\n");
        return ret;
    }

    

    /* Start CDC */
    ret = CDCdrv->Start();
    if(ret != ARM_DRIVER_OK){
        ERROR_PRINT("CDC Start failed\n");
        return ret;
    }

    return ret;
}

void switch_to_next_frame(void)
{
    current_buffer = (current_buffer + 1) % NUM_BUFFERS;
}

void disp_next_frame(void)
{
    CDCdrv->Control(CDC200_FRAMEBUF_UPDATE, (uint32_t)buffers[current_buffer]);
}

void* disp_active_buffer(void)
{
    return buffers[current_buffer];
}

void* disp_inactive_buffer(void)
{
    return buffers[(current_buffer + 1) % NUM_BUFFERS];
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Display errors handler
 */
static void disp_callback(uint32_t event)
{
    if(event & ARM_CDC_DSI_ERROR_EVENT)
    {
        // Transfer Error: Received Hardware error.
        __BKPT(0);
    }
    if (event & ARM_CDC_SCANLINE0_EVENT)
    {
        if (lcd_content_was_changed) 
        {
            lcd_content_was_changed=0;
            switch_to_next_frame();
        }
        disp_next_frame();
        if (tid_display != NULL)
            osThreadFlagsSet(tid_display, LCD_REFRESH_FLAG);
    }
    
}


void configure_display_and_2d() {

#if (D1_MEM_ALLOC == D1_MALLOC_D0LIB)
  /*-------------------------
   * Initialize D/AVE D0 heap
   * -----------------------*/
  if (!d0_initheapmanager(d0_heap, sizeof(d0_heap), d0_mm_fixed_range, NULL, 0,
                          0, 0, d0_ma_unified)) {
    ERROR_PRINT("\r\nError: Heap manager initialization failed\n");
  }
#endif

  // Initialize D/AVE2D
  if (aipl_dave2d_init() != D2_OK) {
    ERROR_PRINT("\r\nError: D/AVE2D initialization falied\n");
    __BKPT(0);
  }

  // init display
  int ret = display_init();

  if (ret != ARM_DRIVER_OK) {
    __BKPT(2);
  }
}