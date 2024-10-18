# 指定库的名称
add_library(tk STATIC)

# 添加库的源文件
file(GLOB SOURCES "*.c")
target_sources(tk PRIVATE ${SOURCES})

# 包含头文件目录
target_include_directories(tk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})