
add_executable(FrameServer
	FrameConfig.h
	Message.pb.cc
	Message.pb.h
	Server/FrameServer.cpp
	Server/FrameServer.h
	Server/FrameService.cpp
	Server/FrameService.h
	Server/FrameConfig.h
	Server/Battle.cpp
	Server/Battle.h
	Server/BattleManager.cpp
	Server/BattleManager.h
	Server/Player.cpp
	Server/Player.h
	Server/PlayerManager.cpp
	Server/PlayerManager.h
	Server/User.cpp
	Server/User.h
	Server/UserManager.cpp
	Server/UserManager.h
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
	Client/FrameConfig.h
)
add_dependencies(FrameClient OpenMS libprotobuf xege)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/External/protobuf/src)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/External/xege/include)
target_include_directories(FrameClient PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(FrameClient OpenMS libprotobuf xege)