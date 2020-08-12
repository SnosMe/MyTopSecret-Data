#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <uchar.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mtsdata.h"
#include "lang/parser.h"

struct Field {
  uint32_t key;
  uint8_t value;
};

static uint8_t raw_data[] = {
  2,1,0,115,110,56,115,109,101,64,103,109,97,105,108,46,99,111,109,0,2,0,80,11,116,114,117,55,57,52,53,0,2,1,0,10,97,116,9,108,105,98,121,116,101,64,103,109,97,105,10,46,99,111,109,0,2,0,2,53,57,52,98,110,116,121,0,2,1,0,10,114,46,107,111,108,97,56,4,64,103,109,97,105,108,45,99,111,139,0,2,0,117,105,111,57,57,56,52,63,0,2,3,0,77,65,77,79,72,87,45,83,45,83,72,65,70,84,9,77,0,2,0,55,77,75,57,55,1,49,75,79,71,1,
};

void mtsd_error(mtsd_error_source src, int error) {
  printf("mtsd_error\n");
}
void mtsd_error_msg(mtsd_error_source src, int error, char *msg) {
  printf("mtsd_error_msg: %s\n", msg);
}

void hexDump (const char * desc, const void * addr, const int len) {
    int i;
    unsigned char buff[17];
    const unsigned char * pc = (const unsigned char *)addr;

    if (desc != NULL)
        printf ("%s:\n", desc);

    for (i = 0; i < len; i++) {
        if ((i % 16) == 0) {
            if (i != 0)
                printf ("  %s\n", buff);
            printf ("  %04x ", i);
        }
        printf (" %02x", pc[i]);
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }
    printf ("  %s\n", buff);
}

void readfile(const char* filename, uint8_t** content, size_t* size) {  
  FILE* file = fopen(filename, "rb");
  if (file == NULL) {
    printf("Cannot open file  '%s'.\n", filename);
    exit(1);
  }

  struct stat file_stat;
  if (fstat(fileno(file), &file_stat) != 0) {
    printf("Cannot get info about file '%s'.\n", filename);
    exit(1);
  }

  *size = file_stat.st_size;
  *content = malloc(*size);
  if (*content == NULL) {
    printf("Cannot allocate memmory to read file.\n");
    exit(1);
  }

  int read = fread(*content, sizeof(uint8_t), *size, file);
  if (read != *size) {
    printf("Fatal error while reading file.\n");
    exit(1);
  }

  fclose(file);
}

typedef struct {
  uint8_t* content;
  size_t size;
  size_t read;
} cli_reader_state;

int read_handler(void *data, uint8_t *buffer, size_t size, size_t *size_read) {
  cli_reader_state *state = data;

  if (state->read == state->size) {
    *size_read = 0;
    return 1;
  }

  if (size > (state->size - state->read)) {
    size = state->size - state->read;
  }

  memcpy(buffer, state->content + state->read, size);
  state->read += size;
  *size_read = size;
  return 1;
}

int main() {
  cli_reader_state reader = { .read = 0 };
  readfile("./data.mtsd", &reader.content, &reader.size);
  mtsd_document doc;
  mtsd_parse(read_handler, &reader, &doc);
  free(reader.content);

  uint8_t* out = malloc(MTSD_HEADER_SIZE + MTSD_PAYLOAD_MAX_SIZE);
  memcpy(out + MTSD_HEADER_SIZE, raw_data, sizeof(raw_data));

  // hexDump("raw", out + MTSD_HEADER_SIZE, sizeof(raw_data));

  size_t compressed_size;
  if (compress_data(out + MTSD_HEADER_SIZE, sizeof(raw_data), &compressed_size) != 0) {
    printf("compress_data ERR\n");
    return 1;
  }
  memset(out + MTSD_HEADER_SIZE + compressed_size, 0, sizeof(raw_data) - compressed_size);

  encrypt(out + MTSD_HEADER_SIZE, compressed_size, "password", sizeof("password"), out + MTSD_HEADER_SIZE - MTSD_RANDOM_BYTES);

  printf("compressed_size: %d B, saved: %d B, ratio: %d%\n", compressed_size, sizeof(raw_data) - compressed_size, compressed_size * 100 / sizeof(raw_data));
  // hexDump("lzma", out + MTSD_HEADER_SIZE, sizeof(raw_data));

  uint8_t* normal;
  decompress_data(out + MTSD_HEADER_SIZE, compressed_size, &normal, sizeof(raw_data));
  hexDump("restored", normal, sizeof(raw_data));

  return 0;
}
