#ifndef MTS_MTSDATA_H
#define MTS_MTSDATA_H

#include <inttypes.h>
#include <stddef.h>

#define MTSD_RANDOM_BYTES 16
#define MTSD_TIMESTAMP_SIZE 2
#define MTSD_CRC_SIZE 2
#define MTSD_HEADER_SIZE (MTSD_RANDOM_BYTES + MTSD_TIMESTAMP_SIZE + MTSD_CRC_SIZE)
#define MTSD_PAYLOAD_MAX_SIZE 0xFFFF

#define MTSD_CHECK(result)    if ((result) != MTSD_OK) { return MTSD_ERR; }

typedef enum MTSData_Result {
  MTSD_ERR = 0,
  MTSD_OK = 1
} mtsd_res;

typedef enum MTSData_Error_Source {
  MTSD_ESELF,
  MTSD_EARGON2,
  MTSD_ERANDOMBYTES,
} mtsd_error_source;

typedef enum MTSData_Error {
  MTSD_EMEMORY,
  MTSD_EREADER,
  MTSD_EENCODE_RECORD_SIZE,
  MTSD_EENCODE_PAYLOAD_SIZE,
} mtsd_err;

typedef struct mtsd_field {
  uint8_t key;
  uint8_t *value;
  size_t value_size;
  struct mtsd_field *next;
} mtsd_field;

typedef struct mtsd_record {
  mtsd_field *fields;
  struct mtsd_record *next;
} mtsd_record;

typedef struct {
  mtsd_record* records;
} mtsd_document;

void mtsd_error(mtsd_error_source src, int error);
void mtsd_error_msg(mtsd_error_source src, int error, char *msg);

int compress_data(uint8_t* data, size_t data_size, size_t* compressed_size);

int decompress_data(uint8_t* compressed, size_t compressed_size,
                    uint8_t** data, size_t data_size);

mtsd_res encrypt(uint8_t* data, size_t data_size,
                 uint8_t* pwd, size_t pwd_size, uint8_t* random_bytes);

typedef int (*mtsd_read_callback)(void *data, uint8_t *buffer, size_t size, size_t *size_read);

mtsd_res parse(mtsd_read_callback read_callback, void *callback_data);

mtsd_res mtsd_encode(mtsd_document* doc, uint8_t* out, size_t* size);

#endif
