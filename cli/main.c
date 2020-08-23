#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <mtsdata.h>
#include <unistd.h>

void mtsd_error(mtsd_error_source src, int error, char* msg) {
  printf("error [libmtsd]: source - %d; code - %d, message - %s\n", src, error, msg);
}
void* mtsd_malloc(size_t size) {
  return malloc(size);
}
void* mtsd_realloc(void* ptr, size_t size) {
  return realloc(ptr, size);
}
void mtsd_free(void* ptr) {
  free(ptr);
}

void read_from_file(const char* filename, uint8_t** content, size_t* size);
void write_to_file(const char* filename, uint8_t* data, size_t size);

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

static size_t records_count(mtsd_document* doc);

int main(int argc, char **argv) {
  if (argc == 1 || !strcmp(argv[1], "--help")) {
    printf(
      "MyTopSecret-Data v1.0 (c) Alexander Drozdov\n"
      "----------------\n"
      "     __ ____    ____  _________   ______   ______\n"
      "    / /|_   \\  /   _||  _   _  |.' ____ \\ |_   _ `.\n"
      "   / /   |   \\/   |  |_/ | | \\_|| (___ \\_|  | | `. \\\n"
      "  / /    | |\\  /| |      | |     _.____`.   | |  | |\n"
      " / /    _| |_\\/_| |_    _| |_   | \\____) | _| |_.' /_\n"
      "/_/    |_____||_____|  |_____|   \\______.'|______.'(_)\n\n"
      "USAGE:\n\n"
      "  mtsd -e plaintext.mtsd -o encrypted.bin\n"
      "  mtsd encrypted.bin\n\n"
      "OPTIONS:\n\n"
      "  -e <file>        Encrypt file\n"
      "  -d <file>        Decrypt file\n"
      "  -o <file>        Save output to file. If ommited prints to stdout\n"
      "  --help           Help\n"
    );
    exit(1);
  }

  int encrypt = 0;
  int decrypt = 0;
  char* input_filename = NULL;
  char* output_filename = NULL;

  for (int i = 1; i < argc; i += 1) {
    if (!strcmp(argv[i], "-e")) {
      encrypt = 1;
    }
    else if (!strcmp(argv[i], "-d")) {
      decrypt = 1;
    }
    else if (!strcmp(argv[i], "-o")) {
      if ((i + 1) >= argc) { printf("Missing value for '-o' option.\n"); exit(1); }
      output_filename = argv[++i];
    }
    else {
      input_filename = argv[i];
    }
  }

  if (!input_filename) {
    printf("Input file name is required.\n");
    exit(1);
  }
  if (!output_filename && encrypt) {
    printf("Output file name is required. (use '-o' option)\n");
    exit(1);
  }

  if (encrypt) {
    cli_reader_state reader = { .read = 0 };
    read_from_file(input_filename, &reader.content, &reader.size);

    printf("Parsing file '%s' (%zu bytes)\n", input_filename, reader.size);
    mtsd_document doc;
    if (!mtsd_parse(read_handler, &reader, &doc)) {
      printf("Cannot parse file\n");
      exit(1);
    }
    free(reader.content);

    printf("Encrypting %zu records\n", mtsd_doc_records_count(&doc));

    char* password = getpass("Password: ");

    uint8_t* encrypted_data = NULL;
    size_t encrypted_size = 0;
    if (!mtsd_encrypt(&doc, (uint8_t*)password, strlen(password), &encrypted_data, &encrypted_size)) {
      printf("Cannot encrypt file\n");
      exit(1);
    }

    printf("Writing %zu bytes to '%s'\n", encrypted_size, output_filename);
    write_to_file(output_filename, encrypted_data, encrypted_size);
  } else {
    uint8_t* encrypted_data = NULL;
    size_t encrypted_size = 0;
    read_from_file(input_filename, &encrypted_data, &encrypted_size);

    printf("Opening file '%s'\n", input_filename);

    char* password = getpass("Password: ");

    printf("Decrypting...\n");

    mtsd_document doc;
    if (!mtsd_decrypt(encrypted_data, encrypted_size, (uint8_t*)password, strlen(password), &doc)) {
      printf("Cannot decrypt file\n");
      exit(1);
    }
    free(encrypted_data);

    uint8_t* plaintext = NULL;
    size_t text_size = 0;
    if (!mtsd_to_text(&doc, &plaintext, &text_size)) {
      printf("Cannot convert data to text\n");
      exit(1);
    }

    if (output_filename) {
      printf("Saving %zu records to '%s'\n", mtsd_doc_records_count(&doc), output_filename);
      write_to_file(output_filename, plaintext, text_size);
    } else {
      printf("\n%.*s", text_size, plaintext);
    }
  }

  return 0;
}
