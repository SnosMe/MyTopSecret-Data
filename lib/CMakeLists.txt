add_subdirectory("../libs/lzma" "${PROJECT_BINARY_DIR}/libs/lzma")
add_subdirectory("../libs/randombytes" "${PROJECT_BINARY_DIR}/libs/randombytes")
add_subdirectory("../libs/tiny-AES-c" "${PROJECT_BINARY_DIR}/libs/tiny-AES-c")
add_subdirectory("../libs/argon2" "${PROJECT_BINARY_DIR}/libs/argon2")

add_library(mtsd STATIC)

target_sources(mtsd PRIVATE
  "encoding.c"
  "compression.c"
  "encryption.c"
  "container.c"
  "lang/reader.c"
  "lang/lexer.c"
  "lang/parser.c"
  "lang/dumpper.c"
  )

target_link_libraries(mtsd PRIVATE
  lzma
  randombytes
  tiny-AES-c
  argon2
  )

target_include_directories(mtsd PRIVATE "${PROJECT_BINARY_DIR}")

target_include_directories(mtsd PUBLIC "../include")
