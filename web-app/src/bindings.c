#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <mtsdata.h>

void mtsd_error(mtsd_error_source src, int error, char* msg) {
  // TODO
}
void* mtsd_malloc(size_t size) {
  return malloc(size);
}
void* mtsd_realloc(void* ptr, size_t size) {
  return realloc(ptr, size);
}
void mtsd_free(void* ptr) {
  free(ptr);
}

int encode(mtsd_document* doc, uint8_t** out);

uint8_t* decrypt(uint8_t* encrypted_data, size_t encrypted_size,
                 uint8_t* password, size_t pass_len) {
  mtsd_document doc;
  if (!mtsd_decrypt(encrypted_data, encrypted_size, password, pass_len, &doc)) {
    return NULL;
  }

  uint8_t* bin = NULL;
  if (!encode(&doc, &bin)) {
    mtsd_doc_free(&doc);
    return NULL;
  }
  mtsd_doc_free(&doc);

  return bin;
}

int encode(mtsd_document* doc, uint8_t** out)
{
  size_t size = 0;
  mtsd_record* record = doc->records;
  size += sizeof(size_t); // records_count
  while (record) {
    mtsd_field* field = record->fields;
    size += sizeof(size_t); // fields_count
    while (field) {
      size += sizeof(field->key);
      size += sizeof(field->value_size);
      size += field->value_size;
      field = field->next;
    }
    record = record->next;
  }

  *out = malloc(size);
  if (!(*out))
    return 0;

  uint8_t* offset = *out;

  {
    size_t records_count = mtsd_doc_records_count(doc);
    memcpy(offset, &records_count, sizeof(records_count));
    offset += sizeof(records_count);
  }

  record = doc->records;
  while (record) {
    {
      size_t fields_count = mtsd_doc_record_fields_count(record);
      memcpy(offset, &fields_count, sizeof(fields_count));
      offset += sizeof(fields_count);
    }

    mtsd_field* field = record->fields;
    while (field) {
      memcpy(offset, &field->key, sizeof(field->key));
      offset += sizeof(field->key);
      memcpy(offset, &field->value_size, sizeof(field->value_size));
      offset += sizeof(field->value_size);
      memcpy(offset, field->value, field->value_size);
      offset += field->value_size;

      field = field->next;
    }
    record = record->next;
  }

  return 1;
}
