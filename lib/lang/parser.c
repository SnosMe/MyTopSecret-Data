#include "parser.h"
#include <stdlib.h>
#include <string.h>

#define TOKEN(state)    ((state)->lexer)

static mtsd_res parser_unexpected();
static mtsd_res parse_field(mtsd_parser *state, mtsd_field *field);
static mtsd_res parse_record(mtsd_parser *state, mtsd_record *record);
static mtsd_res parse_doc(mtsd_parser *state, mtsd_document *doc);

static char* MTSD_KEYS[] = {
  "__NULL",
  "email",
  "password",
  "login",
};

static mtsd_res parser_unexpected() {
  // TODO err_msg
  return MTSD_ERR;
}

static mtsd_res parse_field(mtsd_parser *state, mtsd_field *field) {
  if (TOKEN(state).kind != MTSD_KEY_TOKEN) {
    return parser_unexpected();
  } else {
    state->lexer.consumed = 1;
  }

  // @TODO
  field->key = 1;

  size_t is_multiline = 0;
  for (;;) {
    MTSD_CHECK(lexer_next(state));

    if (TOKEN(state).kind == MTSD_VALUE_TOKEN && !is_multiline) {
      field->value = state->lexer.buffer;
      field->value_size = state->lexer.buffer_size;
      state->lexer.buffer = NULL;
      state->lexer.consumed = 1;
      return MTSD_OK;
    } else if (TOKEN(state).kind == MTSD_VALUE_MULTILINE_TOKEN) {
      field->value_size += (state->lexer.buffer_size + (is_multiline == 0 ? 0 : 1));
      field->value = realloc(field->value, field->value_size);
      if (field->value == NULL) {
        mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
        return MTSD_ERR;
      }
      if (is_multiline) {
        field->value[field->value_size - state->lexer.buffer_size - 1] = '\n';
      }
      memcpy(field->value + (field->value_size - state->lexer.buffer_size), state->lexer.buffer, state->lexer.buffer_size);
      state->lexer.consumed = 1;
      is_multiline = 1;
    } else {
      return MTSD_OK;
    }
  }
}

static mtsd_res parse_record(mtsd_parser *state, mtsd_record *record) {
  mtsd_field *current = record->fields;
  if (current == NULL) {
    current = malloc(sizeof(mtsd_field));
    if (current == NULL) {
      mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
      return MTSD_ERR;
    }
    record->fields = current;
  } else {
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = malloc(sizeof(mtsd_field));
    if (current->next == NULL) {
      mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
      return MTSD_ERR;
    }
    current = current->next;
  }

  current->key = 0;
  current->value = NULL;
  current->value_size = 0;
  current->next = NULL;

  MTSD_CHECK(parse_field(state, current));
  return MTSD_OK;
}

static mtsd_res parse_doc(mtsd_parser *state, mtsd_document *doc) {
  mtsd_record *current = malloc(sizeof(mtsd_record));
  if (current == NULL) {
    mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
    return MTSD_ERR;
  }
  current->fields = NULL;
  current->next = NULL;
  doc->records = current;

  for (;;) {
    MTSD_CHECK(lexer_next(state));
    if (TOKEN(state).kind == MTSD_STREAM_END_TOKEN) return MTSD_OK;

    if (TOKEN(state).kind == MTSD_RECORD_SEPARATOR_TOKEN) {
      current->next = malloc(sizeof(mtsd_record));
      if (current->next == NULL) {
        mtsd_error(MTSD_ESELF, MTSD_EMEMORY);
        return MTSD_ERR;
      }
      current = current->next;
      current->fields = NULL;
      current->next = NULL;
      state->lexer.consumed = 1;
    } else {
      MTSD_CHECK(parse_record(state, current));
    }
  }
}

mtsd_res mtsd_parse(mtsd_read_callback read_callback, void *callback_data, mtsd_document *doc) {
  mtsd_parser state;
  memset(&state, 0, sizeof(state));
  state.input.callback = read_callback;
  state.input.data = callback_data;
  state.offset = -1;
  state.line = 1;
  state.column = 0;
  state.lexer.consumed = 1;

  MTSD_CHECK(parse_doc(&state, doc));

  // for (;;) {
  //   MTSD_CHECK(lexer_next(state));
  //   if (TOKEN(state).kind == MTSD_STREAM_END_TOKEN) {
  //     break;
  //   } else {
  //     print_token(state);
  //     state->lexer.consumed = 1;
  //   }
  // }

  return MTSD_OK;
}
