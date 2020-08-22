#include "mtsdata.h"
#include "private.h"
#include "lang/parser.h"
#include <string.h>
#include <stdlib.h>

#define RECORD_SIZE 1
#define KEY_SIZE 1
#define STR_TERMINATOR_SIZE 1
#define STR_TERMINATOR '\0'

mtsd_res mtsd_encode(mtsd_document* doc, uint8_t* out, size_t* size) {
  size_t written = 0;

  mtsd_record* record = doc->records;
  while (record != NULL) {
    uint8_t fields_num = 0;
    mtsd_field* field = record->fields;
    while (field != NULL) {
      if (fields_num == 0xFF) {
        mtsd_error(MTSD_ESELF, MTSD_EENCODE_RECORD_SIZE);
        return MTSD_ERR;
      }
      fields_num += 1;
      field = field->next;
    }

    if (fields_num) {
      if ((written + RECORD_SIZE) > *size) {
        mtsd_error(MTSD_ESELF, MTSD_EENCODE_PAYLOAD_SIZE);
        return MTSD_ERR;
      }
      out[written] = fields_num;
      written += RECORD_SIZE;
    }
    field = record->fields;
    while (field != NULL) {
      if ((written + KEY_SIZE + field->value_size + STR_TERMINATOR_SIZE) > *size) {
        mtsd_error(MTSD_ESELF, MTSD_EENCODE_PAYLOAD_SIZE);
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

mtsd_res mtsd_decode(uint8_t* data, size_t size, mtsd_document* doc) {
  doc->records = NULL;

  for (size_t i = 0; i < size;) {
    mtsd_record *record = doc->records;
    if (!record) {
      record = malloc(sizeof(mtsd_record));
      if (!record) {
        mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
        return MTSD_ERR;
      }
      doc->records = record;
    } else {
      while (record->next != NULL) {
        record = record->next;
      }
      record->next = malloc(sizeof(mtsd_record));
      if (!record->next) {
        mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
        return MTSD_ERR;
      }
      record = record->next;
    }
    record->next = NULL;
    record->fields = NULL;

    uint8_t fields_num = data[i];
    i += RECORD_SIZE;

    for (uint8_t field_i = 0; field_i < fields_num; field_i += 1, i += 1) {
      if (i == size) {
        mtsd_error(MTSD_ESELF, MTSD_EDECODE_CORRUPTED_PAYLOAD);
        return MTSD_ERR;
      }

      mtsd_field *field = record->fields;
      if (!field) {
        field = malloc(sizeof(mtsd_field));
        if (!field) {
          mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
          return MTSD_ERR;
        }
        record->fields = field;
      } else {
        while (field->next != NULL) {
          field = field->next;
        }
        field->next = malloc(sizeof(mtsd_field));
        if (!field) {
          mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
          return MTSD_ERR;
        }
        field = field->next;
      }
      field->next = NULL;
      field->key = 0;
      field->value = NULL;
      field->value_size = 0;

      field->key = data[i];
      i += KEY_SIZE;

      for (; i < size; i += 1) {
        if (data[i] == STR_TERMINATOR) {
          if (field->value_size) {
            field->value = malloc(field->value_size);
            if (!field->value) {
              mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
              return MTSD_ERR;
            }
            memcpy(field->value, data + i - field->value_size, field->value_size);
          }
          break;
        } else {
          field->value_size += 1;
        }
      }
      if (data[i] != STR_TERMINATOR) {
        mtsd_error(MTSD_ESELF, MTSD_EDECODE_CORRUPTED_PAYLOAD);
        return MTSD_ERR;
      }
    }
  }

  return MTSD_OK;
}

mtsd_res mtsd_to_text(mtsd_document* doc, uint8_t** out, size_t* size) {
  size_t written = 0;
  uint8_t* buf = NULL;

  mtsd_record* record = doc->records;
  while (record != NULL) {
    mtsd_field* field = record->fields;
    while (field != NULL) {
      char* key = "my_key";
      size_t key_len = 6;
      buf = realloc(buf, written + key_len + 2);
      memcpy(buf + written, key, key_len);
      written += key_len;
      buf[written] = ':';
      written += 1;

      int is_multiline = 0;
      for (size_t i = 0; i < field->value_size; i += 1) {
        if (field->value[i] == '\n') {
          is_multiline = 1;
          break;
        }
      }
      if (is_multiline) {
        buf[written] = '\n';
        written += 1;
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

          buf = realloc(buf, written + value_length + 2);
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
        buf[written] = ' ';
        written += 1;
        buf = realloc(buf, written + field->value_size);
        memcpy(buf + written, field->value, field->value_size);
        written += field->value_size;
      }

      buf = realloc(buf, written + 1);
      buf[written] = '\n';
      written += 1;

      field = field->next;
    }

    if (record->next) {
      buf = realloc(buf, written + 4);
      buf[written] = '-';
      buf[written + 1] = '-';
      buf[written + 2] = '-';
      buf[written + 3] = '\n';
      written += 4;
    }

    record = record->next;
  }

  *size = written;
  *out = buf;
  return MTSD_OK;
}
