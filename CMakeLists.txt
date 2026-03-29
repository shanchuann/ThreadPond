cmake_minimum_required(VERSION 3.10)
project(ThreadPond)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/ScheduledThreadPool/include)
include_directories(${PROJECT_SOURCE_DIR}/logsys/include)

aux_source_directory(${PROJECT_SOURCE_DIR}/ScheduledThreadPool/src SRC_THREAD)
aux_source_directory(${PROJECT_SOURCE_DIR}/logsys/src SRC_LOG)

add_subdirectory(test)