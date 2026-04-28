#include <stdint.h>
#include <string.h>

#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_vstream.h"

#define DBG_CPI_INIT_ERROR (1 << 1)
#define DBG_CPI_POWER_ERROR (1 << 2)
#define DBG_CPI_STOP_ERROR (1 << 3)
#define DBG_CPI_SENSOR_CONFIG_ERROR (1 << 4)
#define DBG_CPI_CONFIG_ERROR (1 << 5)
#define DBG_CPI_EVENT_CONFIG_ERROR (1 << 6)
#define DBG_CPI_UNINITIALIZE_ERROR (1 << 7)
#define DBG_NOT_INITIALIZED_ERROR (1 << 8)
#define DBG_ALREADY_ACTIVE_ERROR (1 << 9)
// Hole for (1 << 10)
#define DBG_NO_BUFFER_ERROR (1 << 11)
#define DBG_BUFFER_FULL_ERROR (1 << 12)
#define DBG_CPI_CAPTURE_FRAME_ERROR (1 << 13)
#define DBG_CPI_CAPTURE_VIDEO_ERROR (1 << 14)
#define DBG_NO_BUFFER_TO_RELEASE_ERROR (1 << 15)

extern void PrintErrors(uint32_t errorCode);

typedef struct vDbgStreamDriver_s {
  int32_t         (*Initialize)   (vStreamEvent_t event_cb);                              ///< Pointer to \ref vStreamInitialize : Initialize Virtual Streaming interface.
  int32_t         (*Uninitialize) (void);                                                 ///< Pointer to \ref vStreamUninitialize : De-initialize Virtual Streaming interface.
  int32_t         (*SetBuf)       (void *buf, uint32_t buf_size, uint32_t block_size);    ///< Pointer to \ref vStreamSetBuf : Set Virtual Streaming data buffer.
  int32_t         (*Start)        (uint32_t mode);                                        ///< Pointer to \ref vStreamStart : Start streaming.
  int32_t         (*Stop)         (void);                                                 ///< Pointer to \ref vStreamStop : Stop streaming.
  void *          (*GetBlock)     (void);                                                 ///< Pointer to \ref vStreamGetBlock : Get pointer to data block.
  int32_t         (*ReleaseBlock) (void);                                                 ///< Pointer to \ref vStreamReleaseBlock : Release data block.
  vStreamStatus_t (*GetStatus)    (void);   
  uint32_t        (*ErrorCode)    (void);                                          ///< Pointer to \ref vStreamGetStatus : Get Virtual Streaming status.
} const vDbgStreamDriver_t;