
add_executable(Sample
	main.cpp
)
add_dependencies(Sample OpenMS)
target_include_directories(Sample PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(Sample OpenMS)

add_subdirectory(FrameSync)
add_subdirectory(StatusSync)