#include "mtsdata.h"

#define MTSD_CHECK(result)        if ((result) != MTSD_OK) {\
                                    return MTSD_ERR;\
                                  }

#define MTSD_MALLOC(var, size)    (var) = malloc((size));\
                                  if (!(var)) {\
                                    mtsd_error(MTSD_ESELF, MTSD_EMEMORY);\
                                    return MTSD_ERR;\
                                  }

#define MTSD_REALLOC(var, size)   (var) = realloc((var), (size));\
                                  if (!(var)) {\
                                    mtsd_error(MTSD_ESELF, MTSD_EMEMORY);\
                                    return MTSD_ERR;\
                                  }

#define MTSD_FREE(var)            free((var));

void mtsd_error(mtsd_error_source src, int error);
void mtsd_error_msg(mtsd_error_source src, int error, char *msg);

mtsd_res mtsd_encode(/* In */ mtsd_document* doc,
                     /* In Out */ uint8_t* out,
                     /* In Out */ size_t* size);

mtsd_res mtsd_decode(/* In */ uint8_t* data,
                     /* In */ size_t size,
                     /* In Out */ mtsd_document* doc);

mtsd_res compress_data(/* In Out */ uint8_t* data,
                       /* In Out */ size_t* size);

mtsd_res decompress_data(/* In */ uint8_t* compressed,
                         /* In */ size_t compressed_size,
                         /* Out */ uint8_t* data,
                         /* In Out */ size_t* size);

mtsd_res encrypt(/* In Out */ uint8_t* data,
                 /* In */ size_t data_size,
                 /* In */ uint8_t* pwd,
                 /* In */ size_t pwd_size,
                 /* Out */ uint8_t* random_bytes);

mtsd_res decrypt(/* In Out */ uint8_t* data,
                 /* In */ size_t data_size,
                 /* In */ uint8_t* pwd,
                 /* In */ size_t pwd_size,
                 /* In */ uint8_t* random_bytes);
