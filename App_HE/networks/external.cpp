#if defined(EXTERNAL_NETWORK)

#include "RTE_Components.h"
#include CMSIS_device_header

#include "app_mem_regions.h"

#include <cstddef>
#include <cstdint>

extern "C"
{
extern void *get_network_description();
extern size_t get_description_length();
};

typedef struct {
    uint32_t nn_model_size;
    uint8_t *nn_model_data;
} nn_model_t;

typedef struct {
    uint32_t size;
    uint32_t magic;
    uint32_t nb_models;
    nn_model_t *models;
} nn_header_t;

static const nn_header_t *nn_header=reinterpret_cast<nn_header_t *>(reinterpret_cast<uint8_t*>(APP_OSPI_FLASH_HE_BASE));


void *get_network_description()
{
    return (void *)nn_header;
}

size_t get_description_length()
{
    return nn_header->size;
}

namespace arm {
namespace app {
namespace kws {

const uint8_t * GetModelPointer()
{
    return nn_header->models[0].nn_model_data;
}

size_t GetModelLen()
{
    return nn_header->models[0].nn_model_size;
}

} /* namespace arm */
} /* namespace app */
} 
#endif