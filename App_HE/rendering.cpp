#include "RTE_Components.h"
#include "dave_driver.h"
#include "stream/cpp/nodes/Display.hpp"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */

#include "config.h"
#include "disp.h"
#include "rendering.h"
#include <stdio.h>

#include "cstream_node.h"
#include "scheduler.h"

#include "aipl_dave2d.h"


// Gradient
static uint32_t grad_1x256[256];

int init_rendering()
{

    for (int i = 0; i < 256; i++)
    {
        if (i < 128)
        {
            grad_1x256[i] = 0xFF00FF00;
        }
        else if (i < 192)
        {
            grad_1x256[i] = 0xFFFFFF00;
        }
        else
        {
            grad_1x256[i] = 0xFFFF0000;
        }
    }
    return 0;
}

void render_data()
{


    d2_device *handle = aipl_dave2d_handle();
    /* Prepare frame buffer */

    d2_framebuffer(handle, disp_inactive_buffer(), RTE_PANEL_HACTIVE_TIME,
                   RTE_PANEL_HACTIVE_TIME, RTE_PANEL_VACTIVE_LINE,
                   d2_mode_rgb565);

    d2_startframe(handle);

    // d2_setblendmode(handle, d2_bm_alpha, d2_bm_one_minus_alpha);
    // d2_setalphablendmode(handle, d2_bm_one, d2_bm_one_minus_alpha);

    d2_clear(handle, 0xFFFFFFFF);

    d2_settexture(handle, grad_1x256, /*pitch*/ 1, /*width*/ 1, /*height*/ 256,
                  d2_mode_argb8888);

    d2_settexturemode(handle, d2_tm_filter);

    CStreamNode *disp = get_scheduler_node(DISPLAY_ID);
    if (disp)
    {
        Display *display = reinterpret_cast<Display *>(disp);

        display->drawBins(handle, 1);
        display->drawBins(handle, 0);
    }

    d2_endframe(handle);
    //d2_startframe(handle);
    //d2_endframe(handle);

    //disp_next_frame();
    was_changed=1;
    osThreadFlagsWait(LCD_REFRESH_FLAG, osFlagsWaitAny, osWaitForever);

}
