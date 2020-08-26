#include "private.h"

#include <LzmaDec.h>
#include <LzmaEnc.h>

static void* SzAlloc(ISzAllocPtr p, size_t size)
{
  return mtsd_malloc(size);
}
static void SzFree(ISzAllocPtr p, void* address)
{
  mtsd_free(address);
}
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

mtsd_res mtsd_compress_payload(uint8_t* data, size_t* size, uint8_t* is_compressed)
{
  size_t data_size = *size;
  CLzmaEncProps props;
  LzmaEncProps_Init(&props);
  props.lc = MTSD_LZMA_LC;
  props.lp = MTSD_LZMA_LP;
  props.pb = MTSD_LZMA_PB;

  props.level = 9;
  props.dictSize = data_size;

  uint8_t props_encoded[LZMA_PROPS_SIZE];
  size_t props_encoded_size = LZMA_PROPS_SIZE;

  SRes res = LzmaEncode(
    data, size,
    data, data_size,
    &props, props_encoded, &props_encoded_size, 0, NULL, &g_Alloc, &g_Alloc);

  if (res != SZ_OK) {
    if (res == SZ_ERROR_OUTPUT_EOF) {
      *is_compressed = 0;
      return MTSD_OK;
    } else {
      mtsd_error(MTSD_ELZMA, res, NULL);
      return MTSD_ERR;
    }
  }
  *is_compressed = 1;
  return MTSD_OK;
}

mtsd_res mtsd_decompress_payload(uint8_t* compressed, size_t compressed_size, uint8_t* data, size_t* size)
{
  CLzmaEncProps props;
  props.lc = MTSD_LZMA_LC;
  props.lp = MTSD_LZMA_LP;
  props.pb = MTSD_LZMA_PB;
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
