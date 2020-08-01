## MyTopSecret-Data

Encrypts data with emphasis on storage space.

### Binary layout

```
|0,1,2...........14,15|16,17|18.....n-3|n-2,n-1|

[  0;  15] - random 16 bytes
[ 16;  17] - creation day (local)
[ 18; n-3] - AES encrypted data
[n-2; n-1] - CRC16 of [0; n-3]
        n  - binary length
```

### Use cases

- Printing in HEX format or as a DataMatrix:
  - physical backup for important passwords

### Quick guide

Create file with content:

```
url: facebook
email: test@example.com
password: 12345678
---
url: discord
email: test@example.com
password: 87654321
```

Encrypt

```bash
mtsd -e filename.txt
```

Decrypt

```bash
mtsd filename.dat
```

### Encryption

```
pwd = from_user_input()
rnd_bytes = crypto_random(n=16 bytes)
key = pfsdppdsfs(pwd, 256, salt=rnd_bytes)
encrypted = AES256_CTR(data, key, nonce=rnd_bytes)
```
