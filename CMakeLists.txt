# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.0.0)  
#set(CMAKE_C_COMPILER "/usr/local/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")
# 项目名称
project(main)

#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}) 
set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++14")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})   
find_package(MySQL)
FIND_PACKAGE(Threads)

#向工程添加多个特定的头文件搜索路径,路径之间用空格分隔
include_directories(include)
include_directories(/usr/local/include/c++/7.3.0)

#指定生成可执行文件的目录   PROJECT_BINARY_DIR 运行cmake命令的目录,通常是${PROJECT_SOURCE_DIR}/build   
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

#添加 src 子目录 
add_subdirectory(src)

# 指定生成目标 将名为 main.cpp 的源文件编译成一个名称为 ${PROJECT_NAME} 的可执行文件。
add_executable(${PROJECT_NAME} main.cpp include/blockqueue.hpp)   


# 添加链接库  可执行文件 ${PROJECT_NAME} 需要连接一个名为 Lib 的链接库 
target_link_libraries(${PROJECT_NAME} LIB_SRCS)
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${MYSQL_LIBRARY})