#ifndef MTS_MTSDATA_H
#define MTS_MTSDATA_H

#include <stdint.h>
#include <stddef.h>

#define MTSD_RANDOM_BYTES 16
#define MTSD_TIMESTAMP_SIZE 2
#define MTSD_CRC_SIZE 2
#define MTSD_HEADER_SIZE (MTSD_RANDOM_BYTES + MTSD_TIMESTAMP_SIZE + MTSD_CRC_SIZE)
#define MTSD_PAYLOAD_MAX_SIZE 0xFFFF

#define MTSD_CHECK(result)    if ((result) != MTSD_OK) { return MTSD_ERR; }

typedef enum {
  MTSD_ERR = 0, // if (!ok) { handle... }
  MTSD_OK  = 1, // if (ok) { continue... }
} mtsd_res;

typedef enum {
  MTSD_ESELF,
  MTSD_EARGON2,
  MTSD_ELZMA,
  MTSD_ERANDOMBYTES,
} mtsd_error_source;

typedef enum {
  MTSD_EMEMORY,
  MTSD_EREADER,
  MTSD_EINCOMPRESSIBLE_DATA,
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

typedef int (*mtsd_read_callback)(void *data,
                                  uint8_t *buffer,
                                  size_t size,
                                  size_t *size_read);

mtsd_res mtsd_parse(/* In */ mtsd_read_callback read_callback,
                    /* In */ void *callback_data,
                    /* Out */ mtsd_document *doc);

mtsd_res mtsd_encrypt(/* In */ mtsd_document* doc,
                      /* In */ uint8_t* password,
                      /* In */ size_t password_len,
                      /* Out */ uint8_t** encrypted,
                      /* Out */ size_t* size);

mtsd_res mtsd_decrypt(/* In */ uint8_t* encrypted,
                      /* In */ size_t* size,
                      /* In */ uint8_t* password,
                      /* In */ size_t password_len,
                      /* Out */ mtsd_document *doc);

#endif
