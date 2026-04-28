/*---------------------------------------------------------------------------
 * Copyright (c) 2025 Arm Limited (or its affiliates).
 * All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *---------------------------------------------------------------------------*/

#include <stdint.h>
#include <string.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "Driver_CDC200.h"
#include "lcd.h"

/* Handle Flags Definitions */
#define FLAGS_INIT      (1U << 0)
#define FLAGS_START     (1U << 1)

/* Low Level Driver Instance */
extern ARM_DRIVER_CDC200  Driver_CDC200;
static ARM_DRIVER_CDC200 *DriverCDC = &Driver_CDC200;

/* Stream Buffer Type */
typedef struct {
    uint8_t    *data;       /* Data buffer pointer             */
    uint32_t block_size; /* Size of block in data buffer    */
} DisplayBuf_t;

// Video Driver Configuration Parameters
typedef struct {
    displayEvent_t    callback;  /* VideoOut callback       */
    DisplayBuf_t      buf;       /* VideoOut stream buffer  */
    volatile uint32_t idx_current;
    volatile uint8_t  active;    /* Streaming active flag */
    volatile uint8_t  eos;    /* Streaming active flag */
    uint8_t           flags;
} DisplayHandle_t;

/* Display Handle */
static DisplayHandle_t hVideoOut = {0};

/* Low Level Driver Callback */
static void DriverCDC_Callback(uint32_t cb_event)
{
    uint32_t event;
    uint32_t buf_index;

    event = 0U;

    if (cb_event & ARM_CDC_SCANLINE0_EVENT) {
        /* Start of frame */
        event             |= DISPLAY_EVENT_NEW_FRAME;
        hVideoOut.active = 0U;
    }

    if (cb_event & ARM_CDC_DSI_ERROR_EVENT) {
        /* Error, stream stopped */
        event            |= DISPLAY_EVENT_EOS;

        hVideoOut.active  = 0U;
        hVideoOut.eos     = 1U;
    }

    if ((hVideoOut.callback != NULL) && (event != 0U)) {
        /* Call application callback function */
        hVideoOut.callback(event);
    }
}

/* Initialize streaming interface */
static int32_t Initialize(displayEvent_t event_cb)
{
    int32_t rval;
    int32_t status;

    hVideoOut.callback  = event_cb;
    hVideoOut.active    = 0U;
    hVideoOut.eos       = 0U;
    hVideoOut.flags     = 0U;

    rval                = DISPLAY_OK;

    /* Initialize and configure low level driver */
    status              = DriverCDC->Initialize(DriverCDC_Callback);
    if (status != ARM_DRIVER_OK) {
        rval = DISPLAY_ERROR;
    } else {
        status = DriverCDC->PowerControl(ARM_POWER_FULL);
        if (status != ARM_DRIVER_OK) {
            rval = DISPLAY_ERROR;
        } else {
            status = DriverCDC->Stop();
            if (status != ARM_DRIVER_OK) {
                rval = DISPLAY_ERROR;
            }

            /* Enable scanline event (i.e. start of frame) */
            status = DriverCDC->Control(CDC200_SCANLINE0_EVENT, 1U);
            if (status != ARM_DRIVER_OK) {
                rval = DISPLAY_ERROR;
            }

            /* Configure display */
            status = DriverCDC->Control(CDC200_CONFIGURE_DISPLAY, 0U);
            if (status != ARM_DRIVER_OK) {
                rval = DISPLAY_ERROR;
            }
        }
    }

    if (rval == DISPLAY_OK) {
        hVideoOut.flags = FLAGS_INIT;
    }

    return rval;
}

/* De-initialize streaming interface */
static int32_t Uninitialize(void)
{
    int32_t rval;
    int32_t status;

    hVideoOut.flags    = 0U;
    hVideoOut.callback = NULL;
    hVideoOut.buf.data = NULL;

    /* De-initialize low level driver */
    status             = DriverCDC->Uninitialize();
    if (status != ARM_DRIVER_OK) {
        rval = DISPLAY_ERROR;
    } else {
        rval = DISPLAY_OK;
    }
    return rval;
}

/* Set streaming data buffer */
static int32_t SetBuf(void *buf, uint32_t block_size)
{
    int32_t rval;

    if (buf == NULL) {
        rval = DISPLAY_ERROR_PARAMETER;
    } else if ((block_size == 0U)) {
        rval = DISPLAY_ERROR_PARAMETER;
    } else if ((hVideoOut.flags & FLAGS_INIT) == 0) {
        /* Not initialized */
        rval = DISPLAY_ERROR;
    } else if (hVideoOut.active == 1U) {
        /* Streaming is active */
        rval = DISPLAY_ERROR;
    } else {
        /* Set buffer */
        hVideoOut.buf.data        = (uint8_t *)buf;
        hVideoOut.buf.block_size  = block_size;

        /* Buffer is empty */

        /* Reset indexes */
        hVideoOut.idx_current         = 0U;

        rval                      = DISPLAY_OK;
    }

    return rval;
}

/* Start streaming */
static int32_t Start(uint32_t mode)
{
    int32_t rval;
    int32_t status;
    void   *buf;

    if ((hVideoOut.flags & FLAGS_INIT) == 0) {
        /* Not initialized */
        rval = DISPLAY_ERROR;
    } else if (hVideoOut.buf.data == NULL) {
        /* Buffer not set */
        rval = DISPLAY_ERROR;
    } else if (hVideoOut.active == 1U) {
        /* Already active */
        rval = DISPLAY_OK;
    } else {
        rval             = DISPLAY_OK;

        /* Set active status */
        hVideoOut.active = 1U;

        /* Set pointer to frame buffer */
        buf              = &hVideoOut.buf.data[hVideoOut.idx_current * hVideoOut.buf.block_size];
       
        /* Set frame buffer address */
        status = DriverCDC->Control(CDC200_FRAMEBUF_UPDATE, (uint32_t) buf);
        if (status != ARM_DRIVER_OK) {
            rval = DISPLAY_ERROR;
        } else {
            /* Start streaming */
            if ((hVideoOut.flags & FLAGS_START) == 0) {
                hVideoOut.flags |= FLAGS_START;
                /* Underlying driver supports only continuous mode, hence  */
                /* each subsequent call only updates frame buffer address. */

                /* Start display stream */
                status           = DriverCDC->Start();
                if (status != ARM_DRIVER_OK) {
                    rval = DISPLAY_ERROR;
                }
            }
        }
        if (rval != DISPLAY_OK) {
            /* Clear active flag */
            hVideoOut.active = 0U;
        }
    }

    return rval;
}

/* Stop streaming */
static int32_t Stop(void)
{
    int32_t rval;
    int32_t status;

    if ((hVideoOut.flags & FLAGS_INIT) == 0) {
        /* Not initialized */
        rval = DISPLAY_ERROR;
    } else if (hVideoOut.active == 0U) {
        /* Not active */
        rval = DISPLAY_OK;
    } else {
        /* Stop the stream */
        status = DriverCDC->Stop();
        if (status != ARM_DRIVER_OK) {
            rval = DISPLAY_ERROR;
        } else {
            rval = DISPLAY_OK;
        }

        /* Enable call of the CDC driver Start() function */
        hVideoOut.flags  &= ~FLAGS_START;

        hVideoOut.active  = 0U;
    }

    return rval;
}

/* Get pointer to a data block */
static void *GetRenderingBlock(void)
{
    uint32_t buf_index;
    void    *p;

    if (hVideoOut.buf.data == NULL) {
        /* Buffer not set */
        p = NULL;
    } else {
        /* Determine buffer index */
        buf_index         = (1-hVideoOut.idx_current) * hVideoOut.buf.block_size;

        /* Set return pointer */
        p                 = &hVideoOut.buf.data[buf_index];

    }

    return p;
}

static void SwitchBuffers(void)
{
    void *p = GetRenderingBlock();
    RTSS_CleanInvalidateDCache_by_Addr(p, hVideoOut.buf.block_size);

    hVideoOut.idx_current = 1 - hVideoOut.idx_current;
}


/* Get Audio Interface status */
static displayStatus_t GetStatus(void)
{
    displayStatus_t status;

    /* Get status */
    status.active       = hVideoOut.active;
    status.eos          = hVideoOut.eos;

    /* Clear status */
    hVideoOut.eos       = 0U;

    return status;
}

displayDriver_t Driver_display = {
    Initialize,
    Uninitialize,
    SetBuf,
    Start,
    Stop,
    GetRenderingBlock,
    GetStatus,
    SwitchBuffers
};
