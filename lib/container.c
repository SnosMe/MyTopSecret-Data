#include "private.h"
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
  uint16_t date;
  uint16_t crc16;
  uint8_t random_bytes[MTSD_RANDOM_BYTES];
} mtsd_header;
#pragma pack(pop)

mtsd_res mtsd_encrypt(mtsd_document* doc,
                      uint8_t* password,
                      size_t password_len,
                      uint8_t** encrypted,
                      size_t* size) {
  uint8_t* out = malloc(sizeof(mtsd_header) + MTSD_PAYLOAD_MAX_SIZE);
  if (!out) {
    mtsd_error(MTSD_ESELF, MTSD_EMEMORY, NULL);
    return MTSD_ERR;
  }

  size_t encoded_size = MTSD_PAYLOAD_MAX_SIZE;
  MTSD_CHECK(mtsd_encode(doc, out + sizeof(mtsd_header), &encoded_size));

  *size = encoded_size;
  MTSD_CHECK(compress_data(out + sizeof(mtsd_header), size));
  memset(out + sizeof(mtsd_header) + *size, 0x00, encoded_size - *size);

  MTSD_CHECK(encrypt(out + sizeof(mtsd_header), *size, password, password_len, ((mtsd_header*)out)->random_bytes));

  *encrypted = (uint8_t*)out;
  *size = (sizeof(mtsd_header) + *size);
  return MTSD_OK;
}

mtsd_res mtsd_decrypt(uint8_t* encrypted,
                      size_t size,
                      uint8_t* password,
                      size_t password_len,
                      mtsd_document *doc) {
  uint8_t* cloned = malloc(size - sizeof(mtsd_header));
  memcpy(cloned, encrypted + sizeof(mtsd_header), size - sizeof(mtsd_header));

  MTSD_CHECK(decrypt(cloned, size - sizeof(mtsd_header), password, password_len, ((mtsd_header*)encrypted)->random_bytes));

  size_t encoded_size = MTSD_PAYLOAD_MAX_SIZE;
  uint8_t* encoded = malloc(MTSD_PAYLOAD_MAX_SIZE);
  MTSD_CHECK(decompress_data(cloned, size - sizeof(mtsd_header), encoded, &encoded_size));

  MTSD_CHECK(mtsd_decode(encoded, encoded_size, doc));

  return MTSD_OK;
}
