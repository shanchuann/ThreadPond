cmake_minimum_required(VERSION 3.10)

add_executable(test ScheduledTest.cpp ${SRC_THREAD} ${SRC_LOG})

target_link_libraries(test pthread)