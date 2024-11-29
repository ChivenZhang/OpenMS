
add_executable(FrameServer
	FrameConfig.h
	Message.pb.cc
	Message.pb.h
	Server/FrameServer.cpp
	Server/FrameServer.h
	Server/FrameService.cpp
	Server/FrameService.h
)
add_dependencies(FrameServer OpenMS libprotobuf)
target_include_directories(FrameServer PRIVATE ${OPENMS_SOURCE_DIR}/External/protobuf/src)
target_include_directories(FrameServer PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(FrameServer OpenMS libprotobuf)

add_executable(FrameClient
	FrameConfig.h
	Message.pb.cc
	Message.pb.h
	Client/FrameClient.cpp
	Client/FrameClient.h
	Client/FrameService.cpp
	Client/FrameService.h
)
add_dependencies(FrameClient OpenMS libprotobuf xege)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/External/protobuf/src)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/External/xege/include)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(FrameClient OpenMS libprotobuf xege)