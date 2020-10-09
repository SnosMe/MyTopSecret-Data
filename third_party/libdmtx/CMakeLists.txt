cmake_minimum_required(VERSION 3.12)
project(DMTX VERSION 0.7.5 LANGUAGES C)

add_library(dmtx STATIC "dmtx.c")

target_link_libraries(dmtx PUBLIC -lm)

target_include_directories(dmtx PUBLIC "./")
