
#include "cg_enums.h"
#include "app_config.hpp"
#include <cstdio>

#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "cstream_node.h"
#include "m-profile/armv7m_cachel1.h"

extern "C"
{
#include "RTE_Components.h"
#include CMSIS_device_header

#include "cmsis_os2.h" /* CMSIS-RTOS2 API */
#include "config.h"
#include "scheduler.h"

    extern int app_main(void);
}

#include "cg_queue.hpp"

#include "appnodes/AppDisplay.hpp"
#include "nodes/VStreamAudioSink.hpp"
#include "nodes/VStreamVideoSource.hpp"

#include "cg_queue.hpp"

#if defined(EXTERNAL_NETWORK)
extern "C"
{
#include "md5.h"
#include "network.h"
}
#endif

#define NB_MAX_EVENTS 20
#define NB_MAX_BUFS 20

using namespace arm_cmsis_stream;

extern "C"
{
    extern osThreadId_t tid_interrupts;
    extern osThreadId_t tid_stream;

    extern osThreadId_t cg_eventThread;
}

osMemoryPoolId_t cg_eventPool = nullptr;
osMemoryPoolId_t cg_bufPool = nullptr;
osMemoryPoolId_t cg_mutexPool = nullptr;

osThreadId_t tid_stream = nullptr;
osThreadId_t cg_eventThread = nullptr;
osThreadId_t tid_interrupts = nullptr;

// osMutexId_t bin_mutex;

/* Camera frame buffer (RAW8 or RGB565) */
uint8_t CAM_Frame[CAMERA_BUFFER_SIZE] CAMERA_FRAME_BUF_ATTRIBUTE;

/* Display frame buffer */
uint8_t LCD_Frame[DISPLAY_BUFFER_SIZE] DISPLAY_FRAME_BUF_ATTRIBUTE;

static arm_cmsis_stream::EventQueue *queue = nullptr;


int init_memory_pools()
{

    DEBUG_PRINT("Create event pool\n");
    cg_eventPool = osMemoryPoolNew(NB_MAX_EVENTS, sizeof(ListValue) + 16, NULL);
    if (cg_eventPool == NULL)
    {
        ERROR_PRINT("Can't create CMSIS Event Pool of size %d\n", NB_MAX_EVENTS * (sizeof(ListValue) + 16));
        return 1;
    }

    DEBUG_PRINT("Create buf pool\n");
    cg_bufPool = osMemoryPoolNew(NB_MAX_BUFS, sizeof(Tensor<double>) + 16, NULL);
    if (cg_bufPool == NULL)
    {
        ERROR_PRINT("Can't create CMSIS buf Pool of size %d\n", NB_MAX_BUFS * (sizeof(Tensor<double>) + 16));
        osMemoryPoolDelete(cg_eventPool);
        return 1;
    }

    DEBUG_PRINT("Create mutex pool\n");
    cg_mutexPool = osMemoryPoolNew(NB_MAX_BUFS, sizeof(CG_MUTEX) + 16, NULL);
    if (cg_mutexPool == NULL)
    {
        ERROR_PRINT("Can't create CMSIS mutex Pool of size %d\n", NB_MAX_BUFS * (sizeof(CG_MUTEX) + 16));
        osMemoryPoolDelete(cg_eventPool);
        osMemoryPoolDelete(cg_bufPool);
        return 1;
    }

    return 0;
}

void VideoSrc_Event_Callback(uint32_t event)
{
    if (event & VSTREAM_EVENT_EOS)
    {
        ERROR_PRINT("Camera end of stream\n");
        return;
    }

    if (event & VSTREAM_EVENT_DATA)
    {
        /* LCD frame is available */
        if (tid_interrupts != NULL)
            osThreadFlagsSet(tid_interrupts, VIDEO_SRC_EVT);
    }
}

VStreamVideoSource *video_src = nullptr;

void interrupt_thread(void *arg)
{
    DEBUG_PRINT("Interrupt thread started\n");

    for (;;)
    {
        // Wait for interrupt event
        uint32_t res = osThreadFlagsWait(VIDEO_SRC_EVT, osFlagsWaitAny, osWaitForever);
        if (video_src && (res & VIDEO_SRC_EVT))
        {
            auto destination = LocalDestination{video_src, 0};
            auto evt = Event(kDo, kHighPriority);

            DEBUG_PRINT("Push event for video src\n");
            bool ok = queue->push(destination, std::move(evt));
            if (!ok)
            {
                ERROR_PRINT("Event queue overflow for video src\n");
            }
        }
    }

    // Cleanup and exit the thread if needed
    DEBUG_PRINT("Interrupt thread exit\n");
    osThreadExit();
}

void event_thread(void *argument)
{

    DEBUG_PRINT("Event thread started\n");

    queue->execute();

    // Delete the event queue
    delete queue;

err_evt:
    DEBUG_PRINT("Event thread exit\n");
    osMemoryPoolDelete(cg_eventPool);
    osMemoryPoolDelete(cg_bufPool);
    osMemoryPoolDelete(cg_mutexPool);

    osThreadExit();
}

void stream_thread(void *arg)
{
    uint32_t nb_iter;
    int error;
    DEBUG_PRINT("Stream thread started\n");

    nb_iter = scheduler(&error);
    if (error != 0)
    {
        ERROR_PRINT("Scheduler error %d\n", error);
    }
    DEBUG_PRINT("Scheduler done after %d iterations\n", nb_iter);

err_stream:
    DEBUG_PRINT("End stream thread\n");
    free_scheduler();
    osThreadExit();
}

#if defined(EXTERNAL_NETWORK)
#include "Driver_IO.h"
#include "pinconf.h"
#include "ospi_xip_user.h"
#include "setup_flash_xip.h"

#define OSPI_RESET_PORT 15
#define OSPI_RESET_PIN  7

extern ARM_DRIVER_GPIO ARM_Driver_GPIO_(OSPI_RESET_PORT);
ARM_DRIVER_GPIO       *GPIODrv = &ARM_Driver_GPIO_(OSPI_RESET_PORT);

int init_ospi_flash(void)
{
    int32_t ret;

        ret = pinconf_set(OSPI1_D0_PORT,
                      OSPI1_D0_PIN,
                      OSPI1_D0_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_D1_PORT,
                      OSPI1_D1_PIN,
                      OSPI1_D1_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_D2_PORT,
                      OSPI1_D2_PIN,
                      OSPI1_D2_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_D3_PORT,
                      OSPI1_D3_PIN,
                      OSPI1_D3_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_D4_PORT,
                      OSPI1_D4_PIN,
                      OSPI1_D4_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_D5_PORT,
                      OSPI1_D5_PIN,
                      OSPI1_D5_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_D6_PORT,
                      OSPI1_D6_PIN,
                      OSPI1_D6_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_D7_PORT,
                      OSPI1_D7_PIN,
                      OSPI1_D7_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_RXDS_PORT,
                      OSPI1_RXDS_PIN,
                      OSPI1_RXDS_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST |
                          PADCTRL_READ_ENABLE);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_SCLK_PORT,
                      OSPI1_SCLK_PIN,
                      OSPI1_SCLK_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA | PADCTRL_SLEW_RATE_FAST);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_CS_PORT,
                      OSPI1_CS_PIN,
                      OSPI1_CS_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA);
    if (ret) {
        return 0;
    }

    ret = pinconf_set(OSPI1_SCLKN_PORT,
                      OSPI1_SCLKN_PIN,
                      OSPI1_SCLKN_PIN_FUNCTION,
                      PADCTRL_OUTPUT_DRIVE_STRENGTH_12MA);
    if (ret) {
        return 0;
    }

    ret = GPIODrv->Initialize(OSPI_RESET_PIN, NULL);
    if (ret != ARM_DRIVER_OK) {
        ERROR_PRINT("Failed to initialize GPIO for OSPI reset\n");
        return 0;
    }

    ret = GPIODrv->PowerControl(OSPI_RESET_PIN, ARM_POWER_FULL);
    if (ret != ARM_DRIVER_OK) {
        ERROR_PRINT("Failed to set power for GPIO OSPI reset\n");
        return 0;
    }

    ret = GPIODrv->SetDirection(OSPI_RESET_PIN, GPIO_PIN_DIRECTION_OUTPUT);
    if (ret != ARM_DRIVER_OK) {
        ERROR_PRINT("Failed to set direction for GPIO OSPI reset\n");
        return 0;
    }

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_LOW);
    if (ret != ARM_DRIVER_OK) {
        ERROR_PRINT("Failed to set value LOW for GPIO OSPI reset\n");
        return 0;
    }

    ret = GPIODrv->SetValue(OSPI_RESET_PIN, GPIO_PIN_OUTPUT_STATE_HIGH);
    if (ret != ARM_DRIVER_OK) {
        ERROR_PRINT("Failed to set value HIGH for GPIO OSPI reset\n");
        return 0;
    }

    return 1;
}
int check_network(const char *expected_md5_hex)
{
    int err =init_ospi_flash();

    if (!err)
    {
        return 0;
    }
    err = setup_flash_xip();
    
    if (err)
    {
        return 0;
    }


    unsigned char md5_sum[16];
    char md5_hex[33];
#if 0

    const uint16_t *data = (const uint16_t *)get_network_description();
    int nb = 0;
    printf("0x%08X\n" , (unsigned int)data);
    for(int i=0;i<64;i++)
    {
        printf("%04X " , data[i]);
        nb++;
        if (nb == 8)
        {
            printf("\n");
            nb = 0;
        }
    }
    
    return 0;
#else
    
    const void *data = get_network_description();

    size_t data_len = get_description_length();
    printf("Network length: %d bytes\n", (int)data_len);

    md5_compute(data, data_len, md5_sum);
    md5_to_hex(md5_sum, md5_hex);

    if (std::strncmp(md5_hex, expected_md5_hex, 32) != 0)
    {
        for(int i = 0; i < 32; i++)
        {
            printf("%c", md5_hex[i]);
        }
        printf("\n");

        for(int i = 0; i < 32; i++)
        {
            printf("%c", expected_md5_hex[i]);
        }
        printf("\n");
        return 0; // checksum does not match
    }
    return 1; // checksum matches
#endif
}
#endif

int app_main(void)
{
    CStreamNode *c_video_src = nullptr;

    const osThreadAttr_t dispAttr = {
        .stack_size = 4096,
    };

    const osThreadAttr_t eventAttr = {
        .stack_size = 4096,
        .priority = osPriorityHigh};

    const osThreadAttr_t interruptAttr = {
        .stack_size = 4024,
        .priority = osPriorityHigh};

    const osThreadAttr_t audioAttr = {.stack_size = 4096,
                                      .priority = osPriorityRealtime};

#if defined(EXTERNAL_NETWORK)
    if (!check_network("c147eeaf87c900ff230950424d07ca07"))
    {
        ERROR_PRINT("Neural network checksum error\n");
        return -1;
    }
    else {
        DEBUG_PRINT("Neural network checksum OK\n");
    }
#endif
    osKernelInitialize();

    tid_interrupts = osThreadNew(interrupt_thread, NULL, &interruptAttr);

    cg_eventThread = osThreadNew(event_thread, NULL, &eventAttr);
    tid_stream = osThreadNew(stream_thread, NULL, &audioAttr);

    int err = init_memory_pools();
    if (err != 0)
    {
        goto err_main;
    }

    // bin_mutex = osMutexNew(NULL);

    queue = new (std::nothrow) MyQueue(osPriorityLow, osPriorityNormal, osPriorityHigh);
    if (queue == nullptr)
    {
        ERROR_PRINT("Can't create CMSIS Event Queue\n");
        goto err_main;
    }

    DEBUG_PRINT("Create nodes\n");
    // Init nodes and starts audio stream
    err = init_scheduler(queue);
    if (err != CG_SUCCESS)
    {
        ERROR_PRINT("Error: Failure during scheduler initialization.\n");
        goto err_main;
    }

#if defined(VIDEOSOURCE_ID)
    c_video_src = get_scheduler_node(VIDEOSOURCE_ID);
    if (c_video_src == nullptr)
    {
        ERROR_PRINT("No video source node found\n");
        osThreadExit();
    }
    video_src = reinterpret_cast<VStreamVideoSource *>(c_video_src->obj);

#else
    video_src = nullptr;
#endif

    DEBUG_PRINT("Scheduler initialized successfully\n");

    osKernelStart();

err_main:
    return 0;
}
