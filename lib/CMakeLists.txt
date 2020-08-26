add_subdirectory("../third_party/lzma" "${PROJECT_BINARY_DIR}/third_party/lzma")
add_subdirectory("../third_party/randombytes" "${PROJECT_BINARY_DIR}/third_party/randombytes")
add_subdirectory("../third_party/tiny-AES-c" "${PROJECT_BINARY_DIR}/third_party/tiny-AES-c")
add_subdirectory("../third_party/argon2" "${PROJECT_BINARY_DIR}/third_party/argon2")

add_library(mtsd STATIC)

target_sources(mtsd PRIVATE
  "encoding.c"
  "compression.c"
  "encryption.c"
  "container.c"
  "document.c"
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
