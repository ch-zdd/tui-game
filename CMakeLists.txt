# 指定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(tui-game)

# 指定 C 语言标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加全局编译选项 -g
add_compile_options(-g)
# 添加全局编译选项 -Wall
add_compile_options(-Wall)


# 添加lib目录下的源文件
file(GLOB LIB_SOURCES "lib/*.c")

# 添加子项目目录
add_subdirectory(src/ccz)
add_subdirectory(src/tetris)

if(NOT EXISTS test/log)
    file(MAKE_DIRECTORY test/log)
endif()

install(DIRECTORY  cfg DESTINATION ${CMAKE_SOURCE_DIR}/test)
install(TARGETS ccz DESTINATION ${CMAKE_SOURCE_DIR}/test)
install(TARGETS tetris DESTINATION ${CMAKE_SOURCE_DIR}/test)
