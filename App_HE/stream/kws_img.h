#ifndef KWS_IMG_H
#define KWS_IMG_H

#include <stdint.h>

// Image for word: DOWN
#define DOWN_WIDTH 37
#define DOWN_HEIGHT 9

extern const uint8_t down_img[DOWN_WIDTH * DOWN_HEIGHT];

// End of DOWN

// Image for word: GO
#define GO_WIDTH 18
#define GO_HEIGHT 9

extern const uint8_t go_img[GO_WIDTH * GO_HEIGHT];

// End of GO

// Image for word: LEFT
#define LEFT_WIDTH 30
#define LEFT_HEIGHT 9

extern const uint8_t left_img[LEFT_WIDTH * LEFT_HEIGHT];

// End of LEFT

// Image for word: NO
#define NO_WIDTH 18
#define NO_HEIGHT 9

extern const uint8_t no_img[NO_WIDTH * NO_HEIGHT];

// End of NO

// Image for word: OFF
#define OFF_WIDTH 23
#define OFF_HEIGHT 9

extern const uint8_t off_img[OFF_WIDTH * OFF_HEIGHT];

// End of OFF

// Image for word: ON
#define ON_WIDTH 17
#define ON_HEIGHT 9

extern const uint8_t on_img[ON_WIDTH * ON_HEIGHT];

// End of ON

// Image for word: RIGHT
#define RIGHT_WIDTH 38
#define RIGHT_HEIGHT 9

extern const uint8_t right_img[RIGHT_WIDTH * RIGHT_HEIGHT];

// End of RIGHT

// Image for word: STOP
#define STOP_WIDTH 32
#define STOP_HEIGHT 9

extern const uint8_t stop_img[STOP_WIDTH * STOP_HEIGHT];

// End of STOP

// Image for word: UP
#define UP_WIDTH 17
#define UP_HEIGHT 9

extern const uint8_t up_img[UP_WIDTH * UP_HEIGHT];

// End of UP

// Image for word: YES
#define YES_WIDTH 24
#define YES_HEIGHT 9

extern const uint8_t yes_img[YES_WIDTH * YES_HEIGHT];

// End of YES

extern const uint32_t kws_widths[10];
extern const uint32_t kws_heights[10];

#endif // KWS_IMG_H
