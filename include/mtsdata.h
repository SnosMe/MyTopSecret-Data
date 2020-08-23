#ifndef _LIB_MTSD_H_
#define _LIB_MTSD_H_

#include <stdint.h>
#include <stddef.h>

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
  MTSD_EDECODE_CORRUPTED_PAYLOAD,
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

extern void mtsd_error(mtsd_error_source src, int error, char *msg);
extern void* mtsd_malloc(size_t size);
extern void* mtsd_realloc(void* ptr, size_t size);
extern void mtsd_free(void* ptr);

mtsd_res mtsd_parse(/* In */ mtsd_read_callback read_callback,
                    /* In */ void *callback_data,
                    /* Out */ mtsd_document *doc);

mtsd_res mtsd_encrypt(/* In */ mtsd_document* doc,
                      /* In */ uint8_t* password,
                      /* In */ size_t password_len,
                      /* Out */ uint8_t** encrypted,
                      /* Out */ size_t* size);

mtsd_res mtsd_decrypt(/* In */ uint8_t* encrypted,
                      /* In */ size_t size,
                      /* In */ uint8_t* password,
                      /* In */ size_t password_len,
                      /* Out */ mtsd_document *doc);

mtsd_res mtsd_to_text(/* In */ mtsd_document* doc,
                      /* Out */ uint8_t** out,
                      /* Out */ size_t* size);

size_t mtsd_doc_records_count(const mtsd_document* doc);
size_t mtsd_doc_record_fields_count(const mtsd_record* record);
int mtsd_doc_field_is_multiline_value(const mtsd_field* field);
void mtsd_doc_init(mtsd_document* doc);
void mtsd_doc_record_init(mtsd_record* record);
void mtsd_doc_field_init(mtsd_field* field);
void mtsd_doc_free(mtsd_document* doc);

#endif
