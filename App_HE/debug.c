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
#include <stdio.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_vstream.h"
#include "debug.h"
#include "Driver_CPI.h"

/* Handle Flags Definitions */
#define FLAGS_INIT      (1U << 0)
#define FLAGS_START     (1U << 1)
#define FLAGS_SINGLE    (1U << 2)
#define FLAGS_LIMIT_OWN (1U << 3)
#define FLAGS_BUF_EMPTY (1U << 4)
#define FLAGS_BUF_FULL  (1U << 5)
#define FLAGS_SYNC      (1U << 6)

/* Low Level Driver Instance */
extern ARM_DRIVER_CPI  Driver_CPI;
static ARM_DRIVER_CPI *DriverCPI = &Driver_CPI;

/* Stream Buffer Type */
typedef struct {
    uint8_t    *data;       /* Data buffer pointer             */
    uint32_t block_num;  /* Number of blocks in data buffer */
    uint32_t block_size; /* Size of block in data buffer    */
} StreamBuf_t;

// Video Driver Configuration Parameters
typedef struct {
    vStreamEvent_t    callback; /* VideoOut callback       */
    StreamBuf_t       buf;      /* VideoOut stream buffer  */
    volatile uint32_t idx_get;  /* Index of block to be returned on GetBlock call     */
    volatile uint32_t idx_rel;  /* Index of block to be released on ReleaseBlock call */
    volatile uint32_t idx_in;   /* Index of block currently beeing streamed           */
    volatile uint32_t errorCode;
    volatile uint8_t  active;   /* Streaming active flag */
    volatile uint8_t  overflow; /* Buffer overflow flag  */
    volatile uint8_t  eos;      /* End of stream flag    */
    volatile uint8_t  flags;    /* Handle status flags   */
} StreamHandle_t;

/* vStream Handle */
static StreamHandle_t hVideoIn = {0};

/* Low Level Driver Callback */
static void DriverCPI_Callback(uint32_t cb_event)
{
    uint32_t event;
    uint32_t buf_index;

    event = 0U;

    if (cb_event & ARM_CPI_EVENT_CAMERA_CAPTURE_STOPPED) {
        /* Stopped capturing frame */
        hVideoIn.active  = 0U;

        // Underlying sensor is only supporting continuous mode
        // So there is the risk than another call to Start would
        // occur too late after the active = 0 has been detected
        // and the sensor would have already started to stream
        // and it would lead to an error : CPI Capture Frame Error
        int32_t status; status = DriverCPI->Stop();
        if (status != ARM_DRIVER_OK) {
            hVideoIn.errorCode |= (uint32_t)DBG_CPI_STOP_ERROR;
        }

        event           |= VSTREAM_EVENT_DATA;

        /* Clear buffer empty flag */
        hVideoIn.flags  &= ~FLAGS_BUF_EMPTY;

        /* Increment index of the block to be streamed */
        hVideoIn.idx_in  = (hVideoIn.idx_in + 1U) % hVideoIn.buf.block_num;

        if (hVideoIn.idx_in == hVideoIn.idx_rel) {
            /* Buffer is full */
            hVideoIn.flags |= FLAGS_BUF_FULL;
        }
    }

    if (cb_event & ARM_CPI_EVENT_CAMERA_FRAME_VSYNC_DETECTED) {
        /* Start of frame */
        if ((hVideoIn.flags & FLAGS_SYNC) == 0U) {
            /* First frame is not yet received */
            hVideoIn.flags |= FLAGS_SYNC;
        } else {
            /* Continuous mode, start of next frame */
            event           |= VSTREAM_EVENT_DATA;

            /* Increment index of the block to be streamed */
            hVideoIn.idx_in  = (hVideoIn.idx_in + 1U) % hVideoIn.buf.block_num;

            /* Clear buffer empty flag */
            hVideoIn.flags  &= ~FLAGS_BUF_EMPTY;

            if (hVideoIn.idx_in == hVideoIn.idx_rel) {
                /* Buffer is full */
                hVideoIn.flags    |= FLAGS_BUF_FULL;

                /* No free stream buffer, overwriting buffer */
                hVideoIn.overflow  = 1U;

                event             |= VSTREAM_EVENT_OVERFLOW;
            }

            /* Continuous mode, determine streaming buffer index */
            buf_index = hVideoIn.idx_in * hVideoIn.buf.block_size;

            /* Set new frame buffer address */
            DriverCPI->CaptureVideo(&hVideoIn.buf.data[buf_index]);
        }
    }

    if (cb_event & (ARM_CPI_EVENT_MIPI_CSI2_ERROR | ARM_CPI_EVENT_ERR_CAMERA_INPUT_FIFO_OVERRUN |
                    ARM_CPI_EVENT_ERR_CAMERA_OUTPUT_FIFO_OVERRUN | ARM_CPI_EVENT_ERR_HARDWARE)) {
        /* Signal End of Stream, app should reset the stream */
        hVideoIn.active  = 0U;
        hVideoIn.eos     = 1U;

        event           |= VSTREAM_EVENT_EOS;
    }

    if ((hVideoIn.callback != NULL) && (event != 0U)) {
        /* Call application callback function */
        hVideoIn.callback(event);
    }
}

/* Initialize streaming interface */
static int32_t Initialize(vStreamEvent_t event_cb)
{
    int32_t  rval;
    int32_t  status;
    uint32_t cfg;

    hVideoIn.callback = event_cb;
    hVideoIn.active   = 0U;
    hVideoIn.overflow = 0U;
    hVideoIn.eos      = 0U;
    hVideoIn.flags    = 0U;
    hVideoIn.errorCode= 0U;

    rval              = VSTREAM_OK;

    /* Initialize and configure low level driver */
    status            = DriverCPI->Initialize(DriverCPI_Callback);
    if (status != ARM_DRIVER_OK) {
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_CPI_INIT_ERROR;
    } else {
        status = DriverCPI->PowerControl(ARM_POWER_FULL);
        if (status != ARM_DRIVER_OK) {
            rval = VSTREAM_ERROR;
            hVideoIn.errorCode |= (uint32_t)DBG_CPI_POWER_ERROR;
        } else {
            status = DriverCPI->Stop();
            if (status != ARM_DRIVER_OK) {
                rval = VSTREAM_ERROR;
                hVideoIn.errorCode |= (uint32_t)DBG_CPI_STOP_ERROR;
            }

            status = DriverCPI->Control(CPI_CAMERA_SENSOR_CONFIGURE, 0U);
            if (status != ARM_DRIVER_OK) {
                rval = VSTREAM_ERROR;
                hVideoIn.errorCode |= (uint32_t)DBG_CPI_SENSOR_CONFIG_ERROR;
            }

            status = DriverCPI->Control(CPI_CONFIGURE, 0U);
            if (status != ARM_DRIVER_OK) {
                rval = VSTREAM_ERROR;
                hVideoIn.errorCode |= (uint32_t)DBG_CPI_CONFIG_ERROR;
            }

            /* Set configuration */
            cfg = ARM_CPI_EVENT_CAMERA_CAPTURE_STOPPED | ARM_CPI_EVENT_CAMERA_FRAME_VSYNC_DETECTED |
                  ARM_CPI_EVENT_ERR_CAMERA_INPUT_FIFO_OVERRUN |
                  ARM_CPI_EVENT_ERR_CAMERA_OUTPUT_FIFO_OVERRUN | ARM_CPI_EVENT_ERR_HARDWARE;

            status = DriverCPI->Control(CPI_EVENTS_CONFIGURE, cfg);
            if (status != ARM_DRIVER_OK) {
                rval = VSTREAM_ERROR;
                hVideoIn.errorCode |= (uint32_t)DBG_CPI_EVENT_CONFIG_ERROR;
            }
        }
    }

    if (rval == VSTREAM_OK) {
        hVideoIn.flags = FLAGS_INIT;
    }

    return rval;
}

/* De-initialize streaming interface */
static int32_t Uninitialize(void)
{
    int32_t rval;
    int32_t status;

    hVideoIn.flags    = 0U;
    hVideoIn.callback = NULL;
    hVideoIn.buf.data = NULL;

    /* De-initialize low level driver */
    status            = DriverCPI->Uninitialize();
    if (status != ARM_DRIVER_OK) {
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_CPI_UNINITIALIZE_ERROR;
    } else {
        rval = VSTREAM_OK;
    }
    return rval;
}

/* Set streaming data buffer */
static int32_t SetBuf(void *buf, uint32_t buf_size, uint32_t block_size)
{
    int32_t rval;

    if (buf == NULL) {
        rval = VSTREAM_ERROR_PARAMETER;
    } else if ((buf_size == 0U) || (block_size == 0U) || (block_size > buf_size)) {
        rval = VSTREAM_ERROR_PARAMETER;
    } else if ((hVideoIn.flags & FLAGS_INIT) == 0) {
        /* Not initialized */
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_NOT_INITIALIZED_ERROR;
    } else if (hVideoIn.active == 1U) {
        /* Streaming is active */
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_ALREADY_ACTIVE_ERROR;
    } else {
        /* Set buffer */
        hVideoIn.buf.data        = (uint8_t *)buf;
        hVideoIn.buf.block_num   = buf_size / block_size;
        hVideoIn.buf.block_size  = block_size;

        /* Buffer is empty */
        hVideoIn.flags          |= FLAGS_BUF_EMPTY;

        /* Reset indexes */
        hVideoIn.idx_in          = 0U;
        hVideoIn.idx_get         = 0U;
        hVideoIn.idx_rel         = 0U;

        rval                     = VSTREAM_OK;
    }

    return rval;
}

/* Start streaming */
static int32_t Start(uint32_t mode)
{
    int32_t rval;
    int32_t status;
    void   *buf;

    if ((hVideoIn.flags & FLAGS_INIT) == 0) {
        /* Not initialized */
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_NOT_INITIALIZED_ERROR;
    } else if (hVideoIn.buf.data == NULL) {
        /* Buffer not set */
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_NO_BUFFER_ERROR;
    } else if ((hVideoIn.flags & FLAGS_BUF_FULL) != 0) {
        /* Buffer is full */
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_BUFFER_FULL_ERROR;
    } else if (hVideoIn.active == 1U) {
        /* Already active */
        rval = VSTREAM_OK;
    } else {
        rval             = VSTREAM_OK;

        /* Set active status */
        hVideoIn.active  = 1U;

        /* Clear SYNC flag */
        hVideoIn.flags  &= ~FLAGS_SYNC;

        /* Set pointer to frame buffer */
        buf              = &hVideoIn.buf.data[hVideoIn.idx_in * hVideoIn.buf.block_size];

        if (mode == VSTREAM_MODE_SINGLE) {
            /* Single mode */
            hVideoIn.flags |= FLAGS_SINGLE;

            /* Start capturing frame */
            status          = DriverCPI->CaptureFrame(buf);
            if (status != ARM_DRIVER_OK) {
                rval = VSTREAM_ERROR;
                hVideoIn.errorCode |= (uint32_t)DBG_CPI_CAPTURE_FRAME_ERROR;
            }
        } else {
            /* Continuous mode */
            hVideoIn.flags &= ~FLAGS_SINGLE;

            /* Start streaming frames */
            status          = DriverCPI->CaptureVideo(buf);
            if (status != ARM_DRIVER_OK) {
                rval = VSTREAM_ERROR;
                hVideoIn.errorCode |= (uint32_t)DBG_CPI_CAPTURE_VIDEO_ERROR;
            }
        }
        if (rval != VSTREAM_OK) {
            /* Clear active flag */
            hVideoIn.active = 0U;
        }
    }

    return rval;
}

/* Stop streaming */
static int32_t Stop(void)
{
    int32_t rval;
    int32_t status;

    if ((hVideoIn.flags & FLAGS_INIT) == 0) {
        /* Not initialized */
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_NOT_INITIALIZED_ERROR;
    } else if (hVideoIn.active == 0U) {
        /* Not active */
        rval = VSTREAM_OK;
    } else {
        /* Stop the stream */
        status = DriverCPI->Stop();
        if (status != ARM_DRIVER_OK) {
            rval = VSTREAM_ERROR;
            hVideoIn.errorCode |= (uint32_t)DBG_CPI_STOP_ERROR;
        } else {
            rval = VSTREAM_OK;
        }

        hVideoIn.active = 0U;
    }

    return rval;
}

/* Get pointer to a data block */
static void *GetBlock(void)
{
    uint32_t buf_index;
    void    *p;

    if (hVideoIn.buf.data == NULL) {
        /* Buffer not set */
        p = NULL;
    } else if ((hVideoIn.flags & FLAGS_LIMIT_OWN) != 0) {
        /* App already owns all the blocks */
        p = NULL;
    } else {
        /* Determine buffer index */
        buf_index = hVideoIn.idx_get * hVideoIn.buf.block_size;

        /* Set return pointer */
        p         = &hVideoIn.buf.data[buf_index];

        /* Invalidate Data Cache */
        RTSS_InvalidateDCache_by_Addr(p, hVideoIn.buf.block_size);

        /* Increment index of block returned by Get */
        hVideoIn.idx_get = (hVideoIn.idx_get + 1U) % hVideoIn.buf.block_num;

        if (hVideoIn.idx_get == hVideoIn.idx_rel) {
            /* App owns all the blocks, set the limit flag */
            hVideoIn.flags |= FLAGS_LIMIT_OWN;
        }
        if (hVideoIn.idx_get == hVideoIn.idx_in) {
            /* Buffer is empty */
            hVideoIn.flags |= FLAGS_BUF_EMPTY;
        }
        /* Clear buffer full flag */
        hVideoIn.flags &= ~FLAGS_BUF_FULL;
    }

    return p;
}

/* Release data block */
static int32_t ReleaseBlock(void)
{
    int32_t rval;

    if (hVideoIn.buf.data == NULL) {
        /* Buffer not set */
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_NO_BUFFER_ERROR;
    } else if ((hVideoIn.idx_rel == hVideoIn.idx_get) &&
               ((hVideoIn.flags & FLAGS_LIMIT_OWN) == 0)) {
        /* No blocks to release */
        rval = VSTREAM_ERROR;
        hVideoIn.errorCode |= (uint32_t)DBG_NO_BUFFER_TO_RELEASE_ERROR;
    } else {
        /* Increment index of the block to be released next */
        hVideoIn.idx_rel  = (hVideoIn.idx_rel + 1U) % hVideoIn.buf.block_num;

        /* Clear the limit get flag */
        hVideoIn.flags   &= ~FLAGS_LIMIT_OWN;

        rval              = VSTREAM_OK;
    }

    return rval;
}

/* Get Audio Interface status */
static vStreamStatus_t GetStatus(void)
{
    vStreamStatus_t status;

    /* Get status */
    status.active     = hVideoIn.active;
    status.overflow   = hVideoIn.overflow;
    status.eos        = hVideoIn.eos;

    /* Clear status */
    hVideoIn.overflow = 0U;
    hVideoIn.eos      = 0U;

    return status;
}

uint32_t ErrorCode(void)
{
    return hVideoIn.errorCode;
}

vDbgStreamDriver_t Driver_vDbgStreamVideoIn = {
    Initialize,
    Uninitialize,
    SetBuf,
    Start,
    Stop,
    GetBlock,
    ReleaseBlock,
    GetStatus,
    ErrorCode
};

void PrintErrors(uint32_t errorCode)
{
    if (errorCode & DBG_CPI_INIT_ERROR) {
        printf("CPI Initialization Error\n");
    }
    if (errorCode & DBG_CPI_POWER_ERROR) {
        printf("CPI Power Error\n");
    }
    if (errorCode & DBG_CPI_STOP_ERROR) {
        printf("CPI Stop Error\n");
    }
    if (errorCode & DBG_CPI_SENSOR_CONFIG_ERROR) {
        printf("CPI Sensor Configuration Error\n");
    }
    if (errorCode & DBG_CPI_CONFIG_ERROR) {
        printf("CPI Configuration Error\n");
    }
    if (errorCode & DBG_CPI_EVENT_CONFIG_ERROR) {
        printf("CPI Event Configuration Error\n");
    }
    if (errorCode & DBG_CPI_UNINITIALIZE_ERROR) {
        printf("CPI Uninitialization Error\n");
    }
    if (errorCode & DBG_NOT_INITIALIZED_ERROR) {
        printf("Not Initialized Error\n");
    }
    if (errorCode & DBG_ALREADY_ACTIVE_ERROR) {
        printf("Already Active Error\n");
    }
    if (errorCode & DBG_NO_BUFFER_ERROR) {
        printf("No Buffer Error\n");
    }
    if (errorCode & DBG_BUFFER_FULL_ERROR) {
        printf("Buffer Full Error\n");
    }
    if (errorCode & DBG_CPI_CAPTURE_FRAME_ERROR) {
        printf("CPI Capture Frame Error\n");
    }
    if (errorCode & DBG_CPI_CAPTURE_VIDEO_ERROR) {
        printf("CPI Capture Video Error\n");
    }
    if (errorCode & DBG_NO_BUFFER_TO_RELEASE_ERROR) {
        printf("No Buffer To Release Error\n");
    }
}