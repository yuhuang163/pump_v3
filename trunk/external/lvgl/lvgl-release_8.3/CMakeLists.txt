cmake_minimum_required(VERSION 3.10)

# 设置代码编译字符集
add_definitions("-DUNICODE" "-D_UNICODE")
project(lvgl)

file(GLOB ALL_SRCS *.c)

file(GLOB_RECURSE MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE MAIN_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

message("MAIN_SRC="${MAIN_SRC})
 
add_executable(${PROJECT_NAME} ${ALL_SRCS} ${MAIN_SRC})