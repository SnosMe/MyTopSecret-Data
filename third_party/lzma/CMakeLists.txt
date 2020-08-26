add_library(lzma STATIC)

target_sources(lzma PRIVATE
  "LzmaDec.c"
  "LzmaEnc.c"
  "LzFind.c")

target_compile_definitions(lzma PRIVATE "_7ZIP_ST")

target_include_directories(lzma INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
