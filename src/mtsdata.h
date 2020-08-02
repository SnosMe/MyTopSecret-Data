#ifndef MTS_MTSDATA_H
#define MTS_MTSDATA_H

#include <inttypes.h>
#include <stddef.h>

#define MTSD_RANDOM_BYTES 16
#define MTSD_TIMESTAMP_SIZE 2
#define MTSD_CRC_SIZE 2
#define MTSD_HEADER_SIZE (MTSD_RANDOM_BYTES + MTSD_TIMESTAMP_SIZE + MTSD_CRC_SIZE)
#define MTSD_PAYLOAD_MAX_SIZE 0xFFFF

typedef enum MTSData_Result {
  MTSD_OK = 0,
  MTSD_ERR = 1
} mtsd_res;

typedef enum MTSData_Error_Source {
  MTSD_ESELF = 0,
  MTSD_EARGON2 = 1,
  MTSD_ERANDOMBYTES = 2,
} mtsd_error_source;

typedef enum MTSData_Error {
  MTSD_EMEMORY = 0,
} mtsd_err;

void mtsd_error(mtsd_error_source src, int error);

int compress_data(uint8_t* data, size_t data_size, size_t* compressed_size);

int decompress_data(uint8_t* compressed, size_t compressed_size,
                    uint8_t** data, size_t data_size);

mtsd_res encrypt(uint8_t* data, size_t data_size,
                 uint8_t* pwd, size_t pwd_size, uint8_t* random_bytes);

#endif
