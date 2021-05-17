#include "private.h"

#include <aes.h>
#include <argon2.h>
#include <randombytes.h>

static mtsd_res derive_bytes(uint8_t* salt, uint8_t* pwd, size_t pwd_size, uint8_t* out);

mtsd_res mtsd_encrypt_payload(uint8_t* data, size_t data_size,
                              uint8_t* pwd, size_t pwd_size, uint8_t* salt)
{
  uint8_t derived_bytes[AES_KEYLEN + AES_BLOCKLEN];
  MTSD_CHECK(derive_bytes(salt, pwd, pwd_size, derived_bytes));
  uint8_t* key = derived_bytes;
  uint8_t* iv = derived_bytes + AES_KEYLEN;

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CTR_xcrypt_buffer(&ctx, data, data_size);
  return MTSD_OK;
}

mtsd_res mtsd_decrypt_payload(uint8_t* data, size_t data_size,
                              uint8_t* pwd, size_t pwd_size, uint8_t* salt)
{
  uint8_t derived_bytes[AES_KEYLEN + AES_BLOCKLEN];
  MTSD_CHECK(derive_bytes(salt, pwd, pwd_size, derived_bytes));
  uint8_t* key = derived_bytes;
  uint8_t* iv = derived_bytes + AES_KEYLEN;

  struct AES_ctx ctx;
  AES_init_ctx_iv(&ctx, key, iv);
  AES_CTR_xcrypt_buffer(&ctx, data, data_size);
  return MTSD_OK;
}

static int argon2_alloc(uint8_t** memory, size_t bytes_to_allocate)
{
  *memory = mtsd_malloc(bytes_to_allocate);
  return 0;
}
static void argon2_free(uint8_t* memory, size_t bytes_to_allocate)
{
  mtsd_free(memory);
}

static mtsd_res derive_bytes(uint8_t* salt, uint8_t* pwd, size_t pwd_size, uint8_t* out)
{
  struct Argon2_Context ctx = {
    .out = out, .outlen = (AES_KEYLEN + AES_BLOCKLEN),
    .pwd = pwd, .pwdlen = pwd_size,
    .salt = salt, .saltlen = MTSD_SALT_SIZE,

    .t_cost = MTSD_ARGON2_ITER,
    .m_cost = MTSD_ARGON2_MEM,
    .lanes = MTSD_ARGON2_THREADS,
    .threads = MTSD_ARGON2_THREADS,

    .secret = NULL, .secretlen = 0,
    .ad = NULL, .adlen = 0,
    .version = ARGON2_VERSION_13,
    .allocate_cbk = argon2_alloc, .free_cbk = argon2_free,
    .flags = ARGON2_DEFAULT_FLAGS
  };

  int err = argon2id_ctx(&ctx);
  if (err != ARGON2_OK) {
    mtsd_error(MTSD_EARGON2, err, NULL);
    return MTSD_ERR;
  }
  return MTSD_OK;
}

mtsd_res mtsd_random_bytes(uint8_t* out, size_t size)
{
  int err = randombytes(out, size);
  if (err != 0) {
    mtsd_error(MTSD_ERANDOMBYTES, err, NULL);
    return MTSD_ERR;
  }
  return MTSD_OK;
}
