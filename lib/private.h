#include "mtsdata.h"

#define MTSD_RANDOM_BYTES 16
#define MTSD_PAYLOAD_MAX_SIZE 0xFFFF

#define MTSD_CHECK(result)        if ((result) != MTSD_OK) {\
                                    return MTSD_ERR;\
                                  }

#define MTSD_CHECK_GOTO(result, label)    if ((result) != MTSD_OK) {\
                                            goto label;\
                                          }

#define MTSD_MALLOC(var, size)    (var) = mtsd_malloc((size));\
                                  if (!(var)) {\
                                    mtsd_error(MTSD_ESELF, MTSD_EMEMORY, NULL);\
                                    return MTSD_ERR;\
                                  }

#define MTSD_REALLOC(var, size)   (var) = mtsd_realloc((var), (size));\
                                  if (!(var)) {\
                                    mtsd_error(MTSD_ESELF, MTSD_EMEMORY, NULL);\
                                    return MTSD_ERR;\
                                  }

#define MTSD_FREE(var)            mtsd_free((var));\
                                  (var) = NULL;

mtsd_res mtsd_encode(/* In */ mtsd_document* doc,
                     /* In Out */ uint8_t* out,
                     /* In Out */ size_t* size);

mtsd_res mtsd_decode(/* In */ uint8_t* data,
                     /* In */ size_t size,
                     /* In Out */ mtsd_document* doc);

mtsd_res mtsd_compress_payload(/* In Out */ uint8_t* data,
                               /* In Out */ size_t* size);

mtsd_res mtsd_decompress_payload(/* In */ uint8_t* compressed,
                                 /* In */ size_t compressed_size,
                                 /* Out */ uint8_t* data,
                                 /* In Out */ size_t* size);

mtsd_res mtsd_encrypt_payload(/* In Out */ uint8_t* data,
                              /* In */ size_t data_size,
                              /* In */ uint8_t* pwd,
                              /* In */ size_t pwd_size,
                              /* Out */ uint8_t* random_bytes);

mtsd_res mtsd_decrypt_payload(/* In Out */ uint8_t* data,
                              /* In */ size_t data_size,
                              /* In */ uint8_t* pwd,
                              /* In */ size_t pwd_size,
                              /* In */ uint8_t* random_bytes);
