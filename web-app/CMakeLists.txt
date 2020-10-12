add_subdirectory("../third_party/libdmtx" "${PROJECT_BINARY_DIR}/third_party/libdmtx")

add_executable(mtsd-native src/bindings.c)

target_link_libraries(mtsd-native PUBLIC
  mtsd
  dmtx)

target_include_directories(mtsd-native PUBLIC "${PROJECT_BINARY_DIR}")

target_link_options(mtsd-native PRIVATE
  "SHELL:-s WASM=1"
  "SHELL:-s FILESYSTEM=0"
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s STRICT=1"
  "SHELL:-s SINGLE_FILE=1"
  "SHELL:-s ALLOW_MEMORY_GROWTH=1"
  "SHELL:-s MEMORY_GROWTH_GEOMETRIC_STEP=0"
  "SHELL:-s EXPORTED_FUNCTIONS='[_malloc,_free,_b_mtsd_decrypt,_mtsd_container_is_valid,_b_dmtx_find_regions,_b_dmtx_create,_b_last_error,_b_mtsd_encrypt]'"
  "SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS='[stackSave,stackAlloc,stackRestore]'"
  "-O3"
  "-fno-rtti"
  "-fno-exceptions"
  )
