#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <uchar.h>
#include "mtsdata.h"

struct Field {
  uint32_t key;
  uint8_t value;
};

static uint8_t raw_data[] = {
  2,1,0,115,110,56,115,109,101,64,103,109,97,105,108,46,99,111,109,0,2,0,80,11,116,114,117,55,57,52,53,0,2,1,0,10,97,116,9,108,105,98,121,116,101,64,103,109,97,105,10,46,99,111,109,0,2,0,2,53,57,52,98,110,116,121,0,2,1,0,10,114,46,107,111,108,97,56,4,64,103,109,97,105,108,45,99,111,139,0,2,0,117,105,111,57,57,56,52,63,0,2,3,0,77,65,77,79,72,87,45,83,45,83,72,65,70,84,9,77,0,2,0,55,77,75,57,55,1,49,75,79,71,0,
};

int main() {
  uint8_t* compressed;
  size_t compressed_size;
  if (compress_data(raw_data, 134, &compressed, &compressed_size) != 0) {
    printf("compress_data ERR\n");
    return 1;
  }

  printf("compressed_size: %d B, saved: %d B, ratio: %d%\n", compressed_size, 134 - compressed_size, compressed_size * 100 / 134);

  uint8_t* normal;
  decompress_data(compressed, compressed_size, &normal, 134);

  printf("normal: %d\n", normal[3]);

  return 0;
}
