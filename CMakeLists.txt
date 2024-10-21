# CMakeLists.txt
cmake_minimum_required(VERSION 3.20) # cmake的最低版本要求

project(code_template_cmake CXX) # 项目名称 CXX表示C++项目

set(CMAKE_CXX_STANDARD_REQUIRED TRUE) # C++标准要求
set(CMAKE_CXX_STANDARD 20) # 设置C++标准版本

add_subdirectory(src) # 添加src文件夹作为子目录

set(exeFileName main) # 设置可执行文件名称

add_executable(${exeFileName} main.cpp) # 添加可执行文件

# 添加链接库, MemCheck为src文件夹下CMakeLists.txt中定义的库
target_link_libraries(${exeFileName} PUBLIC MemCheck)

# 编译选项
target_compile_options(${exeFileName} PUBLIC

    "-Wconversion" # 警告隐式转换
    "-Wunused-variable" # 警告未使用变量
    "-Wuninitialized" # 警告未初始化变量
    "-Weffc++" # 警告高效C++(Effective C++)
    "-Wno-return-type" # 警告未返回值
    "-Wdelete-non-virtual-dtor" # 警告delete时非虚析构函数
    "-Wparentheses" # 警告括号
    "-Wempty-body" # 警告空语句
    "-Wsuggest-override" # 警告建议override
    "-Werror=mismatched-new-delete" # new和delete不匹配
    "-Werror=return-type" # 返回值类型错误
    "-Wimplicit-fallthrough" # 隐式跌落(用于switch中不加break的情况)
)

# 以下是添加测试文件的使用(方便单文件测试运行)
add_executable(test test.cpp)
target_link_libraries(test PUBLIC MemCheck)

# 以下是项目中需要使用的子文件夹
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/NET_SRC/Initial_Learn)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/NET_SRC/Socket_Encapsulation)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/NET_SRC/Multiprocess_Server)