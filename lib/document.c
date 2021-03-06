#include "private.h"

static char* MTSD_KEYS[] = {
  /* 000 */ "__NULL",
  /* 001 */ "email",
  /* 002 */ "password",
  /* 003 */ "login",
  /* 004 */ "key",
  /* 005 */ "value",
  /* 006 */ "data",
  /* 007 */ "text",
  /* 008 */ "username",
  /* 009 */ "cvc",
  /* 010 */ "code",
  /* 011 */ "pin",
  /* 012 */ "ip",
  /* 013 */ "name",
  /* 014 */ "url",
  /* 015 */ "service",
  /* 016 */ "db",
  /* 017 */ "from",
  /* 018 */ "to",
  /* 019 */ "port",
  /* 020 */ "protocol",
  /* 021 */ "number",
  /* 022 */ "tel",
  /* 023 */ "address",
  /* 024 */ "user",
  /* 025 */ "desc",
  /* 026 */ "note",
  /* 027 */ "date",
  /* 028 */ "time",
  /* 029 */ "total",
  /* 030 */ "",
  /* 031 */ "",
  /* 032 */ "",
  /* 033 */ "",
  /* 034 */ "",
  /* 035 */ "",
  /* 036 */ "",
  /* 037 */ "",
  /* 038 */ "",
  /* 039 */ "",
  /* 040 */ "",
  /* 041 */ "",
  /* 042 */ "",
  /* 043 */ "",
  /* 044 */ "",
  /* 045 */ "",
  /* 046 */ "",
  /* 047 */ "",
  /* 048 */ "",
  /* 049 */ "",
  /* 050 */ "",
  /* 051 */ "",
  /* 052 */ "",
  /* 053 */ "",
  /* 054 */ "",
  /* 055 */ "",
  /* 056 */ "",
  /* 057 */ "",
  /* 058 */ "",
  /* 059 */ "",
  /* 060 */ "",
  /* 061 */ "",
  /* 062 */ "",
  /* 063 */ "",
  /* 064 */ "",
  /* 065 */ "",
  /* 066 */ "",
  /* 067 */ "",
  /* 068 */ "",
  /* 069 */ "",
  /* 070 */ "",
  /* 071 */ "",
  /* 072 */ "",
  /* 073 */ "",
  /* 074 */ "",
  /* 075 */ "",
  /* 076 */ "",
  /* 077 */ "",
  /* 078 */ "",
  /* 079 */ "",
  /* 080 */ "",
  /* 081 */ "",
  /* 082 */ "",
  /* 083 */ "",
  /* 084 */ "",
  /* 085 */ "",
  /* 086 */ "",
  /* 087 */ "",
  /* 088 */ "",
  /* 089 */ "",
  /* 090 */ "",
  /* 091 */ "",
  /* 092 */ "",
  /* 093 */ "",
  /* 094 */ "",
  /* 095 */ "",
  /* 096 */ "",
  /* 097 */ "",
  /* 098 */ "",
  /* 099 */ "",
  /* 100 */ "",
  /* 101 */ "",
  /* 102 */ "",
  /* 103 */ "",
  /* 104 */ "",
  /* 105 */ "",
  /* 106 */ "",
  /* 107 */ "",
  /* 108 */ "",
  /* 109 */ "",
  /* 110 */ "",
  /* 111 */ "",
  /* 112 */ "",
  /* 113 */ "",
  /* 114 */ "",
  /* 115 */ "",
  /* 116 */ "",
  /* 117 */ "",
  /* 118 */ "",
  /* 119 */ "",
  /* 120 */ "",
  /* 121 */ "",
  /* 122 */ "",
  /* 123 */ "",
  /* 124 */ "",
  /* 125 */ "",
  /* 126 */ "",
  /* 127 */ "",
  /* 128 */ "",
  /* 129 */ "",
  /* 130 */ "",
  /* 131 */ "",
  /* 132 */ "",
  /* 133 */ "",
  /* 134 */ "",
  /* 135 */ "",
  /* 136 */ "",
  /* 137 */ "",
  /* 138 */ "",
  /* 139 */ "",
  /* 140 */ "",
  /* 141 */ "",
  /* 142 */ "",
  /* 143 */ "",
  /* 144 */ "",
  /* 145 */ "",
  /* 146 */ "",
  /* 147 */ "",
  /* 148 */ "",
  /* 149 */ "",
  /* 150 */ "",
  /* 151 */ "",
  /* 152 */ "",
  /* 153 */ "",
  /* 154 */ "",
  /* 155 */ "",
  /* 156 */ "",
  /* 157 */ "",
  /* 158 */ "",
  /* 159 */ "",
  /* 160 */ "",
  /* 161 */ "",
  /* 162 */ "",
  /* 163 */ "",
  /* 164 */ "",
  /* 165 */ "",
  /* 166 */ "",
  /* 167 */ "",
  /* 168 */ "",
  /* 169 */ "",
  /* 170 */ "",
  /* 171 */ "",
  /* 172 */ "",
  /* 173 */ "",
  /* 174 */ "",
  /* 175 */ "",
  /* 176 */ "",
  /* 177 */ "",
  /* 178 */ "",
  /* 179 */ "",
  /* 180 */ "",
  /* 181 */ "",
  /* 182 */ "",
  /* 183 */ "",
  /* 184 */ "",
  /* 185 */ "",
  /* 186 */ "",
  /* 187 */ "",
  /* 188 */ "",
  /* 189 */ "",
  /* 190 */ "",
  /* 191 */ "",
  /* 192 */ "",
  /* 193 */ "",
  /* 194 */ "",
  /* 195 */ "",
  /* 196 */ "",
  /* 197 */ "",
  /* 198 */ "",
  /* 199 */ "",
  /* 200 */ "",
  /* 201 */ "",
  /* 202 */ "",
  /* 203 */ "",
  /* 204 */ "",
  /* 205 */ "",
  /* 206 */ "",
  /* 207 */ "",
  /* 208 */ "",
  /* 209 */ "",
  /* 210 */ "",
  /* 211 */ "",
  /* 212 */ "",
  /* 213 */ "",
  /* 214 */ "",
  /* 215 */ "",
  /* 216 */ "",
  /* 217 */ "",
  /* 218 */ "",
  /* 219 */ "",
  /* 220 */ "",
  /* 221 */ "",
  /* 222 */ "",
  /* 223 */ "",
  /* 224 */ "",
  /* 225 */ "",
  /* 226 */ "",
  /* 227 */ "",
  /* 228 */ "",
  /* 229 */ "",
  /* 230 */ "",
  /* 231 */ "",
  /* 232 */ "",
  /* 233 */ "",
  /* 234 */ "",
  /* 235 */ "",
  /* 236 */ "",
  /* 237 */ "",
  /* 238 */ "",
  /* 239 */ "",
  /* 240 */ "",
  /* 241 */ "",
  /* 242 */ "",
  /* 243 */ "",
  /* 244 */ "",
  /* 245 */ "",
  /* 246 */ "",
  /* 247 */ "",
  /* 248 */ "",
  /* 249 */ "",
  /* 250 */ "",
  /* 251 */ "",
  /* 252 */ "",
  /* 253 */ "",
  /* 254 */ "",
  /* 255 */ "__RESERVED"
};

size_t mtsd_doc_records_count(const mtsd_document* doc)
{
  size_t records_num = 0;
  mtsd_record* record = doc->records;
  while (record) {
    records_num += 1;
    record = record->next;
  }
  return records_num;
}

size_t mtsd_doc_record_fields_count(const mtsd_record* record)
{
  size_t fields_num = 0;
  mtsd_field* field = record->fields;
  while (field) {
    fields_num += 1;
    field = field->next;
  }
  return fields_num;
}

int mtsd_doc_field_is_multiline_value(const mtsd_field* field)
{
  for (size_t i = 0; i < field->value_size; i += 1) {
    if (field->value[i] == '\n') {
      return 1;
    }
  }
  return 0;
}

void mtsd_doc_init(mtsd_document* doc)
{
  doc->records = NULL;
}

void mtsd_doc_record_init(mtsd_record* record)
{
  record->fields = NULL;
  record->next = NULL;
}

void mtsd_doc_field_init(mtsd_field* field)
{
  field->key = 0;
  field->value_size = 0;
  field->value = NULL;
  field->next = NULL;
}

void mtsd_doc_free(mtsd_document* doc)
{
  mtsd_record* record = doc->records;
  while (record) {
    mtsd_field* field = record->fields;
    while (field) {
      if (field->value) {
        MTSD_FREE(field->value);
      }

      mtsd_field* next = field->next;
      MTSD_FREE(field);
      field = next;
    }

    mtsd_record* next = record->next;
    MTSD_FREE(record);
    record = next;
  }

  mtsd_doc_init(doc);
}

uint8_t mtsd_doc_get_key_id(const uint8_t* str, size_t str_size)
{
  if (!str)
    return 0;

  for (uint8_t keyid = 1; keyid < 0xFF; keyid += 1) {
    char* key = MTSD_KEYS[keyid];
    for (size_t i = 0; /* */; i += 1) {
      if (key[i] == '\0') {
        if (str_size == i) {
          return keyid;
        } else {
          break;
        }
      }

      if (i >= str_size || key[i] != str[i])
        break;
    }
  }

  return 0;
}

int mtsd_doc_is_valid_keyid(uint8_t keyid)
{
  if (keyid == 0 || keyid == 0xFF || MTSD_KEYS[keyid][0] == '\0') {
    return 0;
  }
  return 1;
}

char* mtsd_doc_keyid_to_string(uint8_t keyid)
{
  if (!mtsd_doc_is_valid_keyid(keyid)) {
    return "__NULL";
  }
  return MTSD_KEYS[keyid];
}
