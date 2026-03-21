if(OPENMS_ENABLE_SYNC_FRAME)
project(FrameFrontendSDK)
add_executable(${PROJECT_NAME}
        FrontendClient.cpp
        FrontendClient.h
        ClientService.cpp
        ClientService.h
        PlayerService.cpp
        PlayerService.h
)
add_dependencies(${PROJECT_NAME} OpenMS)
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENMS_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenMS)
endif()