if(OPENMS_ENABLE_DISCOVERY)
add_executable(DiscoveryServer
        GatewayServer.cpp
        GatewayServer.h
)
add_dependencies(DiscoveryServer OpenMS)
target_include_directories(DiscoveryServer PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(DiscoveryServer PRIVATE OpenMS)
endif()