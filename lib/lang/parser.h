#ifndef MTSD_LANG_PARSER_H
#define MTSD_LANG_PARSER_H

#include "mtsdata.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
  MTSD_STREAM_START_TOKEN,
  MTSD_STREAM_END_TOKEN,
  MTSD_RECORD_SEPARATOR_TOKEN,
  MTSD_KEY_TOKEN,
  MTSD_VALUE_TOKEN,
  MTSD_VALUE_MULTILINE_TOKEN,
} mtsd_token_kind;

typedef struct {
  size_t line;
  size_t column;
  size_t offset;

  struct {
    mtsd_read_callback callback;
    void *data;
  } input;

  struct {
    uint8_t mb_char[4];
    size_t mb_size;
    int eof;
  } reader;

  struct {
    mtsd_token_kind kind;
    size_t start;
    size_t end;
    uint8_t* buffer;
    size_t buffer_size;
    int consumed;
  } lexer;
} mtsd_parser;

mtsd_res mtsd_parser_input_next(mtsd_parser *state);
mtsd_res mtsd_parser_lexer_next(mtsd_parser *state);
void mtsd_parser_print_token(mtsd_parser *state);

#endif
