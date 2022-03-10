# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("AyanBot")

# 要求最新编译标准
add_compile_options ("/std:c++latest")

# 引入hv依赖并设置外部库目录
include_directories ("dep\\hv\\build\\include")
link_directories ("dep\\hv\\build\\lib\\Debug")

# 包含子项目。
add_subdirectory ("Ayan")
add_subdirectory ("dep/hv")

