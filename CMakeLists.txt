cmake_minimum_required(VERSION 3.12)
project(MiniMatchingEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

add_executable(mini_engine src/main.cpp)
target_include_directories(mini_engine PRIVATE include)
target_link_libraries(mini_engine PRIVATE Threads::Threads)
# Note: deliberately NO -ffast-math (unsafe for price/P&L maths).
target_compile_options(mini_engine PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(mini_engine PRIVATE $<$<CONFIG:Release>:-O3>)
