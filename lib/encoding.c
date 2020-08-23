#include "mtsdata.h"
#include "private.h"
#include "lang/parser.h"
#include <string.h>

#define RECORD_SIZE 1
#define KEY_SIZE 1
#define STR_TERMINATOR_SIZE 1
#define STR_TERMINATOR '\0'

mtsd_res mtsd_encode(mtsd_document* doc, uint8_t* out, size_t* size) {
  size_t written = 0;

  mtsd_record* record = doc->records;
  while (record) {
    size_t fields_num = mtsd_doc_record_fields_count(record);
    if (fields_num > 0xFF) {
      mtsd_error(MTSD_ESELF, MTSD_EENCODE_RECORD_SIZE, NULL);
      return MTSD_ERR;
    }

    if (fields_num) {
      if ((written + RECORD_SIZE) > *size) {
        mtsd_error(MTSD_ESELF, MTSD_EENCODE_PAYLOAD_SIZE, NULL);
        return MTSD_ERR;
      }
      out[written] = fields_num;
      written += RECORD_SIZE;
    }
    mtsd_field* field = record->fields;
    while (field) {
      if ((written + KEY_SIZE + field->value_size + STR_TERMINATOR_SIZE) > *size) {
        mtsd_error(MTSD_ESELF, MTSD_EENCODE_PAYLOAD_SIZE, NULL);
        return MTSD_ERR;
      }
      out[written] = field->key;
      written += KEY_SIZE;
      memcpy(out + written, field->value, field->value_size);
      written += field->value_size;
      out[written] = STR_TERMINATOR;
      written += STR_TERMINATOR_SIZE;

      field = field->next;
    }

    record = record->next;
  }

  *size = written;
  return MTSD_OK;
}

static inline mtsd_res mtsd_decode__(uint8_t* data, size_t size, mtsd_document* doc) {
  for (size_t i = 0; i < size;) {
    mtsd_record *record = doc->records;
    if (!record) {
      MTSD_MALLOC(record, sizeof(mtsd_record));
      doc->records = record;
    } else {
      while (record->next) {
        record = record->next;
      }
      MTSD_MALLOC(record->next, sizeof(mtsd_record));
      record = record->next;
    }
    mtsd_doc_record_init(record);

    uint8_t fields_num = data[i];
    i += RECORD_SIZE;

    for (uint8_t field_i = 0; field_i < fields_num; field_i += 1, i += 1) {
      if (i >= size) {
        mtsd_error(MTSD_ESELF, MTSD_EDECODE_CORRUPTED_PAYLOAD, NULL);
        return MTSD_ERR;
      }

      mtsd_field *field = record->fields;
      if (!field) {
        MTSD_MALLOC(field, sizeof(mtsd_field));
        record->fields = field;
      } else {
        while (field->next) {
          field = field->next;
        }
        MTSD_MALLOC(field->next, sizeof(mtsd_field));
        field = field->next;
      }
      mtsd_doc_field_init(field);

      field->key = data[i];
      i += KEY_SIZE;
      if (!mtsd_doc_is_valid_keyid(field->key)) {
        mtsd_error(MTSD_ESELF, MTSD_EDECODE_CORRUPTED_PAYLOAD, "invalid keyid");
      }

      for (; i < size; i += 1) {
        if (data[i] == STR_TERMINATOR) {
          if (field->value_size) {
            MTSD_MALLOC(field->value, field->value_size);
            memcpy(field->value, data + i - field->value_size, field->value_size);
          }
          break;
        } else {
          field->value_size += 1;
        }
      }
      if (data[i] != STR_TERMINATOR) {
        mtsd_error(MTSD_ESELF, MTSD_EDECODE_CORRUPTED_PAYLOAD, NULL);
        return MTSD_ERR;
      }
    }
  }

  return MTSD_OK;
}

mtsd_res mtsd_decode(uint8_t* data, size_t size, mtsd_document* doc) {
  mtsd_doc_init(doc);
  if (!mtsd_decode__(data, size, doc)) {
    mtsd_doc_free(doc);    
    return MTSD_ERR;
  }
  return MTSD_OK;
}

static inline mtsd_res mtsd_to_text__(mtsd_document* doc, uint8_t** out, size_t* size) {
  #define buf (*out)
  size_t written = 0;
  buf = NULL;

  mtsd_record* record = doc->records;
  while (record) {
    mtsd_field* field = record->fields;
    while (field) {
      char* key = mtsd_doc_keyid_to_string(field->key);
      size_t key_len = strlen(key);
      MTSD_REALLOC(buf, written + key_len + 2);
      memcpy(buf + written, key, key_len);
      written += key_len;

      int is_multiline = mtsd_doc_field_is_multiline_value(field);

      buf[written] = ':';
      written += 1;
      buf[written] = (is_multiline) ? '\n' : ' ';
      written += 1;

      if (is_multiline) {
        size_t value_offset = 0;
        size_t value_length = 0;
        for (;;) {
          while (value_offset < field->value_size) {
            value_offset += 1;
            value_length += 1;
            if (field->value[value_offset] == '\n') {
              value_offset += 1;
              value_length += 1;
              break;
            }
          }

          MTSD_REALLOC(buf, written + value_length + 2);
          buf[written] = ' ', written += 1;
          buf[written] = ' ', written += 1;
          memcpy(buf + written, field->value + value_offset - value_length, value_length);
          written += value_length;

          if (value_offset >= field->value_size) {
            break;
          } else {
            value_length = 0;
          }
        }
      } else {
        MTSD_REALLOC(buf, written + field->value_size);
        memcpy(buf + written, field->value, field->value_size);
        written += field->value_size;
      }

      MTSD_REALLOC(buf, written + 1);
      buf[written] = '\n';
      written += 1;

      field = field->next;
    }

    if (record->next) {
      MTSD_REALLOC(buf, written + 4);
      buf[written + 0] = '-';
      buf[written + 1] = '-';
      buf[written + 2] = '-';
      buf[written + 3] = '\n';
      written += 4;
    }

    record = record->next;
  }
  #undef buf

  *size = written;
  return MTSD_OK;
}

mtsd_res mtsd_to_text(mtsd_document* doc, uint8_t** out, size_t* size) {
  *out = NULL;
  if (!mtsd_to_text__(doc, out, size)) {
    MTSD_FREE(*out);
    return MTSD_ERR;
  }
  return MTSD_OK;  
}
