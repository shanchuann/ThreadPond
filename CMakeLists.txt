cmake_minimum_required(VERSION 3.10)
project(ThreadPond)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(./FixedThreadPool/include)
aux_source_directory(./FixedThreadPool/src SRC_THREAD)

add_subdirectory(test)