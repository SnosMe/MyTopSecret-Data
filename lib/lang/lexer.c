#include "parser.h"
#include <stdlib.h>
#include <string.h>

#define CURRENT(state)    ((state)->reader.mb_char[0])

static mtsd_res lexer_add (mtsd_parser *state);
static void lexer_clear(mtsd_parser *state);
static mtsd_res lexer_read_key(mtsd_parser *state);
static mtsd_res lexer_read_value(mtsd_parser *state);
static void lexer_trim(mtsd_parser *state, size_t max_start, size_t max_end);

static mtsd_res lexer_add (mtsd_parser *state) {
  state->lexer.end += 1;
  size_t len = state->reader.mb_size;
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


static mtsd_res lexer_read_key(mtsd_parser *state) {
  for (;;) {
    if (CURRENT(state) == ':' || CURRENT(state) == '\n') {
      return MTSD_OK;
    } else {
      MTSD_CHECK(lexer_add(state));
    }

    MTSD_CHECK(mtsd_parser_input_next(state));
    if (state->reader.eof) return MTSD_OK;
  }
}

static mtsd_res lexer_read_value(mtsd_parser *state) {
  for (;;) {
    if (CURRENT(state) == '\n') {
      return MTSD_OK;
    } else {
      MTSD_CHECK(lexer_add(state));
    }

    MTSD_CHECK(mtsd_parser_input_next(state));
    if (state->reader.eof) return MTSD_OK;
  }
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

mtsd_res mtsd_parser_lexer_next(mtsd_parser *state) {
  if (!state->lexer.consumed) {
    return MTSD_OK;
  } else {
    state->lexer.consumed = 0;
  }

  for (;;) {
    MTSD_CHECK(mtsd_parser_input_next(state));
    if (state->reader.eof) break;

    lexer_clear(state);
    if (state->column == 1) {
      MTSD_CHECK(lexer_read_key(state));

      if (
        state->lexer.buffer_size >= 2 &&
        state->lexer.buffer[0] == ' ' &&
        state->lexer.buffer[1] == ' '
      ) {
        state->lexer.kind = MTSD_VALUE_MULTILINE_TOKEN;
        if (CURRENT(state) == ':') {
          MTSD_CHECK(lexer_read_value(state));
        }
        lexer_trim(state, 2, -1);
        return MTSD_OK;
      }

      lexer_trim(state, -1, -1);

      if (
        state->lexer.buffer_size == 3 &&
        state->lexer.buffer[0] == '-' &&
        state->lexer.buffer[1] == '-' &&
        state->lexer.buffer[2] == '-'
      ) {
        state->lexer.kind = MTSD_RECORD_SEPARATOR_TOKEN;
        return MTSD_OK;
      }

      if (state->lexer.buffer_size > 0) {
        state->lexer.kind = MTSD_KEY_TOKEN;
        return MTSD_OK;
      }
    } else {
      MTSD_CHECK(lexer_read_value(state));
      
      lexer_trim(state, -1, -1);

      if (state->lexer.buffer_size > 0) {
        state->lexer.kind = MTSD_VALUE_TOKEN;
        return MTSD_OK;
      }
    }
  }

  state->lexer.kind = MTSD_STREAM_END_TOKEN;
  state->lexer.start = state->offset;
  state->lexer.end = state->offset;
  return MTSD_OK;
}
