#include "mtsdata.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <uchar.h>
#include <Windows.h>

typedef enum MTSData_Token_Kind {
  MTSD_STREAM_START_TOKEN,
  MTSD_STREAM_END_TOKEN,
  MTSD_RECORD_SEPARATOR_TOKEN,
  MTSD_STRING_TOKEN,
  MTSD_COLON_TOKEN,
  MTSD_VALUE_MULTILINE_TOKEN,
} mtsd_token_kind;

#define UTF8_MAX 4

typedef struct MTSData_Parser {
  size_t line;
  size_t column;
  uint8_t* start;
  uint8_t* end;
  uint8_t* current;

  struct {
    mtsd_read_callback callback;
    void *data;
  } input;

  struct {
    uint8_t cBuff[UTF8_MAX];
    uint8_t nBuff[UTF8_MAX];
    int next_valid;
    int eof;
  } reader;

  struct {
    size_t start;
    size_t end;
    int kind;
    uint8_t buffer;
  } lexer;

  uint8_t *buffer_old;
  size_t size;
} mtsd_parser;

#define CURRENT(state)    (*(state)->current)

static void print_state(mtsd_parser* state) {
  printf("Parser<> Ln: %zu, Col: %zu\n", state->line, state->column);
}

static int can_read(mtsd_parser *state, size_t n) {
  return ((state->current + n) < (state->buffer_old + state->size));
}

static uint8_t peek(mtsd_parser *state, size_t n) {
  return (state->current < (state->buffer_old + state->size));
}

static uint8_t read_next(mtsd_parser *state) {
  if (!can_read(state, 1)) {
    return '\0';
  }
  ++state->current;

  if (CURRENT(state) == '\r' || CURRENT(state) == '\n') {

  }

  return CURRENT(state);
}

static void parse_string(mtsd_parser* state) {
}

#define UTF8_LEN(bytes)    (((bytes)[0] & 0x80) == 0x00 ? 1 : \
                            ((bytes)[0] & 0xE0) == 0xC0 ? 2 : \
                            ((bytes)[0] & 0xF0) == 0xE0 ? 3 : \
                            ((bytes)[0] & 0xF8) == 0xF0 ? 4 : 0)

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

static mtsd_res input_peek(mtsd_parser *state);

static mtsd_res input_next(mtsd_parser *state) {
  if (state->reader.next_valid) {
    memcpy(state->reader.cBuff, state->reader.nBuff, UTF8_MAX);
    state->reader.next_valid = 0;
    return MTSD_OK;
  }
  MTSD_CHECK(input_read_utf8(state, state->reader.cBuff));
  if (!state->reader.eof && state->reader.cBuff[0] == '\r') {
    MTSD_CHECK(input_peek(state));
    if (!state->reader.eof && state->reader.nBuff[0] == '\n') {
      state->reader.cBuff[0] = '\n';
      state->reader.next_valid = 0;
    }
  }
  if (state->reader.cBuff[0] == '\n') {
    state->line += 1;
    state->column = 0;
  } else if (!state->reader.eof) {
    state->column += 1;
  }
  return MTSD_OK;
}

static mtsd_res input_peek(mtsd_parser *state) {
  if (state->reader.next_valid) {
    return MTSD_OK;
  }
  MTSD_CHECK(input_read_utf8(state, state->reader.nBuff));
  state->reader.next_valid = 1;
  return MTSD_OK;
}

mtsd_res parse(mtsd_read_callback read_callback, void *callback_data) {
  SetConsoleOutputCP(CP_UTF8);

  mtsd_parser state;
  memset(&state, 0, sizeof(state));
  state.input.callback = read_callback;
  state.input.data = callback_data;
  state.line = 1;
  state.column = 0;
  state.end = NULL;

  while (input_next(&state) && !state.reader.eof) {
    // if ()
  }

  while (input_next(&state) && !state.reader.eof) {
    if (state.reader.cBuff[0] == '\n') {
      printf("[\\n]\n");
    } else {
      printf("[%.*s] Ln: %zu, Col: %zu\n", UTF8_LEN(state.reader.cBuff), state.reader.cBuff, state.line, state.column);
    }
  }
  print_state(&state);

  return MTSD_OK;
}
