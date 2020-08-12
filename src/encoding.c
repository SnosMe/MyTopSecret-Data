#include "mtsdata.h"
#include "lang/parser.h"
#include <string.h>

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
      if ((written + RECORD_SIZE) > MTSD_PAYLOAD_MAX_SIZE) {
        mtsd_error(MTSD_ESELF, MTSD_EENCODE_PAYLOAD_SIZE);
        return MTSD_ERR;
      }
      out[written] = fields_num;
      written += RECORD_SIZE;
    }
    field = record->fields;
    while (field != NULL) {
      if ((written + KEY_SIZE + field->value_size + STR_TERMINATOR_SIZE) > MTSD_PAYLOAD_MAX_SIZE) {
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
