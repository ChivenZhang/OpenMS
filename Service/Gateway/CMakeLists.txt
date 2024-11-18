
add_executable(GatewayService
	GatewayHandler.cpp
	GatewayHandler.h
	GatewayServer.cpp
	GatewayServer.h
	GatewayService.cpp
	GatewayService.h
)
add_dependencies(GatewayService OpenMS)

target_include_directories(GatewayService PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(GatewayService PRIVATE ${OPENMS_SOURCE_DIR})
target_link_libraries(GatewayService OpenMS)

add_custom_command(
    TARGET GatewayService POST_BUILD  
    COMMAND "${CMAKE_SOURCE_DIR}/INSTALL.bat"
    COMMENT "Copying dll to output directory"
)