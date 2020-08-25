import lzma
import argon2
from Crypto.Cipher import AES
from hexdump import hexdump

# pip install argon2-cffi pycryptodome hexdump
# python test/payload.py && hexdump -C data.mtsd.bin --skip 21

PASSWORD = b'pass'

SALT = bytes([
  0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
  0xF, 0xE, 0xD, 0xC, 0xB, 0xA, 0x9, 0x8,
  ])

PAYLOAD_UNCOMPRESSIBLE = bytes([1]) + (b'\x0212345678\x00' * 1)
PAYLOAD_COMPRESSIBLE =   bytes([4]) + (b'\x0212345678\x00' * 4)

# No option to disable writing 6-byte endmark
lz_enc = lzma.LZMACompressor(
  format=lzma.FORMAT_RAW,
  filters=[{
    "preset": 9,
    "id": lzma.FILTER_LZMA1,
    "dict_size": 0xFFFF,
    "lc": 0,
    "lp": 0,
    "pb": 0,
    "mode": lzma.MODE_NORMAL,
  }]
)

compressed = lz_enc.compress(PAYLOAD_COMPRESSIBLE) + lz_enc.flush()

derived_bytes = argon2.low_level.hash_secret_raw(
  secret=PASSWORD,
  salt=SALT,
  time_cost=1,
  memory_cost=1<<12,
  parallelism=1,
  hash_len=32+AES.block_size,
  type=argon2.Type.ID
)

def cipher():
  return AES.new(
    key=derived_bytes[:32],
    mode=AES.MODE_CTR,
    initial_value=derived_bytes[32:],
    nonce=b''
  )

print("# Compressible")
hexdump(cipher().encrypt(compressed))

print("# Uncompressible")
hexdump(cipher().encrypt(PAYLOAD_UNCOMPRESSIBLE))

print("---")
