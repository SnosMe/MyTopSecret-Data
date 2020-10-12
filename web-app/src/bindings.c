#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <mtsdata.h>
#include <dmtx.h>

static mtsd_error_source last_error_src = MTSD_ESELF;
static int last_error_code = 0;
static char* last_error_msg = NULL;

void mtsd_error(mtsd_error_source src, int error, char* msg) {
  last_error_src = src;
  last_error_code = error;
  last_error_msg = msg;
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

int b_mtsd_serialize_doc(mtsd_document* doc, uint8_t** out);

uint8_t* b_mtsd_decrypt(uint8_t* encrypted_data, size_t encrypted_size, uint8_t* password, size_t pass_len)
{
  mtsd_document doc;
  if (!mtsd_decrypt(encrypted_data, encrypted_size, password, pass_len, &doc)) {
    return NULL;
  }

  uint8_t* bin = NULL;
  if (!b_mtsd_serialize_doc(&doc, &bin)) {
    mtsd_doc_free(&doc);
    return NULL;
  }
  mtsd_doc_free(&doc);

  return bin;
}

int b_mtsd_serialize_doc(mtsd_document* doc, uint8_t** out)
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

#pragma pack(push, 1)
typedef struct {
  double pos_matrix[3][3];
  size_t data_size;
  uint8_t data[];
} b_dmtx_decoded_region;

typedef struct {
  size_t found;
  b_dmtx_decoded_region dmtx[];
} b_dmtx_decoded;
#pragma pack(pop)

uint8_t* b_dmtx_find_regions(uint8_t* img_data, size_t img_w, size_t img_h, int32_t budget) {
  size_t out_size = sizeof(b_dmtx_decoded);
  uint8_t* out = malloc(out_size);
  ((b_dmtx_decoded*)out)->found = 0;

  DmtxImage* img = dmtxImageCreate(img_data, img_w, img_h, DmtxPack32bppRGBX);
  dmtxImageSetProp(img, DmtxPropImageFlip, DmtxFlipNone);

  DmtxDecode* decode = dmtxDecodeCreate(img, 1);
  dmtxDecodeSetProp(decode, DmtxPropScanGap, 2);
  dmtxDecodeSetProp(decode, DmtxPropEdgeThresh, 5);

  DmtxTime timeout = dmtxTimeAdd(dmtxTimeNow(), budget);

  DmtxRegion* region;
  while ((region = dmtxRegionFindNext(decode, &timeout))) {
    ++((b_dmtx_decoded*)out)->found;

    DmtxMessage* msg = dmtxDecodeMatrixRegion(decode, region, DmtxUndefined);

    size_t alloc_size = out_size +
      sizeof(b_dmtx_decoded_region) + (msg != NULL ? msg->outputIdx : 0);
    out = realloc(out, alloc_size);
    b_dmtx_decoded_region* decoded_matrix = (b_dmtx_decoded_region*)(out + out_size);
    out_size = alloc_size;

    memcpy(decoded_matrix->pos_matrix, region->fit2raw, sizeof(region->fit2raw));

    if (msg) {
      decoded_matrix->data_size = msg->outputIdx;
      memcpy(decoded_matrix->data, msg->output, msg->outputIdx);
      dmtxMessageDestroy(&msg);
    } else {
      decoded_matrix->data_size = 0;
    }

    dmtxRegionDestroy(&region);
  }

  dmtxDecodeDestroy(&decode);
  dmtxImageDestroy(&img);

  if (((b_dmtx_decoded*)out)->found) {
    return out;
  } else {
    free(out);
    return NULL;
  }
}

uint8_t* b_dmtx_create(uint8_t* data, size_t data_size,
                       int module_size, int margin_size,
                       int* width_out, int* height_out) {
  DmtxEncode* encode = dmtxEncodeCreate();
  dmtxEncodeSetProp(encode, DmtxPropScheme, DmtxSchemeBase256);
  dmtxEncodeSetProp(encode, DmtxPropPixelPacking, DmtxPack32bppRGBX);
  dmtxEncodeSetProp(encode, DmtxPropImageFlip, DmtxFlipNone);
  dmtxEncodeSetProp(encode, DmtxPropModuleSize, module_size);
  dmtxEncodeSetProp(encode, DmtxPropMarginSize, margin_size);

  if (dmtxEncodeDataMatrix(encode, data_size, data) != DmtxPass) {
    dmtxEncodeDestroy(&encode);
    return NULL;
  }

  int width = dmtxImageGetProp(encode->image, DmtxPropWidth);
  int height = dmtxImageGetProp(encode->image, DmtxPropHeight);

  *width_out = width;
  *height_out = height;
  uint8_t* img_data = malloc(width * height * 4);
  memcpy(img_data, encode->image->pxl, (width * height * 4));

  dmtxEncodeDestroy(&encode);
  return img_data;
}

typedef struct {
  uint8_t* content;
  size_t size;
  size_t read;
} inmem_reader_state;

static int read_handler(void *data, uint8_t *buffer, size_t size, size_t *size_read) {
  inmem_reader_state *state = data;

  if (state->read == state->size) {
    *size_read = 0;
    return 1;
  }

  if (size > (state->size - state->read)) {
    size = state->size - state->read;
  }

  memcpy(buffer, state->content + state->read, size);
  state->read += size;
  *size_read = size;
  return 1;
}

uint8_t* b_mtsd_encrypt(uint8_t* text, size_t text_size,
                         uint8_t* password, size_t pass_len) {
  inmem_reader_state reader = { .read = 0 };
  reader.content = text;
  reader.size = text_size;

  mtsd_document doc;
  if (!mtsd_parse(read_handler, &reader, &doc)) {
    return NULL;
  }

  uint8_t* encrypted_data = NULL;
  size_t encrypted_size = 0;
  if (!mtsd_encrypt(&doc, password, pass_len, &encrypted_data, &encrypted_size)) {
    mtsd_doc_free(&doc);
    return NULL;
  }
  mtsd_doc_free(&doc);

  uint8_t* packed = malloc(sizeof(encrypted_size) + encrypted_size);
  memcpy(packed, &encrypted_size, sizeof(encrypted_size));
  memcpy(packed + sizeof(encrypted_size), encrypted_data, encrypted_size);
  mtsd_free(encrypted_data);

  return packed;
}

void b_last_error(int* src, int* code, char** msg, int* msg_len) {
  *src = last_error_src;
  *code = last_error_code;
  *msg = last_error_msg;
  *msg_len = strlen(last_error_msg);
}
