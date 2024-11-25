
add_executable(FrameServer
	FrameServer.cpp
)
add_dependencies(FrameServer OpenMS)
target_include_directories(FrameServer PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(FrameServer OpenMS)

add_executable(FrameClient
	FrameClient.cpp
	
)
add_dependencies(FrameClient OpenMS)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(FrameClient OpenMS)