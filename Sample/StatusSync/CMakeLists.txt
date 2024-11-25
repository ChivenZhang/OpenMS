
add_executable(StatusServer
	StatusServer.cpp
	StatusServer.h
)
add_dependencies(StatusServer OpenMS)
target_include_directories(StatusServer PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(StatusServer OpenMS)

add_executable(StatusClient
	StatusClient.cpp
	StatusClient.h
)
add_dependencies(StatusClient OpenMS)
target_include_directories(StatusClient PRIVATE ${OPENMS_SOURCE_DIR}/OpenMS)
target_link_libraries(StatusClient OpenMS)