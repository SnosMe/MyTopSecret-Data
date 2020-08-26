#include "mtsdata.h"

#define MTSD_RANDOM_BYTES     4
#define MTSD_SALT_SIZE        16
#define MTSD_PAYLOAD_MAX_SIZE 0xFFFF
#define MTSD_DATE_FROM        915148800 // 01/01/1999
#define MTSD_LZMA_LC          0
#define MTSD_LZMA_LP          0
#define MTSD_LZMA_PB          0
#define MTSD_ARGON2_ITER      2
#define MTSD_ARGON2_MEM       (1 << 19)
#define MTSD_ARGON2_THREADS   4

#define MTSD_CHECK(result)                                                                                             \
  if ((result) != MTSD_OK) {                                                                                           \
    return MTSD_ERR;                                                                                                   \
  }

#define MTSD_CHECK_GOTO(label, result)                                                                                 \
  if ((result) != MTSD_OK) {                                                                                           \
    goto label;                                                                                                        \
  }

#define MTSD_MALLOC(var, size)                                                                                         \
  (var) = mtsd_malloc((size));                                                                                         \
  if (!(var)) {                                                                                                        \
    mtsd_error(MTSD_ESELF, MTSD_EMEMORY, NULL);                                                                        \
    return MTSD_ERR;                                                                                                   \
  }

#define MTSD_REALLOC(var, size)                                                                                        \
  (var) = mtsd_realloc((var), (size));                                                                                 \
  if (!(var)) {                                                                                                        \
    mtsd_error(MTSD_ESELF, MTSD_EMEMORY, NULL);                                                                        \
    return MTSD_ERR;                                                                                                   \
  }

#define MTSD_FREE(var)                                                                                                 \
  mtsd_free((var));                                                                                                    \
  (var) = NULL;

mtsd_res mtsd_encode(/* In */ mtsd_document* doc,
                     /* Out */ uint8_t** out,
                     /* Out */ size_t* size);

mtsd_res mtsd_decode(/* In */ uint8_t* data,
                     /* In */ size_t size,
                     /* In Out */ mtsd_document* doc);

mtsd_res mtsd_compress_payload(/* In Out */ uint8_t* data,
                               /* In Out */ size_t* size,
                               /* Out */ uint8_t* is_compressed);

mtsd_res mtsd_decompress_payload(/* In */ uint8_t* compressed,
                                 /* In */ size_t compressed_size,
                                 /* Out */ uint8_t* data,
                                 /* In Out */ size_t* size);

mtsd_res mtsd_encrypt_payload(/* In Out */ uint8_t* data,
                              /* In */ size_t data_size,
                              /* In */ uint8_t* pwd,
                              /* In */ size_t pwd_size,
                              /* In */ uint8_t* salt);

mtsd_res mtsd_decrypt_payload(/* In Out */ uint8_t* data,
                              /* In */ size_t data_size,
                              /* In */ uint8_t* pwd,
                              /* In */ size_t pwd_size,
                              /* In */ uint8_t* salt);

mtsd_res mtsd_random_bytes(uint8_t* out, size_t size);
