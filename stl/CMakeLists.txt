cmake_minimum_required(VERSION 3.10)
project(fft)

set(CMAKE_CXX_STANDARD 14)

find_package(Threads REQUIRED)

add_executable(fft main.cpp)
target_link_libraries(fft ${CMAKE_THREAD_LIBS_INIT})
