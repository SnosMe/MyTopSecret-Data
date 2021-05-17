#include "mtsdata.h"
#include "private.h"

#include <stddef.h>
#include <string.h>
#include <time.h>

#define MTSD_CRC_START        2
#define MTSD_DATE_SIZE        3
#define MTSD_DATE_RESOLUTION  3600

#pragma pack(push, 1)
typedef struct {
  uint16_t crc16;
  union {
    uint8_t is_compressed;
    uint8_t date[MTSD_DATE_SIZE];
  };
  uint8_t random_bytes[MTSD_RANDOM_BYTES];
} mtsd_header;
#pragma pack(pop)

static uint16_t crc16(uint8_t* data, size_t size);
static mtsd_res store_date_now(mtsd_header* header);
static int is_compressed_payload(mtsd_header* header);
static void set_is_compressed_payload(mtsd_header* header, int is_compressed);
static void derive_salt(const uint8_t* data, size_t data_size,
                        const uint8_t* pass, size_t pass_len, uint8_t* salt);

mtsd_res mtsd_encrypt(mtsd_document* doc, uint8_t* password, size_t password_len,
                      uint8_t** encrypted, size_t* size)
{
  uint8_t* out = NULL;
  uint8_t* encoded = NULL;

  MTSD_MALLOC(out, sizeof(mtsd_header) + MTSD_PAYLOAD_MAX_SIZE);

  size_t encoded_size = 0;
  MTSD_CHECK_GOTO(error,
    mtsd_encode(doc, &encoded, &encoded_size));
  if (encoded_size > MTSD_PAYLOAD_MAX_SIZE) {
    mtsd_error(MTSD_ESELF, MTSD_EENCODE_PAYLOAD_SIZE, NULL);
    goto error;
  }

  uint8_t is_compressed;
  memcpy(out + sizeof(mtsd_header), encoded, encoded_size);
  *size = encoded_size;
  MTSD_CHECK_GOTO(error,
    mtsd_compress_payload(out + sizeof(mtsd_header), size, &is_compressed));
  set_is_compressed_payload((mtsd_header*)out, is_compressed);
  if (!is_compressed) {
    memcpy(out + sizeof(mtsd_header), encoded, encoded_size);
    *size = encoded_size;
  }
  MTSD_FREE(encoded);

  MTSD_CHECK_GOTO(error,
    store_date_now((mtsd_header*)out));
  MTSD_CHECK_GOTO(error,
    mtsd_random_bytes(((mtsd_header*)out)->random_bytes, MTSD_RANDOM_BYTES));

  uint8_t salt[MTSD_SALT_SIZE];
  derive_salt(out, (sizeof(mtsd_header) + *size), password, password_len, salt);
  MTSD_CHECK_GOTO(error,
    mtsd_encrypt_payload(out + sizeof(mtsd_header), *size, password, password_len, salt));

  ((mtsd_header*)out)->crc16 = crc16(out + MTSD_CRC_START, sizeof(mtsd_header) + *size - MTSD_CRC_START);

  *encrypted = (uint8_t*)out;
  *size = (sizeof(mtsd_header) + *size);
  return MTSD_OK;

error:
  MTSD_FREE(out);
  MTSD_FREE(encoded);
  return MTSD_ERR;
}

mtsd_res mtsd_decrypt(uint8_t* encrypted, size_t size,
                      uint8_t* password, size_t password_len, mtsd_document* doc)
{
  uint8_t* cloned = NULL;
  uint8_t* encoded = NULL;

  MTSD_MALLOC(cloned, size - sizeof(mtsd_header));
  memcpy(cloned, encrypted + sizeof(mtsd_header), size - sizeof(mtsd_header));

  uint8_t salt[MTSD_SALT_SIZE];
  derive_salt(encrypted, size, password, password_len, salt);
  MTSD_CHECK_GOTO(error,
    mtsd_decrypt_payload(cloned, size - sizeof(mtsd_header), password, password_len, salt));

  if (is_compressed_payload((mtsd_header*)encrypted)) {
    encoded = mtsd_malloc(MTSD_PAYLOAD_MAX_SIZE);
    if (!encoded) {
      mtsd_error(MTSD_ESELF, MTSD_EMEMORY, NULL);
      goto error;
    }
    size_t encoded_size = MTSD_PAYLOAD_MAX_SIZE;
    MTSD_CHECK_GOTO(error,
      mtsd_decompress_payload(cloned, size - sizeof(mtsd_header), encoded, &encoded_size));
    MTSD_FREE(cloned);

    MTSD_CHECK_GOTO(error,
      mtsd_decode(encoded, encoded_size, doc));
    MTSD_FREE(encoded);
  } else {
    MTSD_CHECK_GOTO(error,
      mtsd_decode(cloned, size - sizeof(mtsd_header), doc));
  }

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
static uint16_t crc16(uint8_t* data, size_t size)
{
  uint16_t crc = 0xFFFF;

  while (size--) {
    crc ^= *(data++) << 8;

    for (unsigned i = 0; i < 8; i += 1)
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
  }

  return crc;
}

static mtsd_res store_date_now(mtsd_header* header)
{
  time_t t = time(NULL);
  if (t == (time_t)-1) {
    mtsd_error(MTSD_ESELF, MTSD_ETIME, "cannot get system time");
    return MTSD_ERR;
  }
  uint64_t scaled = ((uint64_t)t / MTSD_DATE_RESOLUTION);

  int is_compressed = is_compressed_payload(header);
  header->date[2] = (scaled >> (8 * 0)) & 0xFF;
  header->date[1] = (scaled >> (8 * 1)) & 0xFF;
  header->date[0] = (scaled >> (8 * 2)) & 0xFF;
  set_is_compressed_payload(header, is_compressed);

  return MTSD_OK;
}

static void derive_salt(const uint8_t* data, size_t data_size,
                        const uint8_t* pass, size_t pass_len, uint8_t* salt)
{
#ifndef DEBUG
  mtsd_header* header = (mtsd_header*)data;
  salt[0] = (data_size >> (8 * 0)) & 0xFF;
  salt[1] = (data_size >> (8 * 1)) & 0xFF;
  salt[2] = header->date[0];
  salt[3] = header->date[1];
  salt[4] = header->date[2];
  salt[5] = header->random_bytes[0];
  salt[6] = header->random_bytes[1];
  salt[7] = header->random_bytes[2];
  salt[8] = (pass_len >> (8 * 0)) & 0xFF;
  salt[9] = 0xc1;
  salt[10] = 0x12;
  salt[11] = 0x42;
  salt[12] = 0xf0;
  salt[13] = 0xe1;
  salt[14] = 0xeb;
  salt[15] = 0xa9;

  for (size_t pi = 0, si = 9; pi < pass_len && si < MTSD_SALT_SIZE;
       pi += 1, si += 1) {
    salt[si] ^= pass[pi];
  }
#else
  salt[0] = 0x0; salt[1] = 0x1; salt[2] = 0x2; salt[3] = 0x3;
  salt[4] = 0x4; salt[5] = 0x5; salt[6] = 0x6; salt[7] = 0x7;
  salt[8] = 0xF; salt[9] = 0xE; salt[10] = 0xD; salt[11] = 0xC;
  salt[12] = 0xB; salt[13] = 0xA; salt[14] = 0x9; salt[15] = 0x8;
#endif
}

int mtsd_container_is_valid(uint8_t* data, size_t size)
{
  if (size < sizeof(mtsd_header)) {
    return 0;
  }

  uint16_t crc = ((mtsd_header*)data)->crc16;
  uint16_t actual = crc16(data + MTSD_CRC_START, size - MTSD_CRC_START);

  return (crc == actual);
}

static int is_compressed_payload(mtsd_header* header) {
  return header->is_compressed & 0x80;
}

static void set_is_compressed_payload(mtsd_header* header, int is_compressed) {
  header->is_compressed = (is_compressed)
    ? header->is_compressed | 0x80
    : header->is_compressed & 0x7F;
}
