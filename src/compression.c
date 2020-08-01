#include "mtsdata.h"

#include <stdlib.h>
#include <LzmaEnc.h>
#include <LzmaDec.h>

#define MTS_COMPRESSION_HEADER_SIZE 3

static void* SzAlloc(ISzAllocPtr p, size_t size) {
  return malloc(size);
}
static void SzFree(ISzAllocPtr p, void* address) {
  free(address);
}
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

int compress_data(uint8_t* data, size_t data_size,
                  uint8_t** compressed, size_t* compressed_size) {
  CLzmaEncProps props;
  LzmaEncProps_Init(&props);
  props.lc = 0;
  props.lp = 0;
  props.pb = 0;

  props.level = 9;
  props.dictSize = data_size;

  uint8_t props_encoded[LZMA_PROPS_SIZE];
  size_t props_encoded_size = LZMA_PROPS_SIZE;
  *compressed_size = data_size + 1;
  *compressed = (uint8_t*)malloc(*compressed_size);

  SRes res = LzmaEncode(
    *compressed, compressed_size,
    data, data_size,
    &props, props_encoded, &props_encoded_size, 0, NULL, &g_Alloc, &g_Alloc);

  if (res != SZ_OK) {
    return -1;
  }
  else if (*compressed_size > data_size) {
    return -2;
  }
  return 0;
}

int decompress_data(uint8_t* compressed, size_t compressed_size,
                    uint8_t** data, size_t data_size) {
  CLzmaEncProps props;
  props.lc = 0;
  props.lp = 0;
  props.pb = 0;
  props.dictSize = data_size;

  uint8_t props_encoded[LZMA_PROPS_SIZE];
  props_encoded[0] = (uint8_t)((props.pb * 5 + props.lp) * 9 + props.lc);
  *(uint32_t*)(props_encoded + 1) = props.dictSize;

  *data = malloc(data_size);
  ELzmaStatus status;

  SRes res = LzmaDecode(*data, &data_size, compressed, &compressed_size,
    props_encoded, LZMA_PROPS_SIZE, LZMA_FINISH_ANY, &status, &g_Alloc);

  if (res != SZ_OK) {
    return -1;
  }
  return 0;
}
