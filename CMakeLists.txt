# 指定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(ThreeKingdom)

# 指定 C 语言标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 添加全局编译选项 -g
add_compile_options(-g)
# 添加全局编译选项 -Wall
add_compile_options(-Wall)

# 添加库目录
add_subdirectory(lib)

# 添加主程序目录
add_subdirectory(src)

install(DIRECTORY  cfg DESTINATION /home/zdd/my-threee-kingdom/test)
if(NOT EXISTS /home/zdd/my-threee-kingdom/test/log)
    file(MAKE_DIRECTORY /home/zdd/my-threee-kingdom/test/log)
endif()