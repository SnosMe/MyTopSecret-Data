#ifndef MTS_MTSDATA_H
#define MTS_MTSDATA_H

#include <inttypes.h>
#include <stddef.h>

int compress_data(uint8_t* data, size_t data_size,
                  uint8_t** compressed, size_t* compressed_size);

int decompress_data(uint8_t* compressed, size_t compressed_size,
                    uint8_t** data, size_t data_size);

#endif
