if(OPENMS_ENABLE_SYNC_STATE)
add_executable(StateFrontend
        FrontendClient.cpp
        FrontendClient.h
        ClientService.cpp
        ClientService.h
        PlayerService.cpp
        PlayerService.h
)
add_dependencies(StateFrontend OpenMS)
target_include_directories(StateFrontend PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(StateFrontend PRIVATE OpenMS)
endif()