#include "private.h"

#include <LzmaEnc.h>
#include <LzmaDec.h>

static void* SzAlloc(ISzAllocPtr p, size_t size) {
  return mtsd_malloc(size);
}
static void SzFree(ISzAllocPtr p, void* address) {
  mtsd_free(address);
}
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

mtsd_res mtsd_compress_payload(uint8_t* data, size_t* size) {
  size_t data_size = *size;
  CLzmaEncProps props;
  LzmaEncProps_Init(&props);
  props.lc = 0;
  props.lp = 0;
  props.pb = 0;

  props.level = 9;
  props.dictSize = data_size;

  uint8_t props_encoded[LZMA_PROPS_SIZE];
  size_t props_encoded_size = LZMA_PROPS_SIZE;

  SRes res = LzmaEncode(
    data, size,
    data, data_size,
    &props, props_encoded, &props_encoded_size, 0, NULL, &g_Alloc, &g_Alloc);

  if (res != SZ_OK) {
    mtsd_error(MTSD_ELZMA, res, NULL);
    return MTSD_ERR;
  }
  else if (*size == data_size) {
    mtsd_error(MTSD_ESELF, MTSD_EINCOMPRESSIBLE_DATA, NULL);
    return MTSD_ERR;
  }
  return MTSD_OK;
}

mtsd_res mtsd_decompress_payload(uint8_t* compressed, size_t compressed_size, uint8_t* data, size_t* size) {
  CLzmaEncProps props;
  props.lc = 0;
  props.lp = 0;
  props.pb = 0;
  props.dictSize = *size;

  uint8_t props_encoded[LZMA_PROPS_SIZE];
  props_encoded[0] = (uint8_t)((props.pb * 5 + props.lp) * 9 + props.lc);
  *(uint32_t*)(props_encoded + 1) = props.dictSize;

  ELzmaStatus status;
  SRes res = LzmaDecode(data, size, compressed, &compressed_size,
    props_encoded, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &status, &g_Alloc);

  if (res != SZ_OK && status != LZMA_STATUS_NEEDS_MORE_INPUT) {
    mtsd_error(MTSD_ELZMA, res, NULL);
    return MTSD_ERR;
  }
  return MTSD_OK;
}
