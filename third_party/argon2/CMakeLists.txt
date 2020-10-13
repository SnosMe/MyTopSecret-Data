add_library(argon2 STATIC)

target_sources(argon2 PRIVATE
  "src/argon2.c"
  "src/core.c"
  "src/blake2/blake2b.c"
  "src/thread.c"
  "src/encoding.c"
  "src/opt.c")

if(EMSCRIPTEN)
  target_compile_options(argon2 PRIVATE "-msimd128" "-msse2") # TODO: -mssse3, when wasm perf improves
  target_compile_definitions(argon2 PRIVATE "ARGON2_NO_THREADS")
endif()

target_include_directories(argon2 PUBLIC "include")
