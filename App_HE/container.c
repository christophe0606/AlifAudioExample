
#include "container.h"
#include "md5.h"

#include <string.h>
#include <stdio.h>


#if defined(EXTERNAL_NETWORK)


// Datatypes to save several binaries in flash
// Binary description header
typedef struct {
    uint32_t binary_size;
    const uint8_t *binary_data;
} binary_desc_t;

// List of binaries
struct container_header_t_ {
    uint32_t size;
    uint32_t magic;
    uint32_t nb_binaries;
    binary_desc_t binaries[];
};

static const container_header_t *container_header=(const container_header_t *)(XIP_ADDR_HE);


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

const container_header_t *get_container_description()
{
    return (container_header);
}

const uint8_t * get_binary(uint32_t idx)
{
   return (container_header->binaries[idx].binary_data);
}

uint32_t get_binary_len(uint32_t idx)
{
   return (container_header->binaries[idx].binary_size);
}

static uint32_t get_container_length()
{
    return (container_header->size);
}

int validate_container_description(const char *expected_md5_hex)
{
    unsigned char md5_sum[16];
    char md5_hex[33];

    const container_header_t *header = get_container_description();
    const void *data = (const void *)(header);

    if (header->magic != 0xBEEFDEAD)
    {
        LOG_ERR("Invalid container description header magic: expected 0xBEEFDEAD, got 0x%08X\n", header->magic);
        return 1; // Invalid header
    }

    size_t data_len = get_container_length();

    md5_compute(data, data_len, md5_sum);
    md5_to_hex(md5_sum, md5_hex);

    if (strncmp(md5_hex, expected_md5_hex, 32) != 0)
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
        return 1; // checksum does not match
    }

    return 0; // Valid description
}

int setup_flash()
{
    int err = init_ospi_flash();

    if (!err)
    {
        return 0;
    }
    err = setup_flash_xip();
    
    if (err)
    {
        return 0;
    }
}


#endif
