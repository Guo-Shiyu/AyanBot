# CMakeList.txt: Ayan 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)


# 将源代码添加到此项目的可执行文件。

add_executable (Ayan "Ayan.h" "Ayan.cpp" 
			"core/env.h" "core/env.cpp" "core/bot.h" "core/bot.cpp" "core/service.h" "core/service.cpp" "core/fwd.h" "core/redirect.h" "core/utility.h" "core/utility.cpp" "core/event.h" "core/event.cpp"   "core/apicall.h" "core/apicall.cpp"   "core/message.h" "core/message.cpp" "core/apiimpl.h" "core/apiimpl.cpp") 

add_subdirectory ("core")

target_link_libraries (Ayan "hv.lib")

# TODO: 如有需要，请添加测试并安装目标。
