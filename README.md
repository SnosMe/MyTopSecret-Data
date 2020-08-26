## MyTopSecret-Data

Encrypts data with emphasis on storage space.

### Binary layout

```
|0|1,2|3,4,5,6|7,8,9,10|11|12........n|

[ 0;  0] - Magic number
[ 1;  2] - CRC16 of [3; n]
[ 3;  6] - Time in seconds since 01/01/1999
[ 7; 10] - Random bytes
[11; 11] - Is payload compressed
[12;  n] - AES encrypted data
```

### Use cases

- Printing in Hex format or as a 2D barcode:
  - physical backup for important passwords

### Quick guide

Create file with content:

```
url: github
email: test@example.com
password: 12345678
---
url: discord
email: test@example.com
password: 87654321
```

Encrypt

```bash
mtsd -e plaintext.mtsd -o encrypted.bin
```

Decrypt

```bash
mtsd encrypted.bin
```

### Encryption

```
pwd = from_user_input()
salt = derive_salt_from(time_now, random_bytes)
key, nonce = Argon2d(iter=2,mem=512MB,parallel=4)
encrypted = AES256_CTR(data, key, nonce)
```

### Compatibility

Backward compatibility is not supported in the current implementation. Any changes to Argon2 complexity or LZMA parameters will break the ability to decrypt old data.\
This project was created for personal use.

### Comparison with password managers

- 1Password\
  https://1password.com/files/1Password-White-Paper.pdf

  Master Password + Secret key (Account key, ~16 bytes, stored local only)\
  PBKDF2-HMAC-SHA256 with 100,000+100,000 iterations\
  AES-256-GCM

- Dashlane\
  https://www.dashlane.com/download/Dashlane_SecurityWhitePaper_September2019.pdf

  Argon2d with 3 iterations, 32 MB memory cost, and 2 parallel tasks\
    or PBKDF2-HMAC-SHA256 with 200,000 iterations\
  AES-256-CBC

- LastPass\
  https://enterprise.lastpass.com/wp-content/uploads/LastPass-Technical-Whitepaper-3.pdf

  PBKDF2-HMAC-SHA256 with 100,100 iterations\
  AES-256-CBC
