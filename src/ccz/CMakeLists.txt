cmake_minimum_required(VERSION 3.10)
project(ccz)

# 添加subproject1目录下的源文件
file(GLOB SUBPROJECT1_SOURCES "*.c")

# 添加lib目录下的源文件
file(GLOB LIB_SOURCES "${CMAKE_SOURCE_DIR}/lib/*.c")

# 创建subproject1和main.c的可执行文件
add_executable(ccz ${SUBPROJECT1_SOURCES} ${LIB_SOURCES})

# 链接ncursesw库
target_link_libraries(ccz ncursesw m)