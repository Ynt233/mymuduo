cmake_minimum_required(VERSION 3.0)
project(mymuduo)

# mymuduo编译成so动态库时，动态库的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息
set(CMAKE_CXX_FLAGS, "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")

# 定义参与编译的文件
aux_source_directory(. SRC_LIST)
# 编译生成so
add_library(mymuduo SHARED ${SRC_LIST})