#include "mtsdata.h"
#include "lang/parser.h"
#include <string.h>
// #include <Windows.h>

mtsd_res parse(mtsd_read_callback read_callback, void *callback_data) {
  // SetConsoleOutputCP(CP_UTF8);

  mtsd_parser state;
  memset(&state, 0, sizeof(state));
  state.input.callback = read_callback;
  state.input.data = callback_data;
  state.offset = -1;
  state.line = 1;
  state.column = 0;

  mtsd_document doc = {
    .size = 0,
    .records = NULL
  };
  _parse(&state, &doc);

  return MTSD_OK;
}
