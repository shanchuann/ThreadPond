cmake_minimum_required(VERSION 3.10)
project(ThreadPond)

add_executable(test1 test1.cpp ${SRC_THREAD})

target_link_libraries(test1 pthread)