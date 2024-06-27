cmake_minimum_required(VERSION 3.10)
project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -g")

# 添加需要链接的库
find_package(Threads REQUIRED)
find_library(MYSQLCPPCONN_LIB mysqlcppconn)

# 添加Google Test子模块
find_package(GTest REQUIRED)


# 包含头文件目录
include_directories(
    ../log
    ../buffer
    ../pool
    ../timer
)

# 添加源文件
file(GLOB SOURCES
    ../log/*.cpp
    ../buffer/*.cpp
    ../pool/*.cpp
    ../timer/*.cpp
    ./test.cpp  # 测试文件
)

# 添加可执行文件
add_executable(test ${SOURCES})

# 链接库和线程
target_link_libraries(test
    Threads::Threads
    ${MYSQLCPPCONN_LIB}
    GTest::GTest
    GTest::Main
)
