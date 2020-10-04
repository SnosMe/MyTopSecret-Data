
add_executable(mtsd-native src/bindings.c)

target_link_libraries(mtsd-native PUBLIC mtsd)

target_include_directories(mtsd-native PUBLIC "${PROJECT_BINARY_DIR}")

target_link_options(mtsd-native PRIVATE
  "SHELL:-s WASM=1"
  "SHELL:-s FILESYSTEM=0"
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s STRICT=1"
  "SHELL:-s SINGLE_FILE=1"
  "SHELL:-s ALLOW_MEMORY_GROWTH=1"
  "SHELL:-s EXPORTED_FUNCTIONS='[_malloc,_free]'"
  "-O3"
  "-fno-rtti"
  "-fno-exceptions"
  )
