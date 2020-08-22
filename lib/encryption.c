#include "mtsdata.h"
#include "private.h"

#include <aes.h>
#include <argon2.h>
#include <randombytes.h>

static mtsd_res derive_bytes(uint8_t* random_bytes, uint8_t* pwd, size_t pwd_size, uint8_t* out);

mtsd_res encrypt(uint8_t* data, size_t data_size,
                 uint8_t* pwd, size_t pwd_size, uint8_t* random_bytes) {
  int err = randombytes(random_bytes, MTSD_RANDOM_BYTES);
  if (err != 0) {
    mtsd_error(MTSD_ERANDOMBYTES, err);
    return MTSD_ERR;
  }

  uint8_t derived_bytes[AES_KEYLEN + AES_BLOCKLEN];
  MTSD_CHECK (derive_bytes(random_bytes, pwd, pwd_size, derived_bytes));
  uint8_t* key = derived_bytes;
  uint8_t* iv = derived_bytes + AES_KEYLEN;

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CTR_xcrypt_buffer(&ctx, data, data_size);
  return MTSD_OK;
}

mtsd_res decrypt(uint8_t* data, size_t data_size,
                 uint8_t* pwd, size_t pwd_size, uint8_t* random_bytes) {
  uint8_t derived_bytes[AES_KEYLEN + AES_BLOCKLEN];
  MTSD_CHECK (derive_bytes(random_bytes, pwd, pwd_size, derived_bytes));
  uint8_t* key = derived_bytes;
  uint8_t* iv = derived_bytes + AES_KEYLEN;

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CTR_xcrypt_buffer(&ctx, data, data_size);
  return MTSD_OK;
}

static mtsd_res derive_bytes(uint8_t* random_bytes, uint8_t* pwd, size_t pwd_size, uint8_t* out) {
  struct Argon2_Context ctx = {
    .out = out, .outlen = (AES_KEYLEN + AES_BLOCKLEN),
    .pwd = pwd, .pwdlen = pwd_size,
    .salt = random_bytes, .saltlen = MTSD_RANDOM_BYTES,

    .t_cost = 10,
    .m_cost = (1 << 15), // 32 MB
    .lanes = 1,
    .threads = 1,

    .secret = NULL, .secretlen = 0,
    .ad = NULL, .adlen = 0,
    .version = ARGON2_VERSION_13,
    .allocate_cbk = NULL, .free_cbk = NULL,
    .flags = ARGON2_DEFAULT_FLAGS
  };

  int err = argon2id_ctx(&ctx);
  if (err != ARGON2_OK) {
    mtsd_error(MTSD_EARGON2, err);
    return MTSD_ERR;
  }
  return MTSD_OK;
}
