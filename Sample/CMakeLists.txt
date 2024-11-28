
add_executable(DemoService
	DemoServer.cpp
	DemoServer.h
	DemoService.cpp
	DemoService.h
)
add_dependencies(DemoService OpenMS)
target_include_directories(DemoService PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(DemoService OpenMS)

add_subdirectory(FrameSync)
add_subdirectory(StatusSync)