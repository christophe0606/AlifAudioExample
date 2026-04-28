#ifndef config_h
#define config_h

#include "RTE_Components.h"
#include CMSIS_device_header
#include <RTE_Device.h>
#include "Driver_CDC200.h" // Display driver

#define NB_BINS 128

#define SAMPLE_RATE 16000

#include <stdio.h>
#include <stdint.h>


#define VIDEO_SRC_EVT (0x1)
#define VIDEO_SINK_EVT (0x2)
extern void VideoSink_Event_Callback(uint32_t event);
extern void VideoSrc_Event_Callback(uint32_t event);


// <h>Camera Configuration
// =======================

//  <o>Camera Frame Width
//  <i> Define the camera frame width.
//  <i> Common camera frame widths: 320, 640, 1024, 1280.
//  <i> Default: 640
#ifndef CAMERA_FRAME_WIDTH
#define CAMERA_FRAME_WIDTH          320
#endif

//  <o>Camera Frame Height
//  <i> Define the camera frame height.
//  <i> Common camera frame heights: 240, 480, 600, 720.
//  <i> Default: 480
#ifndef CAMERA_FRAME_HEIGHT
#define CAMERA_FRAME_HEIGHT         320
#endif

//  <o>Camera buffers
//  <i> Number of camera buffer for double buffering
//  <i> Common values: 1,2.
//  <i> Default: 1
#ifndef CAMERA_NB_BUFFERS
#define CAMERA_NB_BUFFERS       1
#endif


//  <o>Frame Type <0=>RAW8 <1=>RGB565
//  <i> Define whether camera frame is raw or RGB.
//  <i> Default: 0
#ifndef CAMERA_FRAME_TYPE
#define CAMERA_FRAME_TYPE           1
#endif

//  <o>Frame Bayer Pattern <0=>RGGB <1=>BGGR <2=>GRBG <3=>GBRG
//  <i> Define the raw camera frame Bayer pattern.
//  <i> Default: 3
#ifndef CAMERA_FRAME_BAYER
#define CAMERA_FRAME_BAYER          3
#endif

//  <s>Frame Buffer Section Name
//  <i> Define the name of the camera frame buffer section.
//  <i> Default: ".bss.camera_frame_buf"
#ifndef CAMERA_FRAME_BUF_SECTION
#define CAMERA_FRAME_BUF_SECTION    ".bss.camera_frame_buf"
#endif

//  <o>Frame Buffer Alignment
//  <i> Define the camera frame buffer alignment in bytes.
//  <i> Default: 32
#ifndef CAMERA_FRAME_BUF_ALIGNMENT
#define CAMERA_FRAME_BUF_ALIGNMENT  32
#endif



// </h>

// <h>Display Configuration
// ========================

//  <o>Display Frame Width
//  <i> Defines the display frame width.
//  <i> Common display frame widths: 480, 800, 1024, 1280.
//  <i> Default: 800
#ifndef DISPLAY_FRAME_WIDTH
#define DISPLAY_FRAME_WIDTH         480
#endif

//  <o>Display Frame Height
//  <i> Defines the display frame height.
//  <i> Common display frame heights: 320, 480, 600, 800.
//  <i> Default: 480
#ifndef DISPLAY_FRAME_HEIGHT
#define DISPLAY_FRAME_HEIGHT        800
#endif


//  <s>Frame Buffer Section Name
//  <i> Define the name of the display frame buffer section
//  <i> Default: ".bss.lcd_frame_buf"
#ifndef DISPLAY_FRAME_BUF_SECTION
#define DISPLAY_FRAME_BUF_SECTION   ".bss.lcd_frame_buf"
#endif

//  <o>Frame Buffer Alignment
//  <i> Define the display frame buffer alignment in bytes
//  <i> Default: 32
#ifndef DISPLAY_FRAME_BUF_ALIGNMENT
#define DISPLAY_FRAME_BUF_ALIGNMENT 32
#endif

/* Number of bytes per pixel for display */
#define DISPLAY_FRAME_COLOR_BYTES   2

// </h>

/* Attributes applied to the camera frame buffer object */
#define CAMERA_FRAME_BUF_ATTRIBUTE \
  __attribute__((section(CAMERA_FRAME_BUF_SECTION), aligned(CAMERA_FRAME_BUF_ALIGNMENT)))

  /* Attributes applied to the display frame buffer object */
#define DISPLAY_FRAME_BUF_ATTRIBUTE \
  __attribute__((section(DISPLAY_FRAME_BUF_SECTION), aligned(DISPLAY_FRAME_BUF_ALIGNMENT)))


  /* Frame type */
#define CAMERA_FRAME_TYPE_RAW8    0U
#define CAMERA_FRAME_TYPE_RGB565  1U

/* Define input image bit depth */
#if (CAMERA_FRAME_TYPE == CAMERA_FRAME_TYPE_RAW8)
#define CAMERA_FRAME_COLOR_BYTES 1
#elif (CAMERA_FRAME_TYPE == CAMERA_FRAME_TYPE_RGB565)
#define CAMERA_FRAME_COLOR_BYTES 2
#else
#error "Camera frame type not supported, check CAMERA_FRAME_TYPE definition."
#endif




/* Define camera RAW frame size */
#define CAMERA_FRAME_SIZE      (CAMERA_FRAME_WIDTH * CAMERA_FRAME_HEIGHT * CAMERA_FRAME_COLOR_BYTES)
#define CAMERA_BUFFER_SIZE     (CAMERA_NB_BUFFERS * CAMERA_FRAME_SIZE)

/* Define display image size */
#define DISPLAY_IMAGE_SIZE     (DISPLAY_FRAME_WIDTH * DISPLAY_FRAME_HEIGHT * DISPLAY_FRAME_COLOR_BYTES)
#define DISPLAY_BUFFER_SIZE    (2 * DISPLAY_IMAGE_SIZE)

extern uint8_t CAM_Frame[CAMERA_BUFFER_SIZE];

/* Display frame buffer */
extern uint8_t LCD_Frame[DISPLAY_BUFFER_SIZE];

#endif