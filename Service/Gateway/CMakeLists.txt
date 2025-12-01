if(OPENMS_ENABLE_GATEWAY)
add_executable(GatewayServer
        GatewayServer.cpp
        GatewayServer.h
        ProxyService.cpp
        ProxyService.h
        GuestService.cpp
        GuestService.h
)
add_dependencies(GatewayServer OpenMS)
target_include_directories(GatewayServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(GatewayServer PRIVATE OpenMS)
endif()