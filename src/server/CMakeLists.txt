
aux_source_directory(. SRC_LIST)#定义变量SRC,包含该目录下所有的源文件
add_executable(chatServer ${SRC_LIST})#执行

#指定可执行链接时需要的动态库
target_link_libraries(chatServer muduo_net muduo_base pthread) #连接动态库