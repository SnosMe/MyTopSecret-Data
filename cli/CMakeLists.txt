
add_executable(mtsd-cli "main.c")

target_link_libraries(mtsd-cli PUBLIC mtsd)

target_include_directories(mtsd-cli PUBLIC "${PROJECT_BINARY_DIR}")
