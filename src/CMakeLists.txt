# 指定可执行文件的名称
add_executable(app app.c app-init.c main.c)

# 链接静态库
target_link_libraries(app tk)

# 包含库的头文件目录
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

install(TARGETS app DESTINATION /home/zdd/my-threee-kingdom/test)