cmake_minimum_required(VERSION 3.16) #指定cmake版本
project(chat)
set(CMAKE_CXX_STANDARD 11) #指定C++版本

#配置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) #指定生成库储存路径

#配置头文件搜索路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include) #指定头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server) #指定头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}//include/server/db) #指定头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}//include/server/model) #指定头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty) #指定头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server/redis) #指定头文件


 
#加载子目录
add_subdirectory(src)