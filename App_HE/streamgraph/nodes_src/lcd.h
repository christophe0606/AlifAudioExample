/******************************************************************************
 * @file     display.h
 * @brief    display header file
 * @version  V1.0.0
 * @date     2. April 2025
 ******************************************************************************/
/*
 * Copyright (c) 2025 Arm Limited. All rights reserved.
 *
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
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#ifdef  __cplusplus
extern  "C"
{
#endif

#include <stdint.h>

// DISPLAY EVENT
#define DISPLAY_EVENT_NEW_FRAME             (1UL)       ///< New frame event
#define DISPLAY_EVENT_EOS               (1UL << 3)  ///< End of stream

// Virtual Streaming Return Codes
#define DISPLAY_OK                      (0)         ///< Operation succeeded
#define DISPLAY_ERROR                   (-1)        ///< Unspecified error
#define DISPLAY_ERROR_PARAMETER         (-2)        ///< Parameter error

// Virtual Streaming Status
typedef struct {
  uint32_t active       :  1;           ///< Streaming active
  uint32_t overflow     :  1;           ///< Data buffer overflow  (cleared on GetStatus)
  uint32_t underflow    :  1;           ///< Data buffer underflow (cleared on GetStatus)
  uint32_t eos          :  1;           ///< End Of Stream         (cleared on GetStatus)
  uint32_t reserved     : 28;
} displayStatus_t;

typedef void (*displayEvent_t) (uint32_t event_flags);  ///< Pointer to \ref displayEvent : Handling of Display Events.


typedef struct displayDriver_s {
  int32_t         (*Initialize)   (displayEvent_t event_cb);                              ///< Pointer to \ref vStreamInitialize : Initialize Virtual Streaming interface.
  int32_t         (*Uninitialize) (void);                                                 ///< Pointer to \ref vStreamUninitialize : De-initialize Virtual Streaming interface.
  int32_t         (*SetBuf)       (void *buf, uint32_t block_size);    ///< Pointer to \ref vStreamSetBuf : Set Virtual Streaming data buffer.
  int32_t         (*Start)        ();                                        ///< Pointer to \ref vStreamStart : Start streaming.
  int32_t         (*Stop)         (void);                                                 ///< Pointer to \ref vStreamStop : Stop streaming.
  void *          (*GetRenderingBlock)     (void);                                                 ///< Pointer to \ref vStreamGetBlock : Get pointer to data block.
  displayStatus_t (*GetStatus)    (void);                                                 ///< Pointer to \ref vStreamGetStatus : Get Virtual Streaming status.
  void            (*SwitchBuffers)(void);
} const displayDriver_t;

#ifdef  __cplusplus
}
#endif

#endif  /* CMSIS_VSTREAM_H_ */
