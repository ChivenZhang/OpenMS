cmake_minimum_required (VERSION 3.8)
project(OpenMS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set(OPENMS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "OpenMS Source Path")

add_definitions(-DOPENMS_TEST)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${OPENMS_SOURCE_DIR})

add_subdirectory(${CMAKE_SOURCE_DIR}/External/libuv)
add_subdirectory(${CMAKE_SOURCE_DIR}/External/iocpp)
add_subdirectory(${CMAKE_SOURCE_DIR}/OpenMS)
add_subdirectory(${CMAKE_SOURCE_DIR}/Service)

if(OPENMS_ENABLE_SAMPLE)
add_executable(Sample Sample/main.cpp Sample/TestHandler.h)
add_dependencies(Sample OpenMS)
target_include_directories(Sample PRIVATE ${CMAKE_SOURCE_DIR}/OpenMS)
target_link_libraries(Sample OpenMS)
endif()