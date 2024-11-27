
add_executable(FrameServer
	FrameServer.cpp
	Message.pb.cc
	Message.pb.h
)
add_dependencies(FrameServer OpenMS libprotobuf)
target_include_directories(FrameServer PRIVATE ${OPENMS_SOURCE_DIR}/External/protobuf/src)
target_include_directories(FrameServer PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(FrameServer OpenMS libprotobuf)

add_executable(FrameClient
	FrameClient.cpp
	Message.pb.cc
	Message.pb.h
)
add_dependencies(FrameClient OpenMS libprotobuf xege)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/External/protobuf/src)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/External/xege/include)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(FrameClient OpenMS libprotobuf xege)