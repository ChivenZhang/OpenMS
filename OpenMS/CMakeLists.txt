cmake_minimum_required (VERSION 3.8)

add_definitions(
	#-DOPENMS_SHARED_LIBRARY
)

add_library(OpenMS STATIC
	Private/Service.cpp
	Private/Service.h
	IService.h
	MS.h
)
add_dependencies(OpenMS uv_a)

target_include_directories(OpenMS PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(OpenMS PRIVATE ${CMAKE_SOURCE_DIR}/External/libuv/include)
target_link_libraries(OpenMS uv_a)