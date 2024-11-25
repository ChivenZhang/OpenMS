
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

if(WIN32)
set_target_properties(FrameClient
	PROPERTIES
	LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
	LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE /ENTRY:mainCRTStartup"
	LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
	LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
	)
endif(WIN32)

target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/External/xege/include)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(FrameClient OpenMS xege)