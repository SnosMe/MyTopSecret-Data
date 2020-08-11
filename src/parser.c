#include "mtsdata.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
// #include <Windows.h>

typedef enum MTSData_Token_Kind {
  MTSD_STREAM_START_TOKEN,
  MTSD_STREAM_END_TOKEN,
  MTSD_RECORD_SEPARATOR_TOKEN,
  MTSD_KEY_TOKEN,
  MTSD_VALUE_TOKEN,
  MTSD_VALUE_MULTILINE_TOKEN,
} mtsd_token_kind;

typedef struct {
  mtsd_token_kind kind;
  size_t start;
  size_t end;
  uint8_t* string;
  size_t string_size;
} mtsd_token;

typedef struct MTSData_Parser {
  size_t line;
  size_t column;
  size_t offset;

  struct {
    mtsd_read_callback callback;
    void *data;
  } input;

  struct {
    uint8_t mb_char[4];
    int eof;
  } reader;

  struct {
    size_t start;
    size_t end;
    uint8_t* buffer;
    size_t buffer_size;
  } lexer;
} mtsd_parser;

#define UTF8_LEN(bytes)    (((bytes)[0] & 0x80) == 0x00 ? 1 : \
                            ((bytes)[0] & 0xE0) == 0xC0 ? 2 : \
                            ((bytes)[0] & 0xF0) == 0xE0 ? 3 : \
                            ((bytes)[0] & 0xF8) == 0xF0 ? 4 : 0)

#define CURRENT(state)    ((state)->reader.mb_char[0])

static mtsd_res input_next(mtsd_parser *state);

static void print_state(mtsd_parser* state) {
  printf("Parser<> Ln: %zu, Col: %zu\n", state->line, state->column);
}

static mtsd_res lexer_add (mtsd_parser *state) {
  state->lexer.end += 1;
  size_t len = UTF8_LEN(state->reader.mb_char);
  state->lexer.buffer = realloc(state->lexer.buffer, state->lexer.buffer_size + len);
  if (state->lexer.buffer == NULL) {
    mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
    return MTSD_ERR;
  }
  memcpy(state->lexer.buffer + state->lexer.buffer_size, state->reader.mb_char, len);
  state->lexer.buffer_size += len;
  return MTSD_OK;
}

static void lexer_clear(mtsd_parser *state) {
  state->lexer.start = state->offset;
  state->lexer.end = state->offset;
  state->lexer.buffer_size = 0;
  if (state->lexer.buffer != NULL) {
    free(state->lexer.buffer);
    state->lexer.buffer = NULL;
  }
}

static mtsd_res parse_key(mtsd_parser *state) {
  for (;;) {
    if (CURRENT(state) == ':' || CURRENT(state) == '\n') {
      return MTSD_OK;
    } else {
      MTSD_CHECK(lexer_add(state));
    }

    MTSD_CHECK(input_next(state));
    if (state->reader.eof) return MTSD_OK;
  }
}

static mtsd_res parse_value(mtsd_parser *state) {
  for (;;) {
    if (CURRENT(state) == '\n') {
      return MTSD_OK;
    } else {
      MTSD_CHECK(lexer_add(state));
    }

    MTSD_CHECK(input_next(state));
    if (state->reader.eof) return MTSD_OK;
  }
}

static mtsd_res input_read_utf8(mtsd_parser *state, uint8_t *buff) {
  size_t read;
  if (!state->input.callback(state->input.data, buff, 1, &read)) {
    mtsd_error(MTSD_ESELF, MTSD_EREADER);
    return MTSD_ERR;
  }

  if (read == 0) {
    state->reader.eof = 1;
  } else {
    // Decode a UTF-8 character

    size_t width = UTF8_LEN(buff);

    if (!width) {
      mtsd_error_msg(MTSD_ESELF, MTSD_EREADER, "invalid leading UTF-8 octet");
      return MTSD_ERR;
    } else if (width > 1) {
      if (!state->input.callback(state->input.data, buff + 1, width - 1, &read)) {
        mtsd_error(MTSD_ESELF, MTSD_EREADER);
        return MTSD_ERR;
      }
      if (read != (width - 1)) {
        mtsd_error_msg(MTSD_ESELF, MTSD_EREADER, "incomplete UTF-8 octet sequence");
        return MTSD_ERR;
      }
    }

    uint8_t octet = buff[0];
    uint32_t value = (octet & 0x80) == 0x00 ? octet & 0x7F :
                     (octet & 0xE0) == 0xC0 ? octet & 0x1F :
                     (octet & 0xF0) == 0xE0 ? octet & 0x0F :
                     (octet & 0xF8) == 0xF0 ? octet & 0x07 : 0;

    for (size_t k = 1; k < width; k ++)
    {
      octet = buff[k];
      if ((octet & 0xC0) != 0x80) {
        mtsd_error_msg(MTSD_ESELF, MTSD_EREADER, "invalid trailing UTF-8 octet");
        return MTSD_ERR;
      } else {
        value = (value << 6) + (octet & 0x3F);
      }
    }

    if (!(
      (width == 1) ||
      (width == 2 && value >= 0x80) ||
      (width == 3 && value >= 0x800) ||
      (width == 4 && value >= 0x10000)
    )) {
      mtsd_error_msg(MTSD_ESELF, MTSD_EREADER, "invalid length of a UTF-8 sequence");
      return MTSD_ERR;
    }

    if ((value >= 0xD800 && value <= 0xDFFF) || value > 0x10FFFF) {
      mtsd_error_msg(MTSD_ESELF, MTSD_EREADER, "invalid Unicode character");
      return MTSD_ERR;
    }
  }

  return MTSD_OK;
}

static mtsd_res input_next(mtsd_parser *state) {
  MTSD_CHECK(input_read_utf8(state, state->reader.mb_char));
  state->offset += 1;
  if (state->reader.mb_char[0] == '\n') {
    state->line += 1;
    state->column = 0;
  } else if (!state->reader.eof) {
    state->column += 1;
  }
  return MTSD_OK;
}

static void lexer_trim(mtsd_parser *state, size_t max_start, size_t max_end) {
  if (max_start > 0) {
    size_t dt = 0;
    for (size_t i = 0; i < state->lexer.buffer_size && i < max_start; i += 1) {
      if (state->lexer.buffer[i] == ' ') {
        dt += 1;
      } else {
        break;
      }
    }
    memmove(state->lexer.buffer, state->lexer.buffer + dt, state->lexer.buffer_size - dt);
    state->lexer.start += dt;
    state->lexer.buffer_size -= dt;
  }
  if (max_end > 0) {
    size_t dt = 0;
    for (size_t i = 0; i < state->lexer.buffer_size && i < max_end; i += 1) {
      if (
        state->lexer.buffer[state->lexer.buffer_size - i - 1] == ' ' ||
        state->lexer.buffer[state->lexer.buffer_size - i - 1] == '\r'
      ) {
        dt += 1;
      } else {
        break;
      }
    }
    state->lexer.end -= dt;
    state->lexer.buffer_size -= dt;
  }
}

static mtsd_res lexer_next(mtsd_parser *state, mtsd_token *token) {
  for (;;) {
    MTSD_CHECK(input_next(state));
    if (state->reader.eof) break;

    lexer_clear(state);
    if (state->column == 1) {
      MTSD_CHECK(parse_key(state));
      if (state->lexer.buffer_size == 0) continue;

      if (
        state->lexer.buffer_size >= 2 &&
        state->lexer.buffer[0] == ' ' &&
        state->lexer.buffer[1] == ' '
      ) {
        token->kind = MTSD_VALUE_MULTILINE_TOKEN;
        if (CURRENT(state) == ':') {
          MTSD_CHECK(parse_value(state));
        }
        lexer_trim(state, 2, -1);
        token->start = state->lexer.start;
        token->end = state->lexer.end;
        token->string = state->lexer.buffer;
        token->string_size = state->lexer.buffer_size;
        return MTSD_OK;
      }

      lexer_trim(state, -1, -1);

      if (
        state->lexer.buffer_size == 3 &&
        state->lexer.buffer[0] == '-' &&
        state->lexer.buffer[1] == '-' &&
        state->lexer.buffer[2] == '-'
      ) {
        token->kind = MTSD_RECORD_SEPARATOR_TOKEN;
        token->start = state->lexer.start;
        token->end = state->lexer.end;
        return MTSD_OK;
      }

      if (state->lexer.buffer_size > 0) {
        token->kind = MTSD_KEY_TOKEN;
        token->start = state->lexer.start;
        token->end = state->lexer.end;
        token->string = state->lexer.buffer;
        token->string_size = state->lexer.buffer_size;
        return MTSD_OK;
      }
    } else {
      MTSD_CHECK(parse_value(state));
      lexer_trim(state, -1, -1);
      if (state->lexer.buffer_size == 0) continue;

      token->kind = MTSD_VALUE_TOKEN;
      token->start = state->lexer.start;
      token->end = state->lexer.end;
      token->string = state->lexer.buffer;
      token->string_size = state->lexer.buffer_size;
      return MTSD_OK;
    }
  }

  token->kind = MTSD_STREAM_END_TOKEN;
  token->start = state->offset;
  token->start = state->offset;
  return MTSD_OK;
}

mtsd_res parse(mtsd_read_callback read_callback, void *callback_data) {
  // SetConsoleOutputCP(CP_UTF8);

  mtsd_parser state;
  memset(&state, 0, sizeof(state));
  state.input.callback = read_callback;
  state.input.data = callback_data;
  state.offset = -1;
  state.line = 1;
  state.column = 0;

  for (;;) {
    mtsd_token token;
    MTSD_CHECK(lexer_next(&state, &token));
    if (token.kind == MTSD_STREAM_END_TOKEN) break;

    if (token.kind == MTSD_KEY_TOKEN) {
      printf("%04zu %04zu [KEY] \"%.*s\"\n", token.start, token.end, token.string_size, token.string);
    }
    else if (token.kind == MTSD_VALUE_TOKEN) {
      printf("%04zu %04zu [VALUE] \"%.*s\"\n", token.start, token.end, token.string_size, token.string);
    }
    else if (token.kind == MTSD_VALUE_MULTILINE_TOKEN) {
      printf("%04zu %04zu [VALUE_MULTILINE] \"%.*s\"\n", token.start, token.end, token.string_size, token.string);
    }
    else if (token.kind == MTSD_RECORD_SEPARATOR_TOKEN) {
      printf("%04zu %04zu [RECORD_SEPARATOR]\n", token.start, token.end);
    }
    else {
      printf("... unknown tok ...\n");
    }
  }

  print_state(&state);

  return MTSD_OK;
}
