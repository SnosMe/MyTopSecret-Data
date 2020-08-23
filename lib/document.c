#include "private.h"

size_t mtsd_doc_records_count(const mtsd_document* doc) {
  size_t records_num = 0;
  mtsd_record* record = doc->records;
  while (record) {
    records_num += 1;
    record = record->next;
  }
  return records_num;
}

size_t mtsd_doc_record_fields_count(const mtsd_record* record) {
  size_t fields_num = 0;
  mtsd_field* field = record->fields;
  while (field) {
    fields_num += 1;
    field = field->next;
  }
  return fields_num;
}

int mtsd_doc_field_is_multiline_value(const mtsd_field* field) {
  for (size_t i = 0; i < field->value_size; i += 1) {
    if (field->value[i] == '\n') {
      return 1;
    }
  }
  return 0;
}

void mtsd_doc_init(mtsd_document* doc) {
  doc->records = NULL;
}

void mtsd_doc_record_init(mtsd_record* record) {
  record->fields = NULL;
  record->next = NULL;
}

void mtsd_doc_field_init(mtsd_field* field) {
  field->key = 0;
  field->value_size = 0;
  field->value = NULL;
  field->next = NULL;
}

void mtsd_doc_free(mtsd_document* doc) {
  mtsd_record* record = doc->records;
  while (record) {
    mtsd_field* field = record->fields;
    while (field) {
      if (field->value) {
        MTSD_FREE(field->value);
      }

      mtsd_field* next = field->next;
      MTSD_FREE(field);
      field = next;
    }

    mtsd_record* next = record->next;
    MTSD_FREE(record);
    record = next;
  }

  mtsd_doc_init(doc);
}
