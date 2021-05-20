#include "parser.h"

#include "../private.h"

#include <string.h>

#define TOKEN(state) ((state)->lexer)

static mtsd_res parser_unexpected();
static mtsd_res parse_field(mtsd_parser* state, mtsd_field* field);
static mtsd_res parse_record(mtsd_parser* state, mtsd_record* record);
static mtsd_res parse_doc(mtsd_parser* state, mtsd_document* doc);

static mtsd_res parser_unexpected()
{
  // TODO err_msg
  return MTSD_ERR;
}

static mtsd_res parse_field(mtsd_parser* state, mtsd_field* field)
{
  if (TOKEN(state).kind != MTSD_KEY_TOKEN) {
    return parser_unexpected();
  } else {
    state->lexer.consumed = 1;
  }

  field->key = mtsd_doc_get_key_id(state->lexer.buffer, state->lexer.buffer_size);
  if (!field->key) {
    mtsd_error(MTSD_ESELF, MTSD_EPARSE_UNKNOWN_KEY, "unknown key");
    return MTSD_ERR;
  }

  int is_multiline = 0;
  for (;;) {
    MTSD_CHECK(mtsd_parser_lexer_next(state));

    if (TOKEN(state).kind == MTSD_VALUE_TOKEN && !is_multiline) {
      field->value = state->lexer.buffer;
      field->value_size = state->lexer.buffer_size;
      state->lexer.buffer = NULL;
      state->lexer.consumed = 1;
      return MTSD_OK;
    } else if (TOKEN(state).kind == MTSD_VALUE_MULTILINE_TOKEN) {
      field->value_size += (state->lexer.buffer_size + (is_multiline == 0 ? 0 : 1));
      MTSD_REALLOC(field->value, field->value_size);
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

static mtsd_res parse_record(mtsd_parser* state, mtsd_record* record)
{
  mtsd_field* current = record->fields;
  if (!current) {
    MTSD_MALLOC(current, sizeof(mtsd_field));
    record->fields = current;
  } else {
    while (current->next) {
      current = current->next;
    }
    MTSD_MALLOC(current->next, sizeof(mtsd_field));
    current = current->next;
  }
  mtsd_doc_field_init(current);

  MTSD_CHECK(parse_field(state, current));
  return MTSD_OK;
}

static mtsd_res parse_doc(mtsd_parser* state, mtsd_document* doc)
{
  mtsd_record* current = NULL;
  MTSD_MALLOC(current, sizeof(mtsd_field));
  doc->records = current;
  mtsd_doc_record_init(current);

  for (;;) {
    MTSD_CHECK(mtsd_parser_lexer_next(state));
    if (TOKEN(state).kind == MTSD_STREAM_END_TOKEN)
      return MTSD_OK;

    if (TOKEN(state).kind == MTSD_RECORD_SEPARATOR_TOKEN) {
      MTSD_MALLOC(current->next, sizeof(mtsd_field));
      current = current->next;
      mtsd_doc_record_init(current);
      state->lexer.consumed = 1;
    } else {
      MTSD_CHECK(parse_record(state, current));
    }
  }
}

mtsd_res mtsd_parse(mtsd_read_callback read_callback, void* callback_data, mtsd_document* doc)
{
  mtsd_parser state = {
    .line = 1,
    .column = 0,
    .offset = -1,

    .input = {
      .callback = read_callback,
      .data = callback_data
    },

    .reader = {
      .mb_char = {0, 0, 0, 0},
      .mb_size = 0,
      .eof = 0,
    },

    .lexer = {
      .kind = MTSD_STREAM_START_TOKEN,
      .start = 0,
      .end = 0,
      .buffer = NULL,
      .buffer_size = 0,
      .consumed = 1
    }
  };

  mtsd_doc_init(doc);
  if (!parse_doc(&state, doc)) {
    mtsd_doc_free(doc);
    MTSD_FREE(state.lexer.buffer);
    return MTSD_ERR;
  } else {
    MTSD_FREE(state.lexer.buffer);
    return MTSD_OK;
  }

  // for (;;) {
  //   MTSD_CHECK(mtsd_parser_lexer_next(&state));
  //   if (TOKEN(&state).kind == MTSD_STREAM_END_TOKEN) {
  //     break;
  //   } else {
  //     mtsd_parser_print_token(&state);
  //     state.lexer.consumed = 1;
  //   }
  // }
}
