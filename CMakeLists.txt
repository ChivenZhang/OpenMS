cmake_minimum_required (VERSION 3.8)
project(OpenMS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR})
set(OPENMS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#add_definitions(-DOPENMS_TEST)

include_directories(${OPENMS_SOURCE_DIR})

add_subdirectory(${OPENMS_SOURCE_DIR}/External/libuv)
add_subdirectory(${OPENMS_SOURCE_DIR}/External/iocpp)
add_subdirectory(${OPENMS_SOURCE_DIR}/External/xege)
add_subdirectory(${OPENMS_SOURCE_DIR}/External/protobuf)
add_subdirectory(${OPENMS_SOURCE_DIR}/External/mbedtls)
add_subdirectory(${OPENMS_SOURCE_DIR}/OpenMS)

if(OPENMS_ENABLE_SAMPLE)

add_subdirectory(${OPENMS_SOURCE_DIR}/Sample)
add_subdirectory(${OPENMS_SOURCE_DIR}/Service)

endif()