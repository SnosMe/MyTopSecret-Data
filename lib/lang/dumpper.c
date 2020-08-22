#include "parser.h"
#include <stdio.h>

void mtsd_parser_print_token(mtsd_parser *state) {
  mtsd_token_kind tok_kind = state->lexer.kind;
  size_t start = state->lexer.start;
  size_t end = state->lexer.end;
  uint8_t *str = state->lexer.buffer;
  size_t str_len = state->lexer.buffer_size;

  if (tok_kind == MTSD_KEY_TOKEN) {
    printf("%04zu %04zu [KEY] \"%.*s\"\n", start, end, str_len, str);
  }
  else if (tok_kind == MTSD_VALUE_TOKEN) {
    printf("%04zu %04zu [VALUE] \"%.*s\"\n", start, end, str_len, str);
  }
  else if (tok_kind == MTSD_VALUE_MULTILINE_TOKEN) {
    printf("%04zu %04zu [VALUE_MULTILINE] \"%.*s\"\n", start, end, str_len, str);
  }
  else if (tok_kind == MTSD_RECORD_SEPARATOR_TOKEN) {
    printf("%04zu %04zu [RECORD_SEPARATOR]\n", start, end);
  }
  else {
    printf("... unknown tok ...\n");
  }
}
