add_library(tiny-AES-c STATIC)

target_sources(tiny-AES-c PRIVATE "aes.c")

target_compile_definitions(tiny-AES-c PRIVATE
  "CBC=0"
  "ECB=0"
  "CTR=1")

target_include_directories(tiny-AES-c INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
