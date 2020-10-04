#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>

void read_from_file(const char* filename, uint8_t** content, size_t* size) {
  FILE* file = fopen(filename, "rb");
  if (!file) {
    printf("Cannot open file '%s'.\n", filename);
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
    printf("Cannot allocate memory to read file.\n");
    exit(1);
  }

  int read = fread(*content, sizeof(uint8_t), *size, file);
  if (read != *size) {
    printf("Fatal error while reading file.\n");
    exit(1);
  }

  fclose(file);
}

void write_to_file(const char* filename, uint8_t* data, size_t size) {
  FILE* file = fopen(filename, "wb");
  if (!file) {
    printf("Cannot open file '%s'.\n", filename);
    exit(1);
  }

  if (fwrite(data, sizeof(uint8_t), size, file) != size) {
    printf("Fatal error while writing to file.\n");
    exit(1);
  }

  fclose(file);
}
