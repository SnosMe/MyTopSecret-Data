#include "mtsdata.h"
#include "private.h"
#include <stddef.h>
#include <string.h>
#include <time.h>

#pragma pack(push, 1)
typedef struct {
  uint16_t crc16;
  uint16_t date;
  uint8_t random_bytes[MTSD_RANDOM_BYTES];
} mtsd_header;
#pragma pack(pop)

static uint16_t crc16(uint8_t* data, size_t size);
static uint16_t get_date_now ();

mtsd_res mtsd_encrypt(mtsd_document* doc,
                      uint8_t* password,
                      size_t password_len,
                      uint8_t** encrypted,
                      size_t* size) {
  uint8_t* out = NULL;
  MTSD_MALLOC(out, sizeof(mtsd_header) + MTSD_PAYLOAD_MAX_SIZE);

  size_t encoded_size = MTSD_PAYLOAD_MAX_SIZE;
  MTSD_CHECK_GOTO(mtsd_encode(doc, out + sizeof(mtsd_header), &encoded_size), error);

  *size = encoded_size;
  MTSD_CHECK_GOTO(mtsd_compress_payload(out + sizeof(mtsd_header), size), error);
  memset(out + sizeof(mtsd_header) + *size, 0x00, encoded_size - *size);

  MTSD_CHECK_GOTO(mtsd_encrypt_payload(out + sizeof(mtsd_header), *size, password, password_len, ((mtsd_header*)out)->random_bytes), error);

  ((mtsd_header*)out)->date = get_date_now();
  ((mtsd_header*)out)->crc16 = crc16(out + 2, sizeof(mtsd_header) + *size - 2);

  *encrypted = (uint8_t*)out;
  *size = (sizeof(mtsd_header) + *size);
  return MTSD_OK;

error:
  MTSD_FREE(out);
  return MTSD_ERR;
}

mtsd_res mtsd_decrypt(uint8_t* encrypted,
                      size_t size,
                      uint8_t* password,
                      size_t password_len,
                      mtsd_document *doc) {
  uint8_t* cloned = NULL;
  uint8_t* encoded = NULL;

  MTSD_MALLOC(cloned, size - sizeof(mtsd_header));
  memcpy(cloned, encrypted + sizeof(mtsd_header), size - sizeof(mtsd_header));

  MTSD_CHECK_GOTO(mtsd_decrypt_payload(cloned, size - sizeof(mtsd_header), password, password_len, ((mtsd_header*)encrypted)->random_bytes), error);

  encoded = mtsd_malloc(MTSD_PAYLOAD_MAX_SIZE);
  if (!encoded) {
    mtsd_error(MTSD_ESELF, MTSD_EMEMORY, NULL);
    goto error;
  }
  size_t encoded_size = MTSD_PAYLOAD_MAX_SIZE;
  MTSD_CHECK_GOTO(mtsd_decompress_payload(cloned, size - sizeof(mtsd_header), encoded, &encoded_size), error);
  MTSD_FREE(cloned);

  MTSD_CHECK_GOTO(mtsd_decode(encoded, encoded_size, doc), error);
  MTSD_FREE(encoded);

  return MTSD_OK;

error:
  MTSD_FREE(cloned);
  MTSD_FREE(encoded);
  return MTSD_ERR;
}

/*
  Name  : CRC-16/CCITT-FALSE
  Poly  : 0x1021
  Init  : 0xFFFF
  RefIn : false
  RefOut: false
  XorOut: 0x0000
  Check : 0x29B1
*/
static uint16_t crc16 (uint8_t* data, size_t size) {
  uint16_t crc = 0xFFFF;

  while(size--)
  {
    crc ^= *(data++) << 8;

    for (unsigned i = 0; i < 8; i += 1)
      crc = (crc & 0x8000)
        ? (crc << 1) ^ 0x1021
        : crc << 1;
  }

  return crc;
}

static uint16_t get_date_now () {
  time_t t = time(NULL);
  struct tm* local = localtime(&t);
  uint16_t delta_y = local->tm_year + 1900 - MTSD_DATE_FROM;
  uint16_t delta_d = (delta_y * 366) + (local->tm_yday + 1);
  return delta_d;
}
