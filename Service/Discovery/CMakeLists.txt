if(OPENMS_ENABLE_DISCOVERY)
add_executable(DiscoveryService
        DiscoveryService.cpp
        DiscoveryService.h
)
add_dependencies(DiscoveryService OpenMS)
target_include_directories(DiscoveryService PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(DiscoveryService PRIVATE OpenMS)
endif()