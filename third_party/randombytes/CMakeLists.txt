add_library(randombytes STATIC)

target_sources(randombytes PRIVATE "randombytes.c")

target_include_directories(randombytes INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
