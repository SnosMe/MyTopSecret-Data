cmake_minimum_required(VERSION 3.15.0)
project(mtsd VERSION 1.0.0)

add_subdirectory("lib")

if(NOT EMSCRIPTEN)
  add_subdirectory("cli")
else()
  add_subdirectory("web-app")
endif()
