#include "parser.h"

mtsd_res _parse(mtsd_parser *state, mtsd_document *doc) {
  for (;;) {
    MTSD_CHECK(lexer_next(state));
    mtsd_token_kind kind = state->lexer.kind;
    if (kind == MTSD_STREAM_END_TOKEN) break;

    print_token(state);

    if (kind == MTSD_KEY_TOKEN) {
    }
    else if (kind == MTSD_VALUE_TOKEN) {
    }
    else if (kind == MTSD_VALUE_MULTILINE_TOKEN) {
    }
    else if (kind == MTSD_RECORD_SEPARATOR_TOKEN) {
    }
    else {
      // return MTSD_ERR;
    }
  }

  return MTSD_OK;
}
