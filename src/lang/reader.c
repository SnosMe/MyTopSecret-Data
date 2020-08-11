#include "parser.h"

#define UTF8_LEN(bytes)    (((bytes)[0] & 0x80) == 0x00 ? 1 : \
                            ((bytes)[0] & 0xE0) == 0xC0 ? 2 : \
                            ((bytes)[0] & 0xF0) == 0xE0 ? 3 : \
                            ((bytes)[0] & 0xF8) == 0xF0 ? 4 : 0)

static mtsd_res input_read_utf8(mtsd_parser *state) {
  size_t read;
  uint8_t *buff = state->reader.mb_char;
  if (!state->input.callback(state->input.data, buff, 1, &read)) {
    mtsd_error(MTSD_ESELF, MTSD_EREADER);
    return MTSD_ERR;
  }

  if (read == 0) {
    state->reader.eof = 1;
  } else {
    // Decode a UTF-8 character

    size_t width = UTF8_LEN(buff);
    state->reader.mb_size = width;

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

mtsd_res input_next(mtsd_parser *state) {
  MTSD_CHECK(input_read_utf8(state));
  state->offset += 1;
  if (state->reader.mb_char[0] == '\n') {
    state->line += 1;
    state->column = 0;
  } else if (!state->reader.eof) {
    state->column += 1;
  }
  return MTSD_OK;
}
