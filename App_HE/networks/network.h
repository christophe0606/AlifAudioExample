#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <stddef.h>

const uint8_t * GetModelPointer();
size_t GetModelLen();

#if defined(EXTERNAL_NETWORK)
void *get_network_description();
size_t get_description_length();
#endif

#endif