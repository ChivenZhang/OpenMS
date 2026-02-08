if(OPENMS_ENABLE_SYNC_FRAME)
add_executable(FrameFrontend
        FrontendClient.cpp
        FrontendClient.h
        ClientService.cpp
        ClientService.h
        PlayerService.cpp
        PlayerService.h
)
add_dependencies(FrameFrontend OpenMS)
target_include_directories(FrameFrontend PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(FrameFrontend PRIVATE OpenMS)
endif()