cmake_minimum_required(VERSION 3.0.0)
project(mtsd VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory("libs/lzma")
add_subdirectory("libs/randombytes")
add_subdirectory("libs/tiny-AES-c")
add_subdirectory("libs/argon2")

add_executable(mtsd
  "src/main.c"
  "src/compression.c"
  "src/encryption.c"
  "src/lang/parser.c"
  "src/lang/lexer.c"
  "src/lang/reader.c"
  "src/lang/dumpper.c"
  )

target_link_libraries(mtsd PUBLIC lzma)
target_link_libraries(mtsd PUBLIC randombytes)
target_link_libraries(mtsd PUBLIC tiny-AES-c)
target_link_libraries(mtsd PUBLIC argon2)

target_include_directories(mtsd PUBLIC "${PROJECT_BINARY_DIR}")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
